#pragma once 

#include "Threading/MessageQueueSystem.h"

namespace TyphoonEngine
{

    class IBaseSystem : public Mq::MessageQueueSystem
    {

    public:

        IBaseSystem() = default;
        virtual ~IBaseSystem() = default;

        virtual void Init( void ) = 0;
        virtual void CreateScene( void ) = 0;
        virtual void Update( float timeSinceLast ) = 0;
        virtual void BeginFrameParallel( void ) = 0;
        virtual void FinishFrameParallel( void ) = 0;
        virtual void FinishFrame( void ) = 0;
        virtual void DestroyScene( void ) = 0;
        virtual void Shutdown( void ) = 0;

    };
}
