#pragma once 

#include "BaseGameSystem.h"
#include "GameEntityManager.h"
#include "System/StaticPluginLoader.h"
#include "OgrePrerequisites.h"
#include "OgreColourValue.h"
#include "Overlay/OgreOverlayPrerequisites.h"

#include "Threading/OgreUniformScalableTask.h"
#include "SdlEmulationLayer.h"
#include "Overlay/OgreOverlaySystem.h"

#if OGRE_USE_SDL2
    #include <SDL.h>
#endif

namespace TyphoonEngine
{
    class SdlInputHandler;

    class GraphicsSystem : public BaseGameSystem, public Ogre::UniformScalableTask
    {
    protected:
        BaseGameSystem*				mLogicSystem;

    #if OGRE_USE_SDL2
        SDL_Window*					mSdlWindow;
        SdlInputHandler*			mInputHandler;
    #endif

        Ogre::Root*					mRoot;
        Ogre::RenderWindow*			mRenderWindow;
        Ogre::SceneManager*			mSceneManager;
        Ogre::Camera*				mCamera;
        Ogre::CompositorWorkspace*	mWorkspace;
        Ogre::String                mPluginsFolder;
        Ogre::String                mWriteAccessFolder;
        Ogre::String                mResourcePath;

        Ogre::v1::OverlaySystem*	mOverlaySystem;

        StaticPluginLoader          mStaticPluginLoader;

        /// Tracks the amount of elapsed time since we last
        /// heard from the LogicSystem finishing a frame
        float               mAccumTimeSinceLastLogicFrame;
        Ogre::uint32        mCurrentTransformIdx;
        GameEntityVec       mGameEntities[Ogre::NUM_SCENE_MEMORY_MANAGER_TYPES];
        GameEntityVec const *mThreadGameEntityToUpdate;
        float               mThreadWeight;

        bool                mQuit;
        bool                mAlwaysAskForConfig;
        bool                mUseHlmsDiskCache;
        bool                mUseMicrocodeCache;

        Ogre::ColourValue   mBackgroundColour;

    #if OGRE_USE_SDL2
        void HandleWindowEvent( const SDL_Event& evt );
    #endif

        bool IsWriteAccessFolder( const Ogre::String &folderPath, const Ogre::String &fileToSave );

        /// @see MessageQueueSystem::processIncomingMessage
        virtual void ProcessIncomingMessage( Mq::MessageId messageId, const void *data );

        static void AddResourceLocation( const Ogre::String &archName, const Ogre::String &typeName, const Ogre::String &secName );

		void LoadHlmsDiskCache(void);
        void SaveHlmsDiskCache(void);
        virtual void SetupResources(void);
        virtual void RegisterHlms(void);
        
		/// Optional override method where you can perform resource group loading
        /// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        virtual void LoadResources(void);
        virtual void ChooseSceneManager(void);
        virtual void CreateCamera(void);
        
		/// Virtual so that advanced samples such as Sample_Compositor can override this
        /// method to change the default behavior if setupCompositor() is overridden, be
        /// aware @mBackgroundColour will be ignored
        virtual Ogre::CompositorWorkspace* SetupCompositor(void);

        /// Optional override method where you can create resource listeners (e.g. for loading screens)
        virtual void CreateResourceListener(void) {}

		void GameEntityAdded( const GameEntityManager::CreatedGameEntity *createdGameEntity );
        void GameEntityRemoved( GameEntity* toRemove );
    
	public:
    
		GraphicsSystem( IGameState *gameState, Ogre::ColourValue backgroundColour = Ogre::ColourValue( 0.2f, 0.4f, 0.6f ) );
        virtual ~GraphicsSystem() override;

        inline void SetLogicSystem( BaseGameSystem *logicSystem )	{ mLogicSystem = logicSystem; }

        virtual void Init( void ) override;
        virtual void Shutdown( void ) override;

		virtual void Update( float deltaTime ) override;

        /** Updates the SceneNodes of all the game entities in the container,
            interpolating them according to weight, reading the transforms from
            mCurrentTransformIdx and mCurrentTransformIdx-1.
        @param gameEntities
            The container with entities to update.
        @param weight
            The interpolation weight, ideally in range [0; 1]
        */
        void UpdateGameEntities( const GameEntityVec &gameEntities, float weight );

        /// Overload Ogre::UniformScalableTask. @see updateGameEntities
        virtual void execute( size_t threadId, size_t numThreads );

        /// Returns the GameEntities that are ready to be rendered. May include entities
        /// that are scheduled to be removed (i.e. they are no longer updated by logic)
        inline const GameEntityVec& GetGameEntities( Ogre::SceneMemoryMgrTypes type ) const { return mGameEntities[type]; }

    #if OGRE_USE_SDL2
        inline SdlInputHandler* GetInputHandler(void)                  { return mInputHandler; }
    #endif

        inline void SetQuit(void)                                      { mQuit = true; }
        inline bool GetQuit(void) const                                { return mQuit; }

		inline float GetAccumTimeSinceLastLogicFrame(void) const       { return mAccumTimeSinceLastLogicFrame; }

		inline Ogre::Root* GetRoot(void) const                         { return mRoot; }
		inline Ogre::RenderWindow* GetRenderWindow(void) const         { return mRenderWindow; }
		inline Ogre::SceneManager* GetSceneManager(void) const         { return mSceneManager; }
		inline Ogre::Camera* GetCamera(void) const                     { return mCamera; }
		inline Ogre::CompositorWorkspace* GetCompositorWorkspace(void) const { return mWorkspace; }
		inline Ogre::v1::OverlaySystem* GetOverlaySystem(void) const   { return mOverlaySystem; }

		inline const Ogre::String& GetPluginsFolder(void) const        { return mPluginsFolder; }
		inline const Ogre::String& GetWriteAccessFolder(void) const    { return mWriteAccessFolder; }
		inline const Ogre::String& GetResourcePath(void) const         { return mResourcePath; }

        virtual void StopCompositor(void);
        virtual void RestartCompositor(void);
    };
}
