#include "LogicState.h"
#include "LogicSystem.h"
#include "GraphicsObjectManager.h"
#include "TyphoonCore.h"

#include "OgreVector3.h"
#include "OgreResourceGroupManager.h"

namespace TyphoonEngine
{
    AppLogicState::AppLogicState()
        : mDisplacement( 0 )
        , mCubeEntity( 0 )
        , mCubeMoDef( 0 )
        , mLogicSystem( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    AppLogicState::~AppLogicState()
    {
        SAFE_DELETE( mCubeMoDef );
    }
    //-----------------------------------------------------------------------------------
    void AppLogicState::CreateScene( void )
    {
        const Ogre::Vector3 origin( 0.0f, 0.0f, 0.0f );

        if ( GraphicsObjectManager* geMgr = mLogicSystem->GetGraphicsObjectManager() )
        {
            mCubeMoDef = new MovableObjectDefinition();
            mCubeMoDef->meshName = "Cube_d.mesh";
            mCubeMoDef->resourceGroup = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
            mCubeMoDef->moType = MoTypeItem;

            mCubeEntity = geMgr->AddGameEntity(
                Ogre::SCENE_DYNAMIC,
                mCubeMoDef,
                origin,
                Ogre::Quaternion::IDENTITY,
                Ogre::Vector3::UNIT_SCALE
            );
        }
    }
    //-----------------------------------------------------------------------------------
    void AppLogicState::Update( float timeSinceLast )
    {
        const Ogre::Vector3 origin( -10.0f, 0.0f, 0.0f );

        mDisplacement += timeSinceLast*10.0f;
        mDisplacement = fmodf( mDisplacement, 20.0f );

        const size_t currIdx = mLogicSystem->GetCurrentTransformIdx();
        mCubeEntity->mTransform[ currIdx ]->vPos = origin+Ogre::Vector3::UNIT_X*mDisplacement;
    }
}
