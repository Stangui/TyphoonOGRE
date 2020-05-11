#pragma once

#include <cstring>

#include "TyphoonCore.h"

namespace Ogre
{
    class Barrier;
}

namespace TyphoonEngine
{

    class GameEntityManager;
    class GraphicsGameState;    
    class GraphicsSystem;
    class AppLogicState;
    class LogicSystem;

    struct ThreadData
    {
        GraphicsSystem*   m_GraphicsSystem;
        LogicSystem*    m_LogicSystem;
        Ogre::Barrier*  m_Barriers;

        ThreadData()
        {
            memset( this, 0, sizeof( ThreadData ) );
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
        GraphicsSystem*       m_GraphicsSystem;
        AppLogicState*      m_LogicGameState;
        LogicSystem*        m_LogicSystem;
        Ogre::Barrier*      m_Barriers;
        ThreadData          m_ThreadData;
    };
}