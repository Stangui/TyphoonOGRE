#include "GraphicsGameState.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "Overlay/OgreTextAreaOverlayElement.h"

extern const double cFrametime;

namespace TyphoonEngine
{
	GraphicsGameState::GraphicsGameState( const Ogre::String &helpDescription ) :
		TyphoonGameState( helpDescription ),
		mEnableInterpolation( true )
	{
	}
	//-----------------------------------------------------------------------------------
	void GraphicsGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
	{
		TyphoonGameState::generateDebugText( timeSinceLast, outText );
		outText += "\nPress F4 to enable interpolation. ";
		outText += mEnableInterpolation ? "[On]" : "[Off]";

		//Show the current weight.
		//The text doesn't get updated every frame while displaying
		//help, so don't show the weight as it is inaccurate.
		if ( mDisplayHelpMode != 0 )
		{
			float weight = mGraphicsSystem->getAccumTimeSinceLastLogicFrame() / cFrametime;
			weight = std::min( 1.0f, weight );

			if ( !mEnableInterpolation )
				weight = 0;

			outText += "\nBlend weight: ";
			outText += Ogre::StringConverter::toString( weight );
		}
	}
	//-----------------------------------------------------------------------------------
	void GraphicsGameState::update( float timeSinceLast )
	{
		float weight = mGraphicsSystem->getAccumTimeSinceLastLogicFrame() / cFrametime;
		weight = std::min( 1.0f, weight );

		if ( !mEnableInterpolation )
			weight = 0;

		mGraphicsSystem->updateGameEntities( mGraphicsSystem->getGameEntities( Ogre::SCENE_DYNAMIC ), weight );

		TyphoonGameState::update( timeSinceLast );
	}
	//-----------------------------------------------------------------------------------
	void GraphicsGameState::keyReleased( const SDL_KeyboardEvent &arg )
	{
		if ( ( arg.keysym.mod & ~( KMOD_NUM | KMOD_CAPS ) ) != 0 )
		{
			TyphoonGameState::keyReleased( arg );
			return;
		}

		if ( arg.keysym.sym == SDLK_F4 )
		{
			mEnableInterpolation = !mEnableInterpolation;
		}
		else
		{
			TyphoonGameState::keyReleased( arg );
		}
	}
}
