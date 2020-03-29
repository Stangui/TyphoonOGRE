#include "LogicGameState.h"
#include "LogicSystem.h"
#include "GameEntityManager.h"

#include "OgreVector3.h"
#include "OgreResourceGroupManager.h"

namespace TyphoonEngine
{
	LogicGameState::LogicGameState() :
		mDisplacement( 0 ),
		mCubeEntity( 0 ),
		mCubeMoDef( 0 ),
		mLogicSystem( 0 )
	{
	}
	//-----------------------------------------------------------------------------------
	LogicGameState::~LogicGameState()
	{
		delete mCubeMoDef;
		mCubeMoDef = 0;
	}
	//-----------------------------------------------------------------------------------
	void LogicGameState::CreateScene( void )
	{
		const Ogre::Vector3 origin( -5.0f, 0.0f, 0.0f );

		GameEntityManager *geMgr = mLogicSystem->GetGameEntityManager();

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
	//-----------------------------------------------------------------------------------
	void LogicGameState::Update( float timeSinceLast )
	{
		const Ogre::Vector3 origin( -10.0f, 0.0f, 0.0f );

		mDisplacement += timeSinceLast * 4.0f;
		mDisplacement = fmodf( mDisplacement, 20.0f );

		const size_t currIdx = mLogicSystem->GetCurrentTransformIdx();
		mCubeEntity->mTransform[ currIdx ]->vPos = origin + Ogre::Vector3::UNIT_X * mDisplacement;
	}
}
