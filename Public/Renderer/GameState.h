#pragma once 

#include "InputListeners.h"

namespace TyphoonEngine
{

	/**
	* Game state interface
	*/
    class IGameState : public MouseListener, public KeyboardListener, public JoystickListener
    {
    public:
        
		IGameState() = default;
		virtual ~IGameState() {}

		virtual void Init( void ) = 0;
		virtual void Shutdown( void ) = 0;

		virtual void CreateScene( void ) = 0;
		virtual void DestroyScene( void ) = 0;

		virtual void Update( float timeSinceLast ) = 0;
		virtual void FinishFrameParallel( void ) = 0;
		virtual void FinishFrame( void ) = 0;

    };
}