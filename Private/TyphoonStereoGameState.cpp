
#include "TyphoonStereoGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreCamera.h"
#include "OgreRenderWindow.h"

using namespace TyphoonEngine;

namespace TyphoonEngine
{
	TyphoonStereoGameState::TyphoonStereoGameState( const Ogre::String &helpDescription ) :
		TyphoonGameState( helpDescription )
	{
		memset( mSceneNode, 0, sizeof( mSceneNode ) );
	}
	//-----------------------------------------------------------------------------------
	void TyphoonStereoGameState::createScene01( void )
	{
		Ogre::SceneManager *sceneManager = mGraphicsSystem->getSceneManager();

		const float armsLength = 2.5f;

		for ( int i = 0; i < 4; ++i )
		{
			for ( int j = 0; j < 4; ++j )
			{
				Ogre::Item *item = sceneManager->createItem( "Cube_d.mesh",
					Ogre::ResourceGroupManager::
					AUTODETECT_RESOURCE_GROUP_NAME,
					Ogre::SCENE_DYNAMIC );

				size_t idx = i * 4 + j;

				mSceneNode[ idx ] = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
					createChildSceneNode( Ogre::SCENE_DYNAMIC );

				mSceneNode[ idx ]->setPosition( ( i - 1.5f ) * armsLength,
					0.0f,
					( j - 1.5f ) * armsLength );

				mSceneNode[ idx ]->roll( Ogre::Radian( idx ) );

				mSceneNode[ idx ]->attachObject( item );
			}
		}

		Ogre::Light *light = sceneManager->createLight();
		Ogre::SceneNode *lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
		lightNode->attachObject( light );
		light->setPowerScale( Ogre::Math::PI ); //Since we don't do HDR, counter the PBS' division by PI
		light->setType( Ogre::Light::LT_DIRECTIONAL );
		light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

		mCameraController = new CameraController( mGraphicsSystem, true );

		TyphoonGameState::createScene01();
	}
	//-----------------------------------------------------------------------------------
	void TyphoonStereoGameState::update( float timeSinceLast )
	{
		for ( int i = 0; i < 16; ++i )
			mSceneNode[ i ]->yaw( Ogre::Radian( timeSinceLast * i * 0.25f ) );

		TyphoonGameState::update( timeSinceLast );
	}
}
