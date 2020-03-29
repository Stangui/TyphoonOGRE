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
		GraphicsSystem* graphicsSystem;
		LogicSystem* logicSystem;
		Ogre::Barrier* barrier;
	};
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

	graphicsSystem->Init();
	barrier->sync();

	if ( graphicsSystem->GetQuit() )
	{
		graphicsSystem->Shutdown();
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
		timeSinceLast = std::min( 1.0, ( endTime - startTime ) / 1000000.0 );
		startTime = endTime;
	}

	barrier->sync();
	graphicsSystem->DestroyScene();
	barrier->sync();
	graphicsSystem->Shutdown();
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
	ThreadData*			threadData		= reinterpret_cast< ThreadData* >( threadHandle->getUserParam() );
	GraphicsSystem*		graphicsSystem	= threadData->graphicsSystem;
	LogicSystem*		logicSystem		= threadData->logicSystem;
	Ogre::Barrier*		barrier			= threadData->barrier;

	logicSystem->Init();
	barrier->sync();

	if ( graphicsSystem->GetQuit() )
	{
		logicSystem->Shutdown();
		return 0; 
	}

	logicSystem->CreateScene();
	barrier->sync();
	
	Ogre::RenderWindow* renderWindow = graphicsSystem->GetRenderWindow();
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

	// Cleanup
	barrier->sync();
	logicSystem->DestroyScene();
	barrier->sync();
	logicSystem->Shutdown();
	barrier->sync();
	return 0;
}
