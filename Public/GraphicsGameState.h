#pragma once

#include "GameState.h"
#include "OgrePrerequisites.h"

namespace TyphoonEngine
{
	class Renderer;

	class GraphicsGameState : public IGameState
	{
	
	private:

		Renderer* mGraphicsSystem;

	public:

		GraphicsGameState();
		virtual ~GraphicsGameState() override {};

		virtual void Init() override {}
		virtual void Shutdown() override {}
		virtual void CreateScene() override;
		virtual void DestroyScene() override {}
		virtual void FinishFrame() override {}
		virtual void FinishFrameParallel() override {}
		virtual void Update( float timeSinceLast ) override;
		inline virtual void SetGraphicSystem( Renderer* sys ) { mGraphicsSystem = sys; }
		
		virtual void keyReleased( const SDL_KeyboardEvent &arg );
	};
}
