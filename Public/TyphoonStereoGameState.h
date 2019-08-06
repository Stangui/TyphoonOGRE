#pragma once

#include "OgrePrerequisites.h"
#include "TyphoonGameState.h"

namespace TyphoonEngine
{
	class TyphoonStereoGameState : public TyphoonGameState
	{
		Ogre::SceneNode     *mSceneNode[ 16 ];

	public:
		TyphoonStereoGameState( const Ogre::String &helpDescription );

		virtual void createScene01( void );

		virtual void update( float timeSinceLast );
	};
}
