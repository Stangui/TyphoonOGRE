//---------------------------------------------------------------------------------------
// This tutorial shows nothing fancy. Only how to setup Ogre to render to
// a window, rolling your own game loop, single threaded.
//
// You only need the basics from the Common framework. Just derive from GameState to
// perform your own scene and update it accordingly.
// Scene setup is divided in two stages (createScene01 & createScene02) because it
// is prepared for multithreading; although it is not strictly necessary in this case.
// See the multithreading tutorial for a better explanation
//---------------------------------------------------------------------------------------

#include "TyphoonApp.h"
#include "GraphicsSystem.h"
#include "LogicSystem.h"
#include "GraphicsGameState.h"
#include "GameEntityManager.h"
#include "LogicGameState.h"
#include "YieldTimer.h"

#include <OgreRenderWindow.h>
#include <OgreTimer.h>

#include "Threading/OgreThreads.h"
#include "Threading/OgreBarrier.h"

extern const double cFrametime;
const double cFrametime = 1.0 / 25.0;

unsigned long renderThread( Ogre::ThreadHandle *threadHandle );
unsigned long gameThread( Ogre::ThreadHandle *threadHandle );
THREAD_DECLARE( renderThread );
THREAD_DECLARE( gameThread );

namespace TyphoonEngine
{
	struct ThreadData
	{
		GraphicsSystem  *graphicsSystem;
		LogicSystem     *logicSystem;
		Ogre::Barrier   *barrier;
	};

/*	class StereoGraphicsSystem : public GraphicsSystem
	{
		Ogre::SceneNode             *mCamerasNode;
		Ogre::Camera                *mEyeCameras[ 2 ];
		Ogre::CompositorWorkspace   *mEyeWorkspaces[ 2 ];

		//-------------------------------------------------------------------------------
		virtual void createCamera( void )
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

		virtual Ogre::CompositorWorkspace* setupCompositor()
		{
			Ogre::uint8 vpModifierMask, executionMask;
			Ogre::Vector4 vpOffsetScale;

			const Ogre::IdString workspaceName( "StereoRenderingWorkspace" );
			Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

			vpModifierMask = 0x01;
			executionMask = 0x01;
			vpOffsetScale = Ogre::Vector4( 0.0f, 0.0f, 0.5f, 1.0f );
			mEyeWorkspaces[ 0 ] = compositorManager->addWorkspace( mSceneManager, mRenderWindow,
				mEyeCameras[ 0 ], workspaceName,
				true, -1, ( Ogre::UavBufferPackedVec* )0,
				( Ogre::ResourceLayoutMap* )0,
				( Ogre::ResourceAccessMap* )0,
				vpOffsetScale,
				vpModifierMask,
				executionMask );

			vpModifierMask = 0x02;
			executionMask = 0x02;
			vpOffsetScale = Ogre::Vector4( 0.5f, 0.0f, 0.5f, 1.0f );
			mEyeWorkspaces[ 1 ] = compositorManager->addWorkspace( mSceneManager, mRenderWindow,
				mEyeCameras[ 1 ], workspaceName,
				true, -1, ( Ogre::UavBufferPackedVec* )0,
				( Ogre::ResourceLayoutMap* )0,
				( Ogre::ResourceAccessMap* )0,
				vpOffsetScale,
				vpModifierMask,
				executionMask );
			return mEyeWorkspaces[ 0 ];
		}

	public:
		StereoGraphicsSystem( GameState *gameState ) :
			GraphicsSystem( gameState )
		{
		}
	};

	void MainEntryPoints::createSystems( GameState **outGraphicsGameState,
		GraphicsSystem **outGraphicsSystem,
		GameState **outLogicGameState,
		LogicSystem **outLogicSystem )
	{
		TyphoonStereoGameState *gfxGameState = new TyphoonStereoGameState(
			"This tutorial demonstrates the most basic rendering loop: Variable framerate.\n"
			"Variable framerate means the application adapts to the current frame rendering\n"
			"performance and boosts or decreases the movement speed of objects to maintain\n"
			"the appearance that objects are moving at a constant velocity.\n"
			"When framerate is low, it looks 'frame skippy'; when framerate is high,\n"
			"it looks very smooth.\n"
			"Note: If you can't exceed 60 FPS, it's probably because of VSync being turned on.\n"
			"\n"
			"Despite what it seems, this is the most basic form of updating, and a horrible way\n"
			"to update your objects if you want to do any kind of serious game development.\n"
			"Keep going through the Tutorials for superior methods of updating the rendering loop.\n"
			"\n"
			"Note: The cube is black because there is no lighting. We are not focusing on that." );

		GraphicsSystem *graphicsSystem = new StereoGraphicsSystem( gfxGameState );

		gfxGameState->_notifyGraphicsSystem( graphicsSystem );

		*outGraphicsGameState = gfxGameState;
		*outGraphicsSystem = graphicsSystem;
	}

	void MainEntryPoints::destroySystems( GameState *graphicsGameState,
		GraphicsSystem *graphicsSystem,
		GameState *logicGameState,
		LogicSystem *logicSystem )
	{
		delete graphicsSystem;
		delete graphicsGameState;
	}

	const char* MainEntryPoints::getWindowTitle( void )
	{
		return "Stereo Rendering Sample";
	}
	*/
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow )
#else
int mainApp( int argc, const char *argv[] )
#endif
{
	TyphoonEngine::GraphicsGameState graphicsGameState(
		"This tutorial shows how to setup two update loops: One for Graphics, another for\n"
		"Logic, each in its own thread. We don't render anything because we will now need\n"
		"to do a robust synchronization for creating, destroying and updating Entities,\n"
		"which is too complex to show in just one tutorial step.\n"
		"\n"
		"The key concept is that Graphic's createScene01 runs in parallel to Logic's\n"
		"createScene01. But we guarantee that createScene02 will be called after all\n"
		"createScene01s have been called. In other words, createScene is divided in\n"
		"two stages and each stage runs in parallel.\n"
		"\n"
		"This means that Logic will be creating the entities in stage 01; and Graphics\n"
		"will see the request to create the Ogre objects (e.g. Item, SceneNode) in\n"
		"stage 02. Meanwhile Graphics may dedicate the time in stage 01 to preload some\n"
		"meshes, overlays, and other resources that will always be needed.\n"
		"Logic in stage 02 will be idle, so it may dedicate that time to load non-\n"
		"graphics related data (like physics representations).\n" );
	TyphoonEngine::GraphicsSystem graphicsSystem( &graphicsGameState );
	TyphoonEngine::LogicGameState logicGameState; //Dummy
	TyphoonEngine::LogicSystem logicSystem( &logicGameState );
	Ogre::Barrier barrier( 2 );

	graphicsGameState._notifyGraphicsSystem( &graphicsSystem );
	logicGameState._notifyLogicSystem( &logicSystem );

	graphicsSystem._notifyLogicSystem( &logicSystem );
	logicSystem._notifyGraphicsSystem( &graphicsSystem );

	TyphoonEngine::GameEntityManager gameEntityManager( &graphicsSystem, &logicSystem );

	TyphoonEngine::ThreadData threadData;
	threadData.graphicsSystem = &graphicsSystem;
	threadData.logicSystem = &logicSystem;
	threadData.barrier = &barrier;

	Ogre::ThreadHandlePtr threadHandles[ 2 ];
	threadHandles[ 0 ] = Ogre::Threads::CreateThread( THREAD_GET( renderThread ), 0, &threadData );
	threadHandles[ 1 ] = Ogre::Threads::CreateThread( THREAD_GET( gameThread ), 1, &threadData );

	Ogre::Threads::WaitForThreads( 2, threadHandles );

	return 0;
}

