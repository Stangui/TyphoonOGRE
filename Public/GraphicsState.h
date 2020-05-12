#pragma once

#include "IBaseState.h"
#include "OgrePrerequisites.h"

namespace TyphoonEngine
{
	class GraphicsSystem;

	class GraphicsGameState : public IBaseState
	{
	
	private:

		GraphicsSystem* m_GraphicsSystem;

	public:

		GraphicsGameState();
		virtual ~GraphicsGameState() override {};

		// IBaseState interface
		virtual void Init() override {}
		virtual void Shutdown() override {}
		virtual void CreateScene() override;
		virtual void DestroyScene() override {}
		virtual void FinishFrame() override {}
		virtual void FinishFrameParallel() override {}
		virtual void Update( float timeSinceLast ) override;

		inline virtual void SetGraphicSystem( GraphicsSystem* sys ) { m_GraphicsSystem = sys; }
		
		virtual void keyReleased( const SDL_KeyboardEvent &arg );
	};
}
