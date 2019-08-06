#pragma once

#include "GraphicsSystem.h"

namespace TyphoonEngine
{

	class TyphoonGraphicsSystem : public GraphicsSystem
	{
		// No resources. They're not needed and a potential point of failure.
		// This is a very simple project
		virtual void setupResources( void ) {}

	public:

		TyphoonGraphicsSystem( GameState *gameState ) : GraphicsSystem( gameState )
		{}

	};

}