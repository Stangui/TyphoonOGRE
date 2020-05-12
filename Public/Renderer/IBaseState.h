#pragma once 

#include "InputListeners.h"

namespace TyphoonEngine
{

	/**
	* Game state interface
	*/
    class IBaseState : public MouseListener, public KeyboardListener, public JoystickListener
    {
    public:
        
		IBaseState() = default;
		virtual ~IBaseState() = default;

		virtual void Init( void ) = 0;
		virtual void Shutdown( void ) = 0;

		virtual void CreateScene( void ) = 0;
		virtual void DestroyScene( void ) = 0;

		virtual void Update( float timeSinceLast ) = 0;
		virtual void FinishFrameParallel( void ) = 0;
		virtual void FinishFrame( void ) = 0;

    };
}