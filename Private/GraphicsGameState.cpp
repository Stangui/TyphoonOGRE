#include "GraphicsGameState.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "TyphoonCore.h"

namespace TyphoonEngine
{

	GraphicsGameState::GraphicsGameState() : mGraphicsSystem( nullptr )
	{
	}

    //-----------------------------------------------------------------------------------
    void GraphicsGameState::CreateScene()
    {
    }


	//-----------------------------------------------------------------------------------
	void GraphicsGameState::Update( float timeSinceLast )
	{
		float weight = std::min( 1.0f, static_cast<float>(mGraphicsSystem->GetAccumTimeSinceLastLogicFrame() / TyphoonEngine::LOGIC_UPDATE_TIME ) );
		mGraphicsSystem->UpdateGameEntities( mGraphicsSystem->GetGameEntities( Ogre::SCENE_DYNAMIC ), weight );
	}

	//-----------------------------------------------------------------------------------
	void GraphicsGameState::keyReleased( const SDL_KeyboardEvent& arg )
	{
		if ( arg.keysym.scancode == SDL_SCANCODE_ESCAPE )
		{
			mGraphicsSystem->SetQuit();
		}

		if ( arg.keysym.scancode == SDL_SCANCODE_D && ( arg.keysym.mod & ( KMOD_LCTRL|KMOD_RCTRL ) ) )
		{
			mGraphicsSystem->ShowDebugText( !mGraphicsSystem->IsDebugTextVisible() );
		}
	}
}
