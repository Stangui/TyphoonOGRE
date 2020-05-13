// Copyright 2020 - Ian Stangoe

#include "TyphoonApp.h"

#include <OgreRenderWindow.h>
#include <OgreTimer.h>

#include "GraphicsSystem.h"
#include "GraphicsState.h"
#include "GraphicsObjectManager.h"
#include "LogicSystem.h"
#include "LogicState.h"
#include "Threading/YieldTimer.h"
#include "Threading/OgreThreads.h"
#include "Threading/OgreBarrier.h"

namespace TyphoonEngine
{
    static unsigned long RenderThread( Ogre::ThreadHandle* threadHandle );
    static unsigned long LogicThread( Ogre::ThreadHandle* threadHandle );
    THREAD_DECLARE( RenderThread );
    THREAD_DECLARE( LogicThread );

    //---------------------------------------------------------------------
    unsigned long RenderThreadImpl( Ogre::ThreadHandle* threadHandle )
    {
        TyphoonEngine::ThreadData* ThreadData = reinterpret_cast< TyphoonEngine::ThreadData* >( threadHandle->getUserParam() );
        assert( ThreadData );

        TyphoonEngine::GraphicsSystem* GraphicsSystem = ThreadData->m_GraphicsSystem;
        Ogre::Barrier* Barriers = ThreadData->m_Barriers;

        assert( GraphicsSystem );
        assert( Barriers );

        GraphicsSystem->Init();
        Barriers->sync();

        if ( GraphicsSystem->GetQuit() )
        {
            GraphicsSystem->Shutdown();
            return 0; //User cancelled config
        }

        GraphicsSystem->CreateScene();
        Barriers->sync();

        const Ogre::RenderWindow* renderWindow = GraphicsSystem->GetRenderWindow();
        Ogre::Timer timer;
        Ogre::uint64 startTime = timer.getMicroseconds();
        double timeSinceLast = 0.0;
 
        while ( !GraphicsSystem->GetQuit() )
        {
            GraphicsSystem->BeginFrameParallel();
            GraphicsSystem->Update( static_cast<float>(timeSinceLast) );
            GraphicsSystem->FinishFrameParallel();
            GraphicsSystem->FinishFrame();

            if ( !renderWindow->isVisible() )
            {
                //Don't burn CPU cycles unnecessary when we're minimized.
                Ogre::Threads::Sleep( 500 );
            }
            
            Ogre::uint64 endTime = timer.getMicroseconds();
            timeSinceLast = ( endTime-startTime )/1000000.0;
            timeSinceLast = std::min( 1.0, timeSinceLast ); //Prevent from going haywire.
            startTime = endTime;
        }

        Barriers->sync();
        GraphicsSystem->DestroyScene();

        Barriers->sync();
        GraphicsSystem->Shutdown();

        Barriers->sync();
        return 0;
    }

    //---------------------------------------------------------------------
    unsigned long RenderThread( Ogre::ThreadHandle* threadHandle )
    {
        unsigned long retVal = -1;

        try
        {
            retVal = RenderThreadImpl( threadHandle );
        }
        catch ( Ogre::Exception& e )
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBoxA( NULL, e.getFullDescription().c_str(), "An exception has occured!",
                MB_OK|MB_ICONERROR|MB_TASKMODAL );
#else
            std::cerr<<"An exception has occured: "<<
                e.getFullDescription().c_str()<<std::endl;
#endif

            abort();
        }

