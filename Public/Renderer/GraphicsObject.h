#pragma once 

#include "OgreVector3.h"
#include "OgreQuaternion.h"
#include "OgreStringVector.h"

namespace TyphoonEngine
{
#define NUM_GAME_ENTITY_BUFFERS 4

    enum MovableObjectType
    {
        MoTypeItem,
        MoTypeEntity,
        NumMovableObjectType
    };

    struct MovableObjectDefinition
    {
        Ogre::String        meshName;
        Ogre::String        resourceGroup;
        Ogre::StringVector  submeshMaterials;
        MovableObjectType   moType;
    };

    struct ObjectTransform
    {
        Ogre::Vector3       vPos;
        Ogre::Quaternion    qRot;
        Ogre::Vector3       vScale;
    };

    struct GraphicsObject
    {

    private:
        Ogre::uint32 mId;

    public:
        //----------------------------------------
        // Only used by Graphics thread
        //----------------------------------------
        Ogre::SceneNode* mSceneNode;
        Ogre::MovableObject* mMovableObject; //Could be Entity, InstancedEntity, Item.

        //Your custom pointers go here, i.e. physics representation.
        //used only by Logic thread (hkpEntity, btRigidBody, etc)

        //----------------------------------------
        // Used by both Logic and Graphics threads
        //----------------------------------------
        ObjectTransform* mTransform[ NUM_GAME_ENTITY_BUFFERS ];
        Ogre::SceneMemoryMgrTypes   mType;

        //----------------------------------------
        // Read-only
        //----------------------------------------
        MovableObjectDefinition const* mMoDefinition;
        size_t							mTransformBufferIdx;

        GraphicsObject( Ogre::uint32 id, const MovableObjectDefinition* moDefinition, Ogre::SceneMemoryMgrTypes type ) 
            : mId( id )
            , mSceneNode( nullptr )
            , mMovableObject( nullptr )
            , mType( type )
            , mMoDefinition( moDefinition )
            , mTransformBufferIdx( 0 )
        {
            for ( int i = 0; i<NUM_GAME_ENTITY_BUFFERS; ++i )
                mTransform[ i ] = nullptr;
        }

        inline Ogre::uint32 getId( void ) const
        {
            return mId;
        }

        bool operator < ( const GraphicsObject* _r ) const
        {
            return mId<_r->mId;
        }

        static bool OrderById( const GraphicsObject* _l, const GraphicsObject* _r )
        {
            return _l->mId<_r->mId;
        }
    };

    typedef std::vector<GraphicsObject*> GameEntityVec;
}
