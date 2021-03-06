#pragma once 

#include "Threading/MessageQueueSystem.h"

namespace TyphoonEngine
{

    class IBaseState;

    class BaseSystem : public Mq::MessageQueueSystem
    {

    public:

        BaseSystem( IBaseState* InitialState );
        virtual ~BaseSystem() = default;

        virtual void Init( void );
        virtual void CreateScene( void );
        virtual void Update( float timeSinceLast );
        virtual void BeginFrameParallel( void );
        virtual void FinishFrameParallel( void );
        virtual void FinishFrame( void );
        virtual void DestroyScene( void );
        virtual void Shutdown( void );

    protected:

        IBaseState* m_CurrentState;
    };
}