        return retVal;
    }

    //---------------------------------------------------------------------
    static unsigned long LogicThread( Ogre::ThreadHandle* threadHandle )
    {
        TyphoonEngine::ThreadData* ThreadData = reinterpret_cast< TyphoonEngine::ThreadData* >( threadHandle->getUserParam() );
        assert( ThreadData );

        TyphoonEngine::GraphicsSystem* GraphicsSystem = ThreadData->m_GraphicsSystem;
        TyphoonEngine::LogicSystem* LogicSystem = ThreadData->m_LogicSystem;
        Ogre::Barrier* Barriers = ThreadData->m_Barriers;

        if ( !LogicSystem )
        {
            return 0;
        }

        assert( Barriers );
        assert( GraphicsSystem );

        Barriers->sync();

        if ( GraphicsSystem->GetQuit() )
        {
            LogicSystem->Shutdown();
            return 0;
        }

        LogicSystem->CreateScene();
        Barriers->sync();

        const Ogre::RenderWindow* renderWindow = GraphicsSystem->GetRenderWindow();
        Ogre::Timer timer;
        Ogre::uint64 startTime = timer.getMicroseconds();
        YieldTimer yieldTimer(&timer);

        while ( !GraphicsSystem->GetQuit() )
        {
            LogicSystem->BeginFrameParallel();
            LogicSystem->Update( TyphoonEngine::LOGIC_UPDATE_TIME );
            LogicSystem->FinishFrameParallel();
            LogicSystem->FinishFrame();

            if ( !renderWindow->isVisible() )
            {
                //Don't burn CPU cycles unnecessary when we're minimized.
                Ogre::Threads::Sleep( 500 );
            }

            startTime = yieldTimer.yield( TyphoonEngine::LOGIC_UPDATE_TIME, startTime );
        }

        // Cleanup
        Barriers->sync();
        LogicSystem->DestroyScene();

        Barriers->sync();
        LogicSystem->Shutdown();

        Barriers->sync();
        return 0;
    }

    //---------------------------------------------------------------------
    TyphoonApplication::TyphoonApplication()
        : m_GameEntityManager( nullptr )
        , m_GraphicsState( nullptr )
        , m_GraphicsSystem( nullptr )
        , m_LogicState( nullptr )
        , m_LogicSystem( nullptr )
        , m_Barriers( nullptr )
    {
    }

    //---------------------------------------------------------------------
    void TyphoonApplication::Run()
    {
        m_GraphicsState = new GraphicsState();
        m_GraphicsSystem = new GraphicsSystem( m_GraphicsState, Ogre::ColourValue(0.32f, 0.54f, 0.9f, 1.f) );
        m_LogicState = new LogicState();
        m_LogicSystem = new LogicSystem( m_LogicState );
        m_Barriers = new Ogre::Barrier( 2 );
        m_GameEntityManager = new GraphicsObjectManager( m_GraphicsSystem, m_LogicSystem );

        m_GraphicsState->SetGraphicSystem( m_GraphicsSystem );
        m_LogicSystem->SetGraphicSystem( m_GraphicsSystem );
        m_GraphicsSystem->SetLogicSystem( m_LogicSystem );
        m_LogicState->SetLogicSystem( m_LogicSystem );

        m_ThreadData.m_GraphicsSystem = m_GraphicsSystem;
        m_ThreadData.m_LogicSystem = m_LogicSystem;
        m_ThreadData.m_Barriers = m_Barriers;

        Ogre::ThreadHandlePtr threadHandles[ ] =
        {
            Ogre::Threads::CreateThread( THREAD_GET( RenderThread ), 0, &m_ThreadData ),
            Ogre::Threads::CreateThread( THREAD_GET( LogicThread ), 1, &m_ThreadData )
        };
        Ogre::Threads::WaitForThreads( 2, threadHandles );        

        SAFE_DELETE( m_GameEntityManager );
        SAFE_DELETE( m_Barriers );
        SAFE_DELETE( m_LogicSystem );
        SAFE_DELETE( m_LogicState );
        SAFE_DELETE( m_GraphicsSystem );
        SAFE_DELETE( m_GraphicsState );
    }
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow )
#else
int mainApp( int argc, const char* argv[ ] )
#endif
{
    TyphoonEngine::TyphoonApplication App;
    App.Run();
    return 0;
}
