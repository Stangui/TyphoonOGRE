
#include "GraphicsObjectManager.h"
#include "GraphicsObject.h"

#include "LogicSystem.h"

namespace TyphoonEngine
{
    const size_t cNumTransforms = 250;

    GraphicsObjectManager::GraphicsObjectManager( Mq::MessageQueueSystem* graphicsSystem, LogicSystem* logicSystem )
        : mCurrentId( 0 )
        , mScheduledForRemovalCurrentSlot( (size_t)-1 )
        , mGraphicsSystem( graphicsSystem )
        , mLogicSystem( logicSystem )
    {
        mLogicSystem->SetGraphicsObjectManager( this );
    }
    //-----------------------------------------------------------------------------------
    GraphicsObjectManager::~GraphicsObjectManager()
    {
        mLogicSystem->SetGraphicsObjectManager( nullptr );

        {
            GameEntityVecVec::iterator itor = mScheduledForRemoval.begin();
            GameEntityVecVec::iterator end  = mScheduledForRemoval.end();
            while( itor != end )
                destroyAllGameEntitiesIn( *itor++ );
            mScheduledForRemoval.clear();
            mScheduledForRemovalAvailableSlots.clear();
        }

        destroyAllGameEntitiesIn( mGameEntities[Ogre::SCENE_DYNAMIC] );
        destroyAllGameEntitiesIn( mGameEntities[Ogre::SCENE_STATIC] );

        std::vector<ObjectTransform*>::const_iterator itor = mTransformBuffers.begin();
        std::vector<ObjectTransform*>::const_iterator end  = mTransformBuffers.end();

        while( itor != end )
        {
            OGRE_FREE_SIMD( *itor, Ogre::MEMCATEGORY_SCENE_OBJECTS );
            ++itor;
        }

        mTransformBuffers.clear();
        mAvailableTransforms.clear();
    }
    //-----------------------------------------------------------------------------------
    GraphicsObject* GraphicsObjectManager::AddGameEntity( Ogre::SceneMemoryMgrTypes type,
                                                  const MovableObjectDefinition *moDefinition,
                                                  const Ogre::Vector3 &initialPos,
                                                  const Ogre::Quaternion &initialRot,
                                                  const Ogre::Vector3 &initialScale )
    {
        GraphicsObject*gameEntity = new GraphicsObject( mCurrentId++, moDefinition, type );

        CreatedGameEntity cge;
        cge.m_gameEntity  = gameEntity;
        cge.m_initialTransform.vPos   = initialPos;
        cge.m_initialTransform.qRot   = initialRot;
        cge.m_initialTransform.vScale = initialScale;

        size_t slot, bufferIdx;
        aquireTransformSlot( slot, bufferIdx );

        gameEntity->mTransformBufferIdx = bufferIdx;
        for( int i=0; i<NUM_GAME_ENTITY_BUFFERS; ++i )
        {
            gameEntity->mTransform[i] = mTransformBuffers[bufferIdx] + slot + cNumTransforms * i;
            memcpy( gameEntity->mTransform[i], &cge.m_initialTransform, sizeof(ObjectTransform) );
        }

        mGameEntities[type].push_back( gameEntity );
        mLogicSystem->QueueSendMessage( mGraphicsSystem, Mq::GAME_ENTITY_ADDED, cge );

        return gameEntity;
    }
    //-----------------------------------------------------------------------------------
    void GraphicsObjectManager::RemoveGameEntity( GraphicsObject *toRemove )
    {
        Ogre::uint32 slot = getScheduledForRemovalAvailableSlot();
        mScheduledForRemoval[slot].push_back( toRemove );
        GameEntityVec::iterator itor = std::lower_bound( mGameEntities[toRemove->mType].begin(),
                                                         mGameEntities[toRemove->mType].end(),
                                                         toRemove, GraphicsObject::OrderById );
        assert( itor != mGameEntities[toRemove->mType].end() && *itor == toRemove );
        mGameEntities[toRemove->mType].erase( itor );
        mLogicSystem->QueueSendMessage( mGraphicsSystem, Mq::GAME_ENTITY_REMOVED, toRemove );
    }
    //-----------------------------------------------------------------------------------
    void GraphicsObjectManager::_notifyGameEntitiesRemoved( size_t slot )
    {
        destroyAllGameEntitiesIn( mScheduledForRemoval[slot] );

        mScheduledForRemoval[slot].clear();
        mScheduledForRemovalAvailableSlots.push_back( slot );
    }
    //-----------------------------------------------------------------------------------
    void GraphicsObjectManager::destroyAllGameEntitiesIn( GameEntityVec &container )
    {
        GameEntityVec::const_iterator itor = container.begin();
        GameEntityVec::const_iterator end  = container.end();

        while( itor != end )
        {
            releaseTransformSlot( (*itor)->mTransformBufferIdx, (*itor)->mTransform[0] );
            delete *itor;
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void GraphicsObjectManager::aquireTransformSlot( size_t &outSlot, size_t &outBufferIdx )
    {
        if( mAvailableTransforms.empty() )
        {
            ObjectTransform *buffer = reinterpret_cast<ObjectTransform*>( OGRE_MALLOC_SIMD(
                        sizeof(ObjectTransform) * cNumTransforms * NUM_GAME_ENTITY_BUFFERS,
                        Ogre::MEMCATEGORY_SCENE_OBJECTS ) );
            mTransformBuffers.push_back( buffer );
            mAvailableTransforms.push_back( Region( 0, cNumTransforms, mTransformBuffers.size() - 1 ) );
        }

        Region &region = mAvailableTransforms.back();
        outSlot = region.m_slotOffset++;
        --region.m_count;
        outBufferIdx = region.m_bufferIdx;

        if( region.m_count == 0 )
            mAvailableTransforms.pop_back();
    }
    //-----------------------------------------------------------------------------------
    void GraphicsObjectManager::releaseTransformSlot( size_t bufferIdx, ObjectTransform *transform )
    {
        //Try to prevent a lot of fragmentation by adding the slot to an existing region.
        //It won't fully avoid it, but this is good/simple enough. If you want to fully
        //prevent fragmentation, see StagingBuffer::mergeContiguousBlocks implementation.
        const size_t slot = transform - mTransformBuffers[bufferIdx];

        std::vector<Region>::iterator itor = mAvailableTransforms.begin();
        std::vector<Region>::iterator end  = mAvailableTransforms.end();

        while( itor != end )
        {
            if( itor->m_bufferIdx == bufferIdx &&
                ( itor->m_slotOffset == slot + 1 || slot == itor->m_slotOffset + itor->m_count ) )
            {
                break;
            }

            ++itor;
        }

        if( itor != end )
        {
            if( itor->m_slotOffset == slot + 1 )
                --itor->m_slotOffset;
            else //if( slot == itor->slot + itor->count )
                ++itor->m_count;
        }
        else
        {
            mAvailableTransforms.push_back( Region( slot, 1, bufferIdx ) );
        }
    }
    //-----------------------------------------------------------------------------------
    Ogre::uint32 GraphicsObjectManager::getScheduledForRemovalAvailableSlot(void)
    {
        if( mScheduledForRemovalCurrentSlot >= mScheduledForRemoval.size() )
        {
            if( mScheduledForRemovalAvailableSlots.empty() )
            {
                mScheduledForRemovalAvailableSlots.push_back( mScheduledForRemoval.size() );
                mScheduledForRemoval.push_back( GameEntityVec() );
            }
			
			mScheduledForRemovalCurrentSlot = mScheduledForRemovalAvailableSlots.back();
            mScheduledForRemovalAvailableSlots.pop_back();
        }

        return static_cast<Ogre::uint32>(mScheduledForRemovalCurrentSlot);
    }
    //-----------------------------------------------------------------------------------
    void GraphicsObjectManager::FinishFrameParallel(void)
    {
        if( mScheduledForRemovalCurrentSlot < mScheduledForRemoval.size() )
        {
            mLogicSystem->QueueSendMessage( mGraphicsSystem, Mq::GAME_ENTITY_SCHEDULED_FOR_REMOVAL_SLOT, mScheduledForRemovalCurrentSlot );
            mScheduledForRemovalCurrentSlot = (size_t)-1;
        }
    }
}
