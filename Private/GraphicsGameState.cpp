#include "GraphicsGameState.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "Overlay/OgreTextAreaOverlayElement.h"

extern const double cFrametime;

namespace TyphoonEngine
{

	GraphicsGameState::GraphicsGameState( const Ogre::String &helpDescription ) 
	{
	}

	//-----------------------------------------------------------------------------------
	void GraphicsGameState::Update( float timeSinceLast )
	{
		float weight = std::min( 1.0f, static_cast<float>(mGraphicsSystem->GetAccumTimeSinceLastLogicFrame() / cFrametime) );
		mGraphicsSystem->UpdateGameEntities( mGraphicsSystem->GetGameEntities( Ogre::SCENE_DYNAMIC ), weight );
	}

	//-----------------------------------------------------------------------------------
	void GraphicsGameState::keyReleased( const SDL_KeyboardEvent &arg )
	{
		if ( ( arg.keysym.mod & ~( KMOD_NUM | KMOD_CAPS ) ) != 0 )
		{
			return;
		}

		if ( arg.keysym.sym == SDLK_ESCAPE )
		{
			mGraphicsSystem->SetQuit();
		}
	}
}
