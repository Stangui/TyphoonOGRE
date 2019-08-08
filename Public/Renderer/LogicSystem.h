#pragma once 

#include "BaseGameSystem.h"
#include "OgrePrerequisites.h"

namespace TyphoonEngine
{
    class GameEntityManager;

    class LogicSystem : public BaseGameSystem
    {
    protected:
        BaseGameSystem*				mGraphicsSystem;
        GameEntityManager*			mGameEntityManager;

        Ogre::uint32                mCurrentTransformIdx;
        std::deque<Ogre::uint32>    mAvailableTransformIdx;

        /// @see MessageQueueSystem::processIncomingMessage
        virtual void ProcessIncomingMessage( Mq::MessageId messageId, const void *data ) override;

    public:
        LogicSystem( IGameState *gameState );
        virtual ~LogicSystem();

		inline void SetGraphicSystem( BaseGameSystem *graphicsSystem )		{ mGraphicsSystem = graphicsSystem; }
		inline void SetGameEntityManager( GameEntityManager *mgr )			{ mGameEntityManager = mgr; }

        void FinishFrameParallel(void) override;

        inline GameEntityManager* GetGameEntityManager(void)               { return mGameEntityManager; }
        inline Ogre::uint32 GetCurrentTransformIdx(void) const             { return mCurrentTransformIdx; }
    };
}
