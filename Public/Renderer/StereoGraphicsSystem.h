#pragma once

#include "GraphicsSystem.h"

namespace TyphoonEngine
{
	class IBaseState;

	class StereoGraphicsSystem : public GraphicsSystem
	{
		Ogre::SceneNode* mCamerasNode;
		Ogre::Camera* mEyeCameras[ 2 ];
		Ogre::CompositorWorkspace* mEyeWorkspaces[ 2 ];

		virtual void createCamera( void );
		virtual Ogre::CompositorWorkspace* setupCompositor();

	public:

		StereoGraphicsSystem( IBaseState* GameState );
	};
}