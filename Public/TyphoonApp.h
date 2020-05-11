#pragma once

#include "GraphicsSystem.h"
#include "LogicSystem.h"
#include "GraphicsGameState.h"
#include "GameEntityManager.h"
#include "LogicGameState.h"
#include "YieldTimer.h"

#include <OgreRenderWindow.h>
#include <OgreTimer.h>

#include "Threading/OgreThreads.h"
#include "Threading/OgreBarrier.h"

#include "TyphoonCore.h"

namespace Ogre
{
    class Barrier;
}

namespace TyphoonEngine
{

    struct ThreadData
    {
        Renderer*   m_GraphicsSystem;
        LogicSystem*    m_LogicSystem;
        Ogre::Barrier*  m_Barriers;

        ThreadData()
        {
            ZeroMemory( this, sizeof( ThreadData ) );
        }
    };

    class TyphoonApplication
    {
    public:

        TyphoonApplication();
        virtual ~TyphoonApplication() = default;

        virtual bool Init();
        virtual void Shutdown();
        virtual void Run();

    private:

        GameEntityManager*  m_GameEntityManager;
        GraphicsGameState*  m_GraphicsGameState;
        Renderer*       m_GraphicsSystem;
        AppLogicState*      m_LogicGameState;
        LogicSystem*        m_LogicSystem;
        Ogre::Barrier*      m_Barriers;
        ThreadData          m_ThreadData;
    };
}