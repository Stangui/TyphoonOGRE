#pragma once

#include <cstring>

#include "TyphoonCore.h"

namespace Ogre
{
    class Barrier;
}

namespace TyphoonEngine
{

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
        virtual void Run();

    private:

        GraphicsObjectManager*  m_GameEntityManager;
        GraphicsState*          m_GraphicsState;
        GraphicsSystem*         m_GraphicsSystem;
        LogicState*             m_LogicState;
        LogicSystem*            m_LogicSystem;
        Ogre::Barrier*          m_Barriers;
        ThreadData              m_ThreadData;
    };
}