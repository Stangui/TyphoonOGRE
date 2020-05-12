#include "GraphicsState.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "TyphoonCore.h"

namespace TyphoonEngine
{

	GraphicsGameState::GraphicsGameState() : m_GraphicsSystem( nullptr )
	{
	}

    //-----------------------------------------------------------------------------------
    void GraphicsGameState::CreateScene()
    {
    }


	//-----------------------------------------------------------------------------------
	void GraphicsGameState::Update( float timeSinceLast )
	{
		float weight = std::min( 1.0f, static_cast<float>(m_GraphicsSystem->GetAccumTimeSinceLastLogicFrame() / TyphoonEngine::LOGIC_UPDATE_TIME ) );
		m_GraphicsSystem->UpdateGameEntities( m_GraphicsSystem->GetGameEntities( Ogre::SCENE_DYNAMIC ), weight );
	}

	//-----------------------------------------------------------------------------------
	void GraphicsGameState::keyReleased( const SDL_KeyboardEvent& arg )
	{
		if ( arg.keysym.scancode == SDL_SCANCODE_ESCAPE )
		{
			m_GraphicsSystem->SetQuit();
		}

		if ( arg.keysym.scancode == SDL_SCANCODE_D && ( arg.keysym.mod & ( KMOD_LCTRL|KMOD_RCTRL ) ) )
		{
			m_GraphicsSystem->ShowDebugText( !m_GraphicsSystem->IsDebugTextVisible() );
		}
	}
}
