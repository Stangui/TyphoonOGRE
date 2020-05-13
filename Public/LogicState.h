#pragma once

#include "OgrePrerequisites.h"
#include "IBaseState.h"

namespace TyphoonEngine
{
    class LogicSystem;
    struct GraphicsObject;
    struct MovableObjectDefinition;

    class LogicState : public IBaseState
    {
        float                       mDisplacement;
        GraphicsObject*             mCubeEntity;
        MovableObjectDefinition*    mCubeMoDef;
        LogicSystem*                mLogicSystem;

    public:

        LogicState();
        virtual ~LogicState() override;

        void Init() override
        {
        };
        void Shutdown() override
        {
        };
        virtual void DestroyScene() override
        {
        }
        virtual void FinishFrame() override
        {
        }
        virtual void FinishFrameParallel() override
        {
        }

        inline void SetLogicSystem( LogicSystem* logicSystem )
        {
            mLogicSystem = logicSystem;
        }

        virtual void CreateScene( void ) override;
        virtual void Update( float timeSinceLast ) override;
    };
}
