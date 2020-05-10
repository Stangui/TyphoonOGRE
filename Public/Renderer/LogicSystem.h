#pragma once 

#include "BaseSystem.h"
#include "OgrePrerequisites.h"

namespace TyphoonEngine
{
    class GameEntityManager;

    class LogicSystem : public BaseSystem
    {
    protected:

        BaseSystem* m_GraphicsSystem;
        GameEntityManager* m_GameEntityManager;
        Ogre::uint32 m_CurrentTransformIdx;
        std::deque<Ogre::uint32> m_AvailableTransformIdx;

        /// @see MessageQueueSystem::processIncomingMessage
        virtual void ProcessIncomingMessage( Mq::MessageId messageId, const void* data ) override;

    public:

        LogicSystem( IGameState* InitialState );
        virtual ~LogicSystem();

        // Overrides - IBaseSystem interface
        virtual void FinishFrameParallel() override;

        inline void SetGraphicSystem( BaseSystem* GraphicsSystem )
        {
            m_GraphicsSystem = GraphicsSystem;
        }

        inline void SetGameEntityManager( GameEntityManager* GEMgr )
        {
            m_GameEntityManager = GEMgr;
        }

        inline GameEntityManager* GetGameEntityManager( void ) const
        {
            return m_GameEntityManager;
        }

        inline Ogre::uint32 GetCurrentTransformIdx( void ) const
        {
            return m_CurrentTransformIdx;
        }

    };
}
