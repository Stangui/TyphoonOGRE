#include "LogicState.h"
#include "LogicSystem.h"
#include "GraphicsObjectManager.h"
#include "TyphoonCore.h"

#include "OgreVector3.h"
#include "OgreResourceGroupManager.h"

#define NUM_CUBES 6

namespace TyphoonEngine
{
    LogicState::LogicState() : mLogicSystem( 0 )
    {
    }
    
    //-----------------------------------------------------------------------------------
    LogicState::~LogicState()
    {
        for ( const auto* def:mCubeMoDefs )
        {
            SAFE_DELETE( def );
        }
    }
    
    //-----------------------------------------------------------------------------------
    void LogicState::CreateScene( void )
    {
        if ( GraphicsObjectManager* geMgr = mLogicSystem->GetGraphicsObjectManager() )
        {
            const float step = 20.f/( NUM_CUBES-1 );
            for ( int i = 0; i<NUM_CUBES; ++i )
            {
                ItemDefinition* def = new ItemDefinition();
                def->meshName = "Cube_d.mesh";
                def->resourceGroup = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
                def->moType = MoTypeItem;

                mCubeMoDefs.push_back( def );

                mCubeEntities.push_back( geMgr->AddGameEntity(
                    Ogre::SCENE_DYNAMIC,
                    def,
                    Ogre::Vector3(-10.f + step * i, 0.f, 0.f),
                    Ogre::Quaternion::IDENTITY,
                    Ogre::Vector3::UNIT_SCALE
                ));
            }
        }
    }
    
    //-----------------------------------------------------------------------------------
    void LogicState::Update( float timeSinceLast )
    {
        for ( auto* ent:mCubeEntities )
        {
            const size_t currIdx = mLogicSystem->GetCurrentTransformIdx();
            const size_t prevIdx = currIdx==0 ? NUM_GAME_ENTITY_BUFFERS-1 : currIdx-1;
            Ogre::Vector3& cPos = ent->mTransform[ currIdx ]->vPos;
            Ogre::Vector3& pPos = ent->mTransform[ prevIdx ]->vPos;
            cPos = pPos + Ogre::Vector3::UNIT_X * timeSinceLast;
            if ( cPos.x>10.f )
            {
                cPos.x = -10.f;
            }
        }
    }
}
