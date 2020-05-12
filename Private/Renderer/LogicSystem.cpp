
#include "LogicSystem.h"
#include "IBaseState.h"
#include "SdlInputHandler.h"
#include "GraphicsObjectManager.h"

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreConfigFile.h"

#include "OgreRenderWindow.h"
#include "OgreCamera.h"

#include "Unlit/OgreHlmsUnlit.h"
#include "PBS/OgreHlmsPbs.h"
#include "OgreHlmsManager.h"
#include "OgreArchiveManager.h"

#include "Compositor/OgreCompositorManager2.h"

#include "Overlay/OgreOverlaySystem.h"

#if OGRE_USE_SDL2
    #include <SDL_syswm.h>
#endif

namespace TyphoonEngine
{
    LogicSystem::LogicSystem( IBaseState* InitialState ) 
        : m_GraphicsSystem( nullptr )
        , m_GameEntityManager( nullptr )
        , m_CurrentTransformIdx( 1 )
    {
        //mCurrentTransformIdx is 1, 0 and NUM_GAME_ENTITY_BUFFERS - 1 are taken by GraphicsSytem at startup
        //The range to fill is then [2; NUM_GAME_ENTITY_BUFFERS-1]
        for( Ogre::uint32 i=2; i<NUM_GAME_ENTITY_BUFFERS-1; ++i )
            m_AvailableTransformIdx.push_back( i );
    }
    
    //-----------------------------------------------------------------------------------
    LogicSystem::~LogicSystem()
    {
    }

    //-----------------------------------------------------------------------------------
    void LogicSystem::Init( void )
    {
        if ( m_LogicState )
        {
            m_LogicState->Init();
        }
    }
    
    //-----------------------------------------------------------------------------------
    void LogicSystem::CreateScene( void )
    {
        if ( m_LogicState )
        {
            m_LogicState->CreateScene();
        }
    }
    
    //-----------------------------------------------------------------------------------
    void LogicSystem::BeginFrameParallel( void )
    {
    }
    
    //-----------------------------------------------------------------------------------
    void LogicSystem::FinishFrame( void )
    {
        if ( m_LogicState )
        {
            m_LogicState->FinishFrame();
        }
    }
    
    //-----------------------------------------------------------------------------------
    void LogicSystem::Update( float deltaTime )
    {
        if ( m_LogicState )
        {
            m_LogicState->Update( deltaTime );
        }
    }
    
    //-----------------------------------------------------------------------------------
    void LogicSystem::DestroyScene( void )
    {
        if ( m_LogicState )
        {
            m_LogicState->DestroyScene();
        }
    }
    
    //-----------------------------------------------------------------------------------
    void LogicSystem::Shutdown( void )
    {
        if ( m_LogicState )
        {
            m_LogicState->Shutdown();
        }
    }

    //-----------------------------------------------------------------------------------
    void LogicSystem::FinishFrameParallel(void)
    {
        if( m_GameEntityManager )
            m_GameEntityManager->FinishFrameParallel();

        //Notify the GraphicsSystem we're done rendering this frame.
        if( m_GraphicsSystem )
        {
            size_t idxToSend = m_CurrentTransformIdx;

            if( m_AvailableTransformIdx.empty() )
            {
                //Don't relinquish our only ID left.
                //If you end up here too often, Graphics' thread is too slow,
                //or you need to increase NUM_GAME_ENTITY_BUFFERS
                idxToSend = std::numeric_limits<Ogre::uint32>::max();
            }
            else
            {
                //Until Graphics constantly releases the indices we send them, to avoid writing
                //to transform data that may be in use by the other thread (race condition)
                m_CurrentTransformIdx = m_AvailableTransformIdx.front();
                m_AvailableTransformIdx.pop_front();
            }

            this->QueueSendMessage( m_GraphicsSystem, Mq::LOGICFRAME_FINISHED, idxToSend );
        }

        if ( m_LogicState )
        {
            m_LogicState->FinishFrameParallel();
        }
    }
    
    //-----------------------------------------------------------------------------------
    void LogicSystem::ProcessIncomingMessage( Mq::MessageId messageId, const void *data )
    {
        switch( messageId )
        {
        case Mq::LOGICFRAME_FINISHED:
            {
                Ogre::uint32 newIdx = *reinterpret_cast<const Ogre::uint32*>( data );
                assert( (m_AvailableTransformIdx.empty() ||
                        newIdx == (m_AvailableTransformIdx.back() + 1) % NUM_GAME_ENTITY_BUFFERS) &&
                        "Indices are arriving out of order!!!" );

                m_AvailableTransformIdx.push_back( newIdx );
            }
            break;
        case Mq::GAME_ENTITY_SCHEDULED_FOR_REMOVAL_SLOT:
            m_GameEntityManager->_notifyGameEntitiesRemoved( *reinterpret_cast<const Ogre::uint32*>( data ) );
            break;
        case Mq::SDL_EVENT:
            //TODO
            break;
        default:
            break;
        }
    }
}
