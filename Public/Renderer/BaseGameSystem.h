#pragma once 

#include "Threading/MessageQueueSystem.h"

namespace TyphoonEngine
{
	class IGameState;

	class BaseGameSystem : public Mq::MessageQueueSystem
	{

	protected:
		IGameState*	mCurrentGameState;

	public:

		BaseGameSystem( IGameState *gameState );
		virtual ~BaseGameSystem();

		virtual void Initialise( void );
		virtual void Deinitialise( void );

		virtual void CreateScene( void );
		virtual void DestroyScene( void );

		virtual void BeginFrameParallel( void );
		virtual void Update( float timeSinceLast );
		virtual void FinishFrameParallel( void );
		virtual void FinishFrame( void );
	};
}
