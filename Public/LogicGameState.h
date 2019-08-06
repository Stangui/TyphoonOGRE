#pragma once

#include "OgrePrerequisites.h"
#include "TyphoonGameState.h"

namespace TyphoonEngine
{
	class LogicSystem;
	struct GameEntity;
	struct MovableObjectDefinition;

	class LogicGameState : public GameState
	{
		float               mDisplacement;
		GameEntity              *mCubeEntity;
		MovableObjectDefinition *mCubeMoDef;

		LogicSystem         *mLogicSystem;

	public:
		LogicGameState();
		~LogicGameState();

		void _notifyLogicSystem( LogicSystem *logicSystem ) { mLogicSystem = logicSystem; }

		virtual void createScene01( void ) override;
		virtual void update( float timeSinceLast ) override;
	};
}
