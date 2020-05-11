#include "StereoGraphicsSystem.h"
#include "GameState.h"
#include <OgreSceneManager.h>
#include <OgreCamera.h>
#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <Compositor/OgreCompositorManager2.h>

namespace TyphoonEngine
{
	//-------------------------------------------------------------------------------
	void StereoGraphicsSystem::createCamera( void )
	{
		//Use one node to control both cameras
		mCamerasNode = mSceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->createChildSceneNode( Ogre::SCENE_DYNAMIC );
		mCamerasNode->setName( "Cameras Node" );
		mCamerasNode->setPosition( 0, 5, 15 );

		mEyeCameras[ 0 ] = mSceneManager->createCamera( "Left Eye" );
		mEyeCameras[ 1 ] = mSceneManager->createCamera( "Right Eye" );

		const Ogre::Real eyeDistance = 0.5f;
		const Ogre::Real eyeFocusDistance = 0.45f;

		for ( int i = 0; i < 2; ++i )
		{
			const Ogre::Vector3 camPos( eyeDistance * ( i * 2 - 1 ), 0, 0 );
			mEyeCameras[ i ]->setPosition( camPos );

			Ogre::Vector3 lookAt( eyeFocusDistance * ( i * 2 - 1 ), -5, -15 );
			//Ogre::Vector3 lookAt( 0, 0, 0 );

			// Look back along -Z
			mEyeCameras[ i ]->lookAt( lookAt );
			mEyeCameras[ i ]->setNearClipDistance( 0.2f );
			mEyeCameras[ i ]->setFarClipDistance( 1000.0f );
			mEyeCameras[ i ]->setAutoAspectRatio( true );

			//By default cameras are attached to the Root Scene Node.
			mEyeCameras[ i ]->detachFromParent();
			mCamerasNode->attachObject( mEyeCameras[ i ] );
		}

		mCamera = mEyeCameras[ 0 ];
	}

	//-------------------------------------------------------------------------------
	Ogre::CompositorWorkspace* StereoGraphicsSystem::setupCompositor()
	{
		Ogre::uint8 vpModifierMask, executionMask;
		Ogre::Vector4 vpOffsetScale;

		const Ogre::IdString workspaceName( "StereoRenderingWorkspace" );
		Ogre::CompositorManager2* compositorManager = mRoot->getCompositorManager2();

		vpModifierMask = 0x01;
		executionMask = 0x01;
		vpOffsetScale = Ogre::Vector4( 0.0f, 0.0f, 0.5f, 1.0f );
		mEyeWorkspaces[ 0 ] = compositorManager->addWorkspace( 
			mSceneManager, 
			mRenderWindow,
			mEyeCameras[ 0 ], 
			workspaceName,
			true, 
			-1, 
			( Ogre::UavBufferPackedVec* )0,
			( Ogre::ResourceLayoutMap* )0,
			( Ogre::ResourceAccessMap* )0,
			vpOffsetScale,
			vpModifierMask,
			executionMask 
		);

		vpModifierMask = 0x02;
		executionMask = 0x02;
		vpOffsetScale = Ogre::Vector4( 0.5f, 0.0f, 0.5f, 1.0f );
		mEyeWorkspaces[ 1 ] = compositorManager->addWorkspace( 
			mSceneManager, 
			mRenderWindow,
			mEyeCameras[ 1 ], 
			workspaceName,
			true, 
			-1, 
			( Ogre::UavBufferPackedVec* )0,
			( Ogre::ResourceLayoutMap* )0,
			( Ogre::ResourceAccessMap* )0,
			vpOffsetScale,
			vpModifierMask,
			executionMask 
		);
		return mEyeWorkspaces[ 0 ];
	}

	//-------------------------------------------------------------------------------
	StereoGraphicsSystem::StereoGraphicsSystem( IGameState* GameState ) : Renderer( GameState )
	{
	}
};