using namespace TyphoonEngine;

//---------------------------------------------------------------------
unsigned long renderThreadApp( Ogre::ThreadHandle *threadHandle )
{
	ThreadData *threadData = reinterpret_cast< ThreadData* >( threadHandle->getUserParam() );
	GraphicsSystem *graphicsSystem = threadData->graphicsSystem;
	Ogre::Barrier *barrier = threadData->barrier;

	graphicsSystem->initialize( "Tutorial 06: Multithreading" );
	barrier->sync();

	if ( graphicsSystem->getQuit() )
	{
		graphicsSystem->deinitialize();
		return 0; //User cancelled config
	}

	graphicsSystem->createScene01();
	barrier->sync();

	graphicsSystem->createScene02();
	barrier->sync();

	Ogre::RenderWindow *renderWindow = graphicsSystem->getRenderWindow();

	Ogre::Timer timer;

	Ogre::uint64 startTime = timer.getMicroseconds();

	double timeSinceLast = 1.0 / 60.0;

	while ( !graphicsSystem->getQuit() )
	{
		graphicsSystem->beginFrameParallel();
		graphicsSystem->update( timeSinceLast );
		graphicsSystem->finishFrameParallel();

		if ( !renderWindow->isVisible() )
		{
			//Don't burn CPU cycles unnecessary when we're minimized.
			Ogre::Threads::Sleep( 500 );
		}

		Ogre::uint64 endTime = timer.getMicroseconds();
		timeSinceLast = ( endTime - startTime ) / 1000000.0;
		timeSinceLast = std::min( 1.0, timeSinceLast ); //Prevent from going haywire.
		startTime = endTime;
	}

	barrier->sync();

	graphicsSystem->destroyScene();
	barrier->sync();

	graphicsSystem->deinitialize();
	barrier->sync();

	return 0;
}
unsigned long renderThread( Ogre::ThreadHandle *threadHandle )
{
	unsigned long retVal = -1;

	try
	{
		retVal = renderThreadApp( threadHandle );
	}
	catch ( Ogre::Exception& e )
	{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		MessageBoxA( NULL, e.getFullDescription().c_str(), "An exception has occured!",
			MB_OK | MB_ICONERROR | MB_TASKMODAL );
#else
		std::cerr << "An exception has occured: " <<
			e.getFullDescription().c_str() << std::endl;
#endif

		abort();
	}

	return retVal;
}
//---------------------------------------------------------------------
unsigned long gameThread( Ogre::ThreadHandle *threadHandle )
{
	ThreadData *threadData = reinterpret_cast< ThreadData* >( threadHandle->getUserParam() );
	GraphicsSystem *graphicsSystem = threadData->graphicsSystem;
	LogicSystem *logicSystem = threadData->logicSystem;
	Ogre::Barrier *barrier = threadData->barrier;

	logicSystem->initialize();
	barrier->sync();

	if ( graphicsSystem->getQuit() )
	{
		logicSystem->deinitialize();
		return 0; //Render thread cancelled early
	}

	logicSystem->createScene01();
	barrier->sync();

	logicSystem->createScene02();
	barrier->sync();

	Ogre::RenderWindow *renderWindow = graphicsSystem->getRenderWindow();

	Ogre::Timer timer;
	YieldTimer yieldTimer( &timer );

	Ogre::uint64 startTime = timer.getMicroseconds();

	while ( !graphicsSystem->getQuit() )
	{
		logicSystem->beginFrameParallel();
		logicSystem->update( static_cast< float >( cFrametime ) );
		logicSystem->finishFrameParallel();

		logicSystem->finishFrame();

		if ( !renderWindow->isVisible() )
		{
			//Don't burn CPU cycles unnecessary when we're minimized.
			Ogre::Threads::Sleep( 500 );
		}

		//YieldTimer will wait until the current time is greater than startTime + cFrametime
		startTime = yieldTimer.yield( cFrametime, startTime );
	}

	barrier->sync();

	logicSystem->destroyScene();
	barrier->sync();

	logicSystem->deinitialize();
	barrier->sync();

	return 0;
}
