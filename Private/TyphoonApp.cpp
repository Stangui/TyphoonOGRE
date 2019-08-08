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
const double cFrametime = 1.0 / 30.0;

unsigned long renderThread( Ogre::ThreadHandle *threadHandle );
unsigned long gameThread( Ogre::ThreadHandle *threadHandle );
THREAD_DECLARE( renderThread );
THREAD_DECLARE( gameThread );

namespace TyphoonEngine
{
	struct ThreadData
	{
		GraphicsSystem*	graphicsSystem;
		LogicSystem*	logicSystem;
		Ogre::Barrier*	barrier;
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

using namespace TyphoonEngine;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow )
#else
int mainApp( int argc, const char *argv[] )
#endif
{
	GraphicsGameState graphicsGameState( "Basic Multithreaded App" );
	GraphicsSystem graphicsSystem( &graphicsGameState );
	LogicGameState logicGameState;
	LogicSystem logicSystem( &logicGameState );
	Ogre::Barrier barrier( 2 );

	graphicsGameState.SetGraphicSystem( &graphicsSystem );
	logicSystem.SetGraphicSystem( &graphicsSystem );
	graphicsSystem.SetLogicSystem( &logicSystem );
	logicGameState.SetLogicSystem( &logicSystem );

	GameEntityManager gameEntityManager( &graphicsSystem, &logicSystem );

	ThreadData threadData;
	threadData.graphicsSystem = &graphicsSystem;
	threadData.logicSystem = &logicSystem;
	threadData.barrier = &barrier;

	Ogre::ThreadHandlePtr threadHandles[ 2 ];
	threadHandles[ 0 ] = Ogre::Threads::CreateThread( THREAD_GET( renderThread ), 0, &threadData );
	threadHandles[ 1 ] = Ogre::Threads::CreateThread( THREAD_GET( gameThread ), 1, &threadData );

	Ogre::Threads::WaitForThreads( 2, threadHandles );

	return 0;
}


//---------------------------------------------------------------------
unsigned long renderThreadApp( Ogre::ThreadHandle *threadHandle )
{
	ThreadData *threadData = reinterpret_cast< ThreadData* >( threadHandle->getUserParam() );
	GraphicsSystem *graphicsSystem = threadData->graphicsSystem;
	Ogre::Barrier *barrier = threadData->barrier;

	graphicsSystem->Initialise();
	barrier->sync();

	if ( graphicsSystem->GetQuit() )
	{
		graphicsSystem->Deinitialise();
		return 0; //User cancelled config
	}

	graphicsSystem->CreateScene();
	barrier->sync();

	Ogre::RenderWindow *renderWindow = graphicsSystem->GetRenderWindow();

	Ogre::Timer timer;

	Ogre::uint64 startTime = timer.getMicroseconds();

	double timeSinceLast = 1.0 / 120.0;

	while ( !graphicsSystem->GetQuit() )
	{
		graphicsSystem->BeginFrameParallel();
		graphicsSystem->Update( static_cast<float>(timeSinceLast) );
		graphicsSystem->FinishFrameParallel();

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

	graphicsSystem->DestroyScene();
	barrier->sync();

	graphicsSystem->Deinitialise();
	barrier->sync();

	return 0;
}

//---------------------------------------------------------------------
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

	logicSystem->Initialise();
	barrier->sync();

	if ( graphicsSystem->GetQuit() )
	{
		logicSystem->Deinitialise();
		return 0; //Render thread cancelled early
	}

	logicSystem->CreateScene();
	barrier->sync();
	
	Ogre::RenderWindow *renderWindow = graphicsSystem->GetRenderWindow();

	Ogre::Timer timer;
	YieldTimer yieldTimer( &timer );

	Ogre::uint64 startTime = timer.getMicroseconds();

	while ( !graphicsSystem->GetQuit() )
	{
		logicSystem->BeginFrameParallel();
		logicSystem->Update( static_cast< float >( cFrametime ) );
		logicSystem->FinishFrameParallel();

		logicSystem->FinishFrame();

		if ( !renderWindow->isVisible() )
		{
			//Don't burn CPU cycles unnecessary when we're minimized.
			Ogre::Threads::Sleep( 500 );
		}
		
		//YieldTimer will wait until the current time is greater than startTime + cFrametime
		startTime = yieldTimer.yield( cFrametime, startTime );
	}

	barrier->sync();

	logicSystem->DestroyScene();
	barrier->sync();

	logicSystem->Deinitialise();
	barrier->sync();

	return 0;
}
