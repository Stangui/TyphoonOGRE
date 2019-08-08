#include "BaseGameSystem.h"
#include "GameState.h"

namespace TyphoonEngine
{
    BaseGameSystem::BaseGameSystem( IGameState *gameState ) : mCurrentGameState( gameState )
    {
    }
    //-----------------------------------------------------------------------------------
    BaseGameSystem::~BaseGameSystem()
    {
    }
    //-----------------------------------------------------------------------------------
    void BaseGameSystem::Initialise(void)
    {
        mCurrentGameState->Initialise();
    }
    //-----------------------------------------------------------------------------------
    void BaseGameSystem::Deinitialise(void)
    {
        mCurrentGameState->Deinitialise();
    }
    //-----------------------------------------------------------------------------------
    void BaseGameSystem::CreateScene(void)
    {
        mCurrentGameState->CreateScene();
    }
    //-----------------------------------------------------------------------------------
    void BaseGameSystem::DestroyScene(void)
    {
        mCurrentGameState->DestroyScene();
    }
    //-----------------------------------------------------------------------------------
    void BaseGameSystem::BeginFrameParallel(void)
    {
        this->ProcessIncomingMessages();
    }
    //-----------------------------------------------------------------------------------
    void BaseGameSystem::Update( float timeSinceLast )
    {
        mCurrentGameState->Update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void BaseGameSystem::FinishFrameParallel(void)
    {
        mCurrentGameState->FinishFrameParallel();

        this->FlushQueuedMessages();
    }
    //-----------------------------------------------------------------------------------
    void BaseGameSystem::FinishFrame(void)
    {
        mCurrentGameState->FinishFrame();
    }
}
