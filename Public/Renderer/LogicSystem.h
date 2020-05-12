#pragma once 

#include "IBaseSystem.h"
#include "OgrePrerequisites.h"

namespace TyphoonEngine
{
    class GraphicsObjectManager;
    class IBaseState;

    class LogicSystem : public IBaseSystem
    {
    protected:

        IBaseSystem* m_GraphicsSystem;
        IBaseState* m_LogicState;
        GraphicsObjectManager* m_GameEntityManager;
        Ogre::uint32 m_CurrentTransformIdx;
        std::deque<Ogre::uint32> m_AvailableTransformIdx;

        /// @see MessageQueueSystem::processIncomingMessage
        virtual void ProcessIncomingMessage( Mq::MessageId messageId, const void* data ) override;

    public:

        LogicSystem( IBaseState* InitialState );
        virtual ~LogicSystem();


        // IBaseSystem interface
        virtual void Init( void ) override;
        virtual void CreateScene( void ) override;
        virtual void BeginFrameParallel( void ) override;
        virtual void FinishFrameParallel( void ) override;
        virtual void FinishFrame( void ) override;
        virtual void Update( float deltaTime ) override;
        virtual void DestroyScene( void ) override;
        virtual void Shutdown( void ) override;

        inline void SetGraphicSystem( IBaseSystem* GraphicsSystem )
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
