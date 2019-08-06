#pragma once 

#include "InputListeners.h"

namespace TyphoonEngine
{
    class GameState : public MouseListener, public KeyboardListener, public JoystickListener
    {
    public:
        virtual ~GameState() {}

        virtual void initialize(void) {}
        virtual void deinitialize(void) {}

        virtual void createScene01(void) {}
        virtual void createScene02(void) {}

        virtual void destroyScene(void) {}

        virtual void update( float timeSinceLast ) {}
        virtual void finishFrameParallel(void) {}
        virtual void finishFrame(void) {}
    };
}