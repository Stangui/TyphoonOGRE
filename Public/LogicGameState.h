#pragma once

#include "OgrePrerequisites.h"
#include "GameState.h"

namespace TyphoonEngine
{
	class LogicSystem;
	struct GameEntity;
	struct MovableObjectDefinition;

	class LogicGameState : public IGameState
	{
		float						mDisplacement;
		GameEntity*					mCubeEntity;
		MovableObjectDefinition*	mCubeMoDef;
		LogicSystem*				mLogicSystem;

	public:

		LogicGameState();
		virtual ~LogicGameState() override;

		void Initialise() override {};
		void Deinitialise() override {};
		virtual void DestroyScene() override {}
		virtual void FinishFrame() override {}
		virtual void FinishFrameParallel() override {}

		inline void SetLogicSystem( LogicSystem* logicSystem ) { mLogicSystem = logicSystem; }

		virtual void CreateScene( void ) override;
		virtual void Update( float timeSinceLast ) override;
	};
}
