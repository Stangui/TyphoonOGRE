#pragma once 

#include "BaseSystem.h"
#include "OgrePrerequisites.h"

namespace TyphoonEngine
{
    class GraphicsObjectManager;

    class LogicSystem : public BaseSystem
    {
    protected:

        BaseSystem* m_GraphicsSystem;
        GraphicsObjectManager* m_GameEntityManager;
        Ogre::uint32 m_CurrentTransformIdx;
        std::deque<Ogre::uint32> m_AvailableTransformIdx;

        /// @see MessageQueueSystem::processIncomingMessage
        virtual void ProcessIncomingMessage( Mq::MessageId messageId, const void* data ) override;

    public:

        LogicSystem( IBaseState* InitialState );
        virtual ~LogicSystem();


        // IBaseSystem interface
        virtual void FinishFrameParallel( void ) override;
        
        inline void SetGraphicSystem( BaseSystem* GraphicsSystem )
        {
            m_GraphicsSystem = GraphicsSystem;
        }

        inline void SetGraphicsObjectManager( GraphicsObjectManager* GEMgr )
        {
            m_GameEntityManager = GEMgr;
        }

        inline GraphicsObjectManager* GetGraphicsObjectManager( void ) const
        {
            return m_GameEntityManager;
        }

        inline Ogre::uint32 GetCurrentTransformIdx( void ) const
        {
            return m_CurrentTransformIdx;
        }

    };
}
