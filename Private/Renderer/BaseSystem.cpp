#include "IBaseSystem.h"
#include "IGameState.h"

namespace TyphoonEngine
{
    BaseSystem::BaseSystem( IGameState* InitialState ) 
        : m_CurrentState( InitialState )
    {
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::Init(void)
    {
        if ( m_CurrentState )
            m_CurrentState->Init();
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::Shutdown(void)
    {
        if ( m_CurrentState )
            m_CurrentState->Shutdown();
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::CreateScene(void)
    {
        if ( m_CurrentState )
            m_CurrentState->CreateScene();
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::DestroyScene(void)
    {
        if ( m_CurrentState )
            m_CurrentState->DestroyScene();
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::BeginFrameParallel(void)
    {
        this->ProcessIncomingMessages();
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::Update( float timeSinceLast )
    {
        if ( m_CurrentState )
            m_CurrentState->Update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::FinishFrameParallel(void)
    {
        if ( m_CurrentState )
            m_CurrentState->FinishFrameParallel();

        this->FlushQueuedMessages();
    }
    //-----------------------------------------------------------------------------------
    void BaseSystem::FinishFrame(void)
    {
        if ( m_CurrentState )
            m_CurrentState->FinishFrame();
    }
}
