#pragma once

#include "OgrePrerequisites.h"
#include "TyphoonGameState.h"

namespace TyphoonEngine
{
	class GraphicsSystem;

	class GraphicsGameState : public TyphoonGameState
	{
	
	private:

		bool        mEnableInterpolation;

		virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

	public:
		GraphicsGameState( const Ogre::String &helpDescription );

		virtual void update( float timeSinceLast );

		virtual void keyReleased( const SDL_KeyboardEvent &arg );
	};
}
