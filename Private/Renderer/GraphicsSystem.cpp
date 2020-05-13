
#include "GraphicsSystem.h"
#include "TyphoonCore.h"

#include "IBaseState.h"
#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
#include "SdlInputHandler.h"
#endif
#include "GraphicsObject.h"

#include "OgreRoot.h"
#include "OgreException.h"
#include "OgreConfigFile.h"
#include "OgreFrameStats.h"

#include "OgreRenderWindow.h"
#include "OgreCamera.h"
#include "OgreItem.h"

#include "Unlit/OgreHlmsUnlit.h"
#include "PBS/OgreHlmsPbs.h"
#include "OgreHlmsManager.h"
#include "OgreArchiveManager.h"

#include "Compositor/OgreCompositorManager2.h"

#include "Overlay/OgreOverlay.h"
#include "Overlay/OgreOverlayManager.h"
#include "Overlay/OgreOverlaySystem.h"
#include "Overlay/OgreTextAreaOverlayElement.h"
#include "Overlay/OgreOverlayContainer.h"

#include "OgreWindowEventUtilities.h"

#include "OgreFileSystemLayer.h"

#include "OgreHlmsDiskCache.h"
#include "OgreGpuProgramManager.h"

#include "OgreLogManager.h"

#include <SDL_syswm.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#include "OSX/macUtils.h"
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
#include "System/iOS/iOSUtils.h"
#else
#include "System/OSX/OSXUtils.h"
#endif
#endif

#include <iostream>

namespace TyphoonEngine
{

    static const char* CONFIGS_FOLDER = ".\\Configs\\";
  
    GraphicsSystem::GraphicsSystem( IBaseState* InitialState, Ogre::ColourValue backgroundColour )
        : BaseSystem( InitialState )
        , mLogicSystem( nullptr )
        , mSdlWindow( 0 )
        , mInputHandler( 0 )
        , mRoot( nullptr )
        , mRenderWindow( nullptr )
        , mSceneManager( nullptr )
        , mCamera( nullptr )
        , mWorkspace( nullptr )
        , mConfigsFolder( CONFIGS_FOLDER )
        , mSaveFolder("")
        , mResourcePath( CONFIGS_FOLDER )
        , mOverlaySystem( nullptr )
        , mDebugPanel( nullptr )
        , mAccumTimeSinceLastLogicFrame( 0 )
        , mCurrentTransformIdx( 0 )
        , mThreadGameEntityToUpdate( nullptr )
        , mThreadWeight( 0 )
        , mQuit( 0 )
        , mUseHlmsDiskCache( 1 )
        , mUseMicrocodeCache( 1 )
        , bShowDebug( 0 )
        , mBackgroundColour( backgroundColour )
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        // Note:  macBundlePath works for iOS too. It's misnamed.
        mResourcePath = Ogre::macBundlePath()+"/Contents/Resources/";
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        mResourcePath = Ogre::macBundlePath()+"/";
#endif
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        mPluginsFolder = mResourcePath;
#endif
        Ogre::FileSystemLayer filesystemLayer( OGRE_VERSION_NAME );
        mSaveFolder = filesystemLayer.getWritablePath( "" );
    }

    //-----------------------------------------------------------------------------------
    GraphicsSystem::~GraphicsSystem()
    {
        if ( mRoot )
        {
            Ogre::LogManager::getSingleton().logMessage(
                "WARNING: GraphicsSystem::deinitialize() not called!!!", Ogre::LML_CRITICAL );
        }
    }

    //-----------------------------------------------------------------------------------
    bool GraphicsSystem::IsWriteAccessFolder( const Ogre::String& folderPath, const Ogre::String& fileToSave )
    {
        if ( !Ogre::FileSystemLayer::createDirectory( folderPath ) )
            return false;

        std::ofstream of( ( folderPath+fileToSave ).c_str(), std::ios::out|std::ios::binary|std::ios::app );
        if ( !of )
            return false;

        return true;
    }

    //-----------------------------------------------------------------------------------
    bool GraphicsSystem::ParseConfigs( const char* configFilepath )
    {
        return false;
    }

    //-----------------------------------------------------------------------------------
    void GraphicsSystem::Init()
    {
        if ( SDL_Init(
            SDL_INIT_TIMER|
            SDL_INIT_VIDEO|
            SDL_INIT_JOYSTICK|
            SDL_INIT_GAMECONTROLLER|
            SDL_INIT_EVENTS )!=0 )
        {
            OGRE_EXCEPT(
                Ogre::Exception::ERR_INTERNAL_ERROR,
                "Cannot initialize SDL2!",
                "GraphicsSystem::initialize" );
        }

        Ogre::String pluginsPath;
        // only use plugins.cfg if not static
#ifndef OGRE_STATIC_LIB
#if OGRE_DEBUG_MODE && !((OGRE_PLATFORM == OGRE_PLATFORM_APPLE) || (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS))
        pluginsPath = mConfigsFolder+"plugins_d.cfg";
#else
        pluginsPath = mConfigsFolder+"plugins.cfg";
#endif
#endif

        mRoot = OGRE_NEW Ogre::Root( 
            pluginsPath,
            mConfigsFolder+"Ogre.cfg",
            mSaveFolder+"Ogre.log" );

        if ( !mRoot->restoreConfig() )
        {
            if ( !mRoot->showConfigDialog() )
            {
                mQuit = true;
                return;
            }
        }

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        {
            Ogre::RenderSystem* renderSystem = mRoot->getRenderSystemByName( "Metal Rendering Subsystem" );
            mRoot->setRenderSystem( renderSystem );
        }
#endif

        mRoot->getRenderSystem()->setConfigOption( "sRGB Gamma Conversion", "Yes" );
        mRoot->initialise( false );

        Ogre::ConfigOptionMap& cfgOpts = mRoot->getRenderSystem()->getConfigOptions();

        int width = DEFAULT_WINDOW_WIDTH;
        int height = DEFAULT_WINDOW_HEIGHT;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        {
            Ogre::Vector2 screenRes = iOSUtils::getScreenResolutionInPoints();
            width = static_cast< int >( screenRes.x );
            height = static_cast< int >( screenRes.y );
        }
#endif

        Ogre::ConfigOptionMap::iterator opt = cfgOpts.find( "Video Mode" );
        if ( opt!=cfgOpts.end() )
        {
            //Ignore leading space
            const Ogre::String::size_type start = opt->second.currentValue.find_first_of( "012356789" );
            //Get the width and height
            Ogre::String::size_type widthEnd = opt->second.currentValue.find( ' ', start );
            // we know that the height starts 3 characters after the width and goes until the next space
            Ogre::String::size_type heightEnd = opt->second.currentValue.find( ' ', widthEnd+3 );
            // Now we can parse out the values
            width = Ogre::StringConverter::parseInt( opt->second.currentValue.substr( 0, widthEnd ) );
            height = Ogre::StringConverter::parseInt( opt->second.currentValue.substr( widthEnd+3, heightEnd ) );
        }

        Ogre::NameValuePairList params;
        bool fullscreen = Ogre::StringConverter::parseBool( cfgOpts[ "Full Screen" ].currentValue );
        int screen = 0;
        int posX = SDL_WINDOWPOS_CENTERED_DISPLAY( screen );
        int posY = SDL_WINDOWPOS_CENTERED_DISPLAY( screen );

        if ( fullscreen )
        {
            posX = SDL_WINDOWPOS_UNDEFINED_DISPLAY( screen );
            posY = SDL_WINDOWPOS_UNDEFINED_DISPLAY( screen );
        }

        const Ogre::String windowTitle = "Typhoon Engine Window";
        mSdlWindow = SDL_CreateWindow(
            windowTitle.c_str(),// window title
            posX,               // initial x position
            posY,               // initial y position
            width,              // width, in pixels
            height,             // height, in pixels
            SDL_WINDOW_SHOWN
            |( fullscreen ? SDL_WINDOW_FULLSCREEN : 0 )|SDL_WINDOW_RESIZABLE );

        //Get the native whnd
        SDL_SysWMinfo wmInfo;
        SDL_VERSION( &wmInfo.version );

        if ( SDL_GetWindowWMInfo( mSdlWindow, &wmInfo )==SDL_FALSE )
        {
            OGRE_EXCEPT( Ogre::Exception::ERR_INTERNAL_ERROR,
                "Couldn't get WM Info! (SDL2)",
                "GraphicsSystem::initialize" );
        }

        Ogre::String winHandle;
        switch ( wmInfo.subsystem )
        {
#ifdef WIN32
        case SDL_SYSWM_WINDOWS:
            // Windows code
            winHandle = Ogre::StringConverter::toString( ( uintptr_t )wmInfo.info.win.window );
            break;
#elif __MACOSX__
        case SDL_SYSWM_COCOA:
            //required to make OGRE play nice with our window
            params.insert( std::make_pair( "macAPICocoaUseNSView", "true" ) );

            winHandle = Ogre::StringConverter::toString( WindowContentViewHandle( wmInfo ) );
            break;
#else
        case SDL_SYSWM_X11:
            winHandle = Ogre::StringConverter::toString( ( uintptr_t )wmInfo.info.x11.window );
            break;
#endif
        default:
            OGRE_EXCEPT( Ogre::Exception::ERR_NOT_IMPLEMENTED,
                "Unexpected WM! (SDL2)",
                "GraphicsSystem::initialize" );
            break;
        }

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        params.insert( std::make_pair( "externalWindowHandle", winHandle ) );
#else
        params.insert( std::make_pair( "parentWindowHandle", winHandle ) );
#endif

        params.insert( std::make_pair( "title", windowTitle ) );
        params.insert( std::make_pair( "gamma", "true" ) );
        params.insert( std::make_pair( "FSAA", cfgOpts[ "FSAA" ].currentValue ) );
        params.insert( std::make_pair( "vsync", cfgOpts[ "VSync" ].currentValue ) );

        mRenderWindow = Ogre::Root::getSingleton().createRenderWindow( windowTitle, width, height, fullscreen, &params );

        mOverlaySystem = OGRE_NEW Ogre::v1::OverlaySystem();

        SetupResources();
        LoadResources();
        ChooseSceneManager();
        CreateCamera();
        mWorkspace = SetupCompositor();

        mInputHandler = new SdlInputHandler( mSdlWindow, m_CurrentState, m_CurrentState, m_CurrentState );

        BaseSystem::Init();

#if OGRE_PROFILING
        Ogre::Profiler::getSingleton().setEnabled( true );
#if OGRE_PROFILING == OGRE_PROFILING_INTERNAL
        Ogre::Profiler::getSingleton().endProfile( "" );
#endif
#if OGRE_PROFILING == OGRE_PROFILING_INTERNAL_OFFLINE
        Ogre::Profiler::getSingleton().getOfflineProfiler().setDumpPathsOnShutdown(
            mWriteAccessFolder+"ProfilePerFrame",
            mWriteAccessFolder+"ProfileAccum" );
#endif
#endif
    }

    //-----------------------------------------------------------------------------------
    void GraphicsSystem::ShowDebugText( bool bShow )
    {
        bShowDebug = bShow;
        if ( mDebugPanel )
        {
            bShow ? mDebugPanel->show() : mDebugPanel->hide();
        }
    }

    //-----------------------------------------------------------------------------------
    void GraphicsSystem::createDebugTextOverlay( void )
    {
        Ogre::v1::OverlayManager& overlayManager = Ogre::v1::OverlayManager::getSingleton();
        mDebugPanel = overlayManager.create( "DebugText" );

        Ogre::v1::OverlayContainer* panel = static_cast< Ogre::v1::OverlayContainer* >(
            overlayManager.createOverlayElement( "Panel", "DebugPanel" ) );
        mDebugText = static_cast< Ogre::v1::TextAreaOverlayElement* >(
            overlayManager.createOverlayElement( "TextArea", "DebugText" ) );
        mDebugText->setFontName( "DebugFont" );
        mDebugText->setCharHeight( 0.025f );

        mDebugTextShadow = static_cast< Ogre::v1::TextAreaOverlayElement* >(
            overlayManager.createOverlayElement( "TextArea", "0DebugTextShadow" ) );
        mDebugTextShadow->setFontName( "DebugFont" );
        mDebugTextShadow->setCharHeight( 0.025f );
        mDebugTextShadow->setColour( Ogre::ColourValue::Black );
        mDebugTextShadow->setPosition( 0.001f, 0.001f );

        panel->addChild( mDebugTextShadow );
        panel->addChild( mDebugText );
        mDebugPanel->add2D( panel );
        mDebugPanel->show();
    }

    //-----------------------------------------------------------------------------------
    void GraphicsSystem::generateDebugText( float timeSinceLast )
    {
        const Ogre::FrameStats* frameStats = GetRoot()->getFrameStats();

        Ogre::String finalText;
        finalText.reserve( 128 );
        finalText = "Frame time:\t";
        finalText += Ogre::StringConverter::toString( timeSinceLast*1000.f );
        finalText += " ms\n";
        finalText += "Frame FPS:\t";
        finalText += Ogre::StringConverter::toString( 1.f/timeSinceLast );
        finalText += "\nAvg time:\t";
        finalText += Ogre::StringConverter::toString( frameStats->getAvgTime() );
        finalText += " ms\n";
        finalText += "Avg FPS:\t";
        finalText += Ogre::StringConverter::toString( 1000.0f/frameStats->getAvgTime() );
        
        mDebugText->setCaption( finalText );
        mDebugTextShadow->setCaption( finalText );
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::Shutdown( void )
    {
        BaseSystem::Shutdown();

        SaveHlmsDiskCache();

        if ( mSceneManager )
            mSceneManager->removeRenderQueueListener( mOverlaySystem );

        OGRE_DELETE mOverlaySystem;
        mOverlaySystem = nullptr;

        SAFE_DELETE( mInputHandler );

        OGRE_DELETE mRoot;
        mRoot = nullptr;

        if ( mSdlWindow )
        {
            // Restore desktop resolution on exit
            SDL_SetWindowFullscreen( mSdlWindow, 0 );
            SDL_DestroyWindow( mSdlWindow );
            mSdlWindow = nullptr;
        }

        SDL_Quit();
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::Update( float timeSinceLast )
    {
        Ogre::WindowEventUtilities::messagePump();

        SDL_Event evt;
        while ( SDL_PollEvent( &evt ) )
        {
            switch ( evt.type )
            {
            case SDL_WINDOWEVENT:
                HandleWindowEvent( evt );
                break;
            case SDL_QUIT:
                mQuit = true;
                break;
            default:
                break;
            }

            mInputHandler->_handleSdlEvents( evt );
        }

        BaseSystem::Update( timeSinceLast );

        if ( bShowDebug )
        {
            generateDebugText( timeSinceLast );
        }

        if ( mRenderWindow->isVisible() )
            mQuit |= !mRoot->renderOneFrame();

        mAccumTimeSinceLastLogicFrame += timeSinceLast;
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::HandleWindowEvent( const SDL_Event& evt )
    {
        switch ( evt.window.event )
        {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            int w, h;
            SDL_GetWindowSize( mSdlWindow, &w, &h );
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
            mRenderWindow->resize( w, h );
#else
            mRenderWindow->windowMovedOrResized();
#endif
            break;
        case SDL_WINDOWEVENT_RESIZED:
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
            mRenderWindow->resize( evt.window.data1, evt.window.data2 );
#else
            mRenderWindow->windowMovedOrResized();
#endif
            break;
        case SDL_WINDOWEVENT_CLOSE:
            break;
        case SDL_WINDOWEVENT_SHOWN:
            mRenderWindow->setVisible( true );
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            mRenderWindow->setVisible( false );
            break;
        }
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::ProcessIncomingMessage( Mq::MessageId messageId, const void* data )
    {
        switch ( messageId )
        {
        case Mq::LOGICFRAME_FINISHED:
        {
            Ogre::uint32 newIdx = *reinterpret_cast< const Ogre::uint32* >( data );
            if ( newIdx!=std::numeric_limits<Ogre::uint32>::max() )
            {
                mAccumTimeSinceLastLogicFrame = 0;
                //Tell the LogicSystem we're no longer using the index previous to the current one.
                this->QueueSendMessage( 
                    mLogicSystem, 
                    Mq::LOGICFRAME_FINISHED,
                    ( mCurrentTransformIdx+NUM_GAME_ENTITY_BUFFERS-1 ) % NUM_GAME_ENTITY_BUFFERS );

                assert( ( mCurrentTransformIdx+1 ) % NUM_GAME_ENTITY_BUFFERS == newIdx &&
                    "Graphics is receiving indices out of order!!!" );

                //Get the new index the LogicSystem is telling us to use.
                mCurrentTransformIdx = newIdx;
            }
        }
        break;
        case Mq::GAME_ENTITY_ADDED:
            GameEntityAdded( reinterpret_cast< const GraphicsObjectManager::CreatedGameEntity* >( data ) );
            break;
        case Mq::GAME_ENTITY_REMOVED:
            GameEntityRemoved( *reinterpret_cast< GraphicsObject* const* >( data ) );
            break;
        case Mq::GAME_ENTITY_SCHEDULED_FOR_REMOVAL_SLOT:
            //Acknowledge/notify back that we're done with this slot.
            this->QueueSendMessage( 
                mLogicSystem, 
                Mq::GAME_ENTITY_SCHEDULED_FOR_REMOVAL_SLOT,
                *reinterpret_cast< const Ogre::uint32* >( data ) );
            break;
        default:
            break;
        }
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::AddResourceLocation(
        const Ogre::String& archName,
        const Ogre::String& typeName,
        const Ogre::String& secName )
    {
#if (OGRE_PLATFORM == OGRE_PLATFORM_APPLE) || (OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS)
        // OS X does not set the working directory relative to the app,
        // In order to make things portable on OS X we need to provide
        // the loading with it's own bundle path location
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
            Ogre::String( Ogre::macBundlePath()+"/"+archName ), typeName, secName );
#else
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation( archName, typeName, secName );
#endif
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::LoadHlmsDiskCache( void )
    {
        if ( mUseMicrocodeCache==0 && mUseHlmsDiskCache==0 )
            return;

        Ogre::HlmsManager* hlmsManager = mRoot->getHlmsManager();
        Ogre::HlmsDiskCache diskCache( hlmsManager );
        Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();
        Ogre::Archive* rwAccessFolderArchive = archiveManager.load( mSaveFolder, "FileSystem", true );

        if ( mUseMicrocodeCache != 0 )
        {
            //Make sure the microcode cache is enabled.
            Ogre::GpuProgramManager::getSingleton().setSaveMicrocodesToCache( true );
            const Ogre::String filename = "microcodeCodeCache.cache";
            if ( rwAccessFolderArchive->exists( filename ) )
            {
                Ogre::DataStreamPtr shaderCacheFile = rwAccessFolderArchive->open( filename );
                Ogre::GpuProgramManager::getSingleton().loadMicrocodeCache( shaderCacheFile );
            }
        }

        if ( mUseHlmsDiskCache != 0 )
        {
            for ( size_t i = Ogre::HLMS_LOW_LEVEL+1u; i<Ogre::HLMS_MAX; ++i )
            {
                Ogre::Hlms* hlms = hlmsManager->getHlms( static_cast< Ogre::HlmsTypes >( i ) );
                if ( hlms )
                {
                    Ogre::String filename = "hlmsDiskCache"+Ogre::StringConverter::toString( i )+".bin";

                    try
                    {
                        if ( rwAccessFolderArchive->exists( filename ) )
                        {
                            Ogre::DataStreamPtr diskCacheFile = rwAccessFolderArchive->open( filename );
                            diskCache.loadFrom( diskCacheFile );
                            diskCache.applyTo( hlms );
                        }
                    }
                    catch ( Ogre::Exception& )
                    {
                        Ogre::LogManager::getSingleton().logMessage(
                            "Error loading cache from "+mSaveFolder+"/"+
                            filename+"! If you have issues, try deleting the file "
                            "and restarting the app" );
                    }
                }
            }
        }

        archiveManager.unload( mSaveFolder );
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::SaveHlmsDiskCache( void )
    {
        if ( mRoot->getRenderSystem() && 
            Ogre::GpuProgramManager::getSingletonPtr() &&
            ( mUseMicrocodeCache || mUseHlmsDiskCache ) )
        {
            Ogre::HlmsManager* hlmsManager = mRoot->getHlmsManager();
            Ogre::HlmsDiskCache diskCache( hlmsManager );
            Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();
            Ogre::Archive* rwAccessFolderArchive = archiveManager.load( mSaveFolder, "FileSystem", false );

            if ( mUseHlmsDiskCache )
            {
                for ( size_t i = Ogre::HLMS_LOW_LEVEL+1u; i<Ogre::HLMS_MAX; ++i )
                {
                    Ogre::Hlms* hlms = hlmsManager->getHlms( static_cast< Ogre::HlmsTypes >( i ) );
                    if ( hlms )
                    {
                        diskCache.copyFrom( hlms );

                        Ogre::DataStreamPtr diskCacheFile =
                            rwAccessFolderArchive->create( "hlmsDiskCache"+
                                Ogre::StringConverter::toString( i )+
                                ".bin" );
                        diskCache.saveTo( diskCacheFile );
                    }
                }
            }

            if ( Ogre::GpuProgramManager::getSingleton().isCacheDirty()&&mUseMicrocodeCache )
            {
                const Ogre::String filename = "microcodeCodeCache.cache";
                Ogre::DataStreamPtr shaderCacheFile = rwAccessFolderArchive->create( filename );
                Ogre::GpuProgramManager::getSingleton().saveMicrocodeCache( shaderCacheFile );
            }

            archiveManager.unload( mSaveFolder );
        }
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::SetupResources( void )
    {
        // Load resource paths from config file
        Ogre::ConfigFile cf;
        cf.load( mResourcePath+"resources2.cfg" );

        // Go through all sections & settings in the file
        Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

        Ogre::String secName, typeName, archName;
        while ( seci.hasMoreElements() )
        {
            secName = seci.peekNextKey();
            Ogre::ConfigFile::SettingsMultiMap* settings = seci.getNext();

            if ( secName!="Hlms" )
            {
                Ogre::ConfigFile::SettingsMultiMap::iterator i;
                for ( i = settings->begin(); i!=settings->end(); ++i )
                {
                    typeName = i->first;
                    archName = i->second;
                    AddResourceLocation( archName, typeName, secName );
                }
            }
        }
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::RegisterHlms( void )
    {
        Ogre::ConfigFile cf;
        cf.load( mResourcePath+"resources2.cfg" );

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
        Ogre::String rootHlmsFolder = Ogre::macBundlePath()+'/'+
            cf.getSetting( "DoNotUseAsResource", "Hlms", "" );
#else
        Ogre::String rootHlmsFolder = mResourcePath+cf.getSetting( "DoNotUseAsResource", "Hlms", "" );
#endif

        if ( rootHlmsFolder.empty() )
            rootHlmsFolder = "./";
        else if ( *( rootHlmsFolder.end()-1 )!='/' )
            rootHlmsFolder += "/";

        //At this point rootHlmsFolder should be a valid path to the Hlms data folder

        Ogre::HlmsUnlit* hlmsUnlit = 0;
        Ogre::HlmsPbs* hlmsPbs = 0;

        //For retrieval of the paths to the different folders needed
        Ogre::String mainFolderPath;
        Ogre::StringVector libraryFoldersPaths;
        Ogre::StringVector::const_iterator libraryFolderPathIt;
        Ogre::StringVector::const_iterator libraryFolderPathEn;

        Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();

        {
            //Create & Register HlmsUnlit
            //Get the path to all the subdirectories used by HlmsUnlit
            Ogre::HlmsUnlit::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
            Ogre::Archive* archiveUnlit = archiveManager.load( rootHlmsFolder+mainFolderPath, "FileSystem", true );
            Ogre::ArchiveVec archiveUnlitLibraryFolders;
            libraryFolderPathIt = libraryFoldersPaths.begin();
            libraryFolderPathEn = libraryFoldersPaths.end();
            while ( libraryFolderPathIt!=libraryFolderPathEn )
            {
                Ogre::Archive* archiveLibrary =
                    archiveManager.load( rootHlmsFolder+*libraryFolderPathIt, "FileSystem", true );
                archiveUnlitLibraryFolders.push_back( archiveLibrary );
                ++libraryFolderPathIt;
            }

            //Create and register the unlit Hlms
            hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit( archiveUnlit, &archiveUnlitLibraryFolders );
            Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsUnlit );
        }

        {
            //Create & Register HlmsPbs
            //Do the same for HlmsPbs:
            Ogre::HlmsPbs::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
            Ogre::Archive* archivePbs = archiveManager.load( rootHlmsFolder+mainFolderPath, "FileSystem", true );

            //Get the library archive(s)
            Ogre::ArchiveVec archivePbsLibraryFolders;
            libraryFolderPathIt = libraryFoldersPaths.begin();
            libraryFolderPathEn = libraryFoldersPaths.end();
            while ( libraryFolderPathIt!=libraryFolderPathEn )
            {
                Ogre::Archive* archiveLibrary =
                    archiveManager.load( rootHlmsFolder+*libraryFolderPathIt, "FileSystem", true );
                archivePbsLibraryFolders.push_back( archiveLibrary );
                ++libraryFolderPathIt;
            }

            //Create and register
            hlmsPbs = OGRE_NEW Ogre::HlmsPbs( archivePbs, &archivePbsLibraryFolders );
            Ogre::Root::getSingleton().getHlmsManager()->registerHlms( hlmsPbs );
        }


        Ogre::RenderSystem* renderSystem = mRoot->getRenderSystem();
        if ( renderSystem->getName()=="Direct3D11 Rendering Subsystem" )
        {
            //Set lower limits 512kb instead of the default 4MB per Hlms in D3D 11.0
            //and below to avoid saturating AMD's discard limit (8MB) or
            //saturate the PCIE bus in some low end machines.
            bool supportsNoOverwriteOnTextureBuffers;
            renderSystem->getCustomAttribute( "MapNoOverwriteOnDynamicBufferSRV", &supportsNoOverwriteOnTextureBuffers );

            if ( !supportsNoOverwriteOnTextureBuffers )
            {
                hlmsPbs->setTextureBufferDefaultSize( 512*1024 );
                hlmsUnlit->setTextureBufferDefaultSize( 512*1024 );
            }
        }
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::LoadResources( void )
    {
        RegisterHlms();

        LoadHlmsDiskCache();

        // Initialise, parse scripts etc
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups( true );
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::ChooseSceneManager( void )
    {
        Ogre::InstancingThreadedCullingMethod threadedCullingMethod = Ogre::INSTANCING_CULLING_SINGLETHREAD;
#if OGRE_DEBUG_MODE
        //Debugging multithreaded code is a PITA, disable it.
        const size_t numThreads = 1;
#else
        //getNumLogicalCores() may return 0 if couldn't detect
        const size_t numThreads = std::max<size_t>( 1, Ogre::PlatformInformation::getNumLogicalCores() );
        //See doxygen documentation regarding culling methods.
        //In some cases you may still want to use single thread.
        //if( numThreads > 1 )
        //	threadedCullingMethod = Ogre::INSTANCING_CULLING_THREADED;
#endif
        // Create the SceneManager, in this case a generic one
        mSceneManager = mRoot->createSceneManager( Ogre::ST_GENERIC,
            numThreads,
            threadedCullingMethod,
            "ExampleSMInstance" );

        mSceneManager->addRenderQueueListener( mOverlaySystem );
        mSceneManager->getRenderQueue()->setSortRenderQueue(
            Ogre::v1::OverlayManager::getSingleton().mDefaultRenderQueueId,
            Ogre::RenderQueue::StableSort );

        //Set sane defaults for proper shadow mapping
        mSceneManager->setShadowDirectionalLightExtrusionDistance( 500.0f );
        mSceneManager->setShadowFarDistance( 500.0f );
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::CreateCamera( void )
    {
        mCamera = mSceneManager->createCamera( "Main Camera" );

        // Position it at 500 in Z direction
        mCamera->setPosition( Ogre::Vector3( 0, 5, 15 ) );
        // Look back along -Z
        mCamera->lookAt( Ogre::Vector3( 0, 0, 0 ) );
        mCamera->setNearClipDistance( 0.2f );
        mCamera->setFarClipDistance( 1000.0f );
        mCamera->setAutoAspectRatio( true );

        mLight = mSceneManager->createLight();
        mLight->setType( Ogre::Light::LT_DIRECTIONAL );
        mSceneManager->createSceneNode( Ogre::SceneMemoryMgrTypes::SCENE_DYNAMIC)->attachObject( mLight );
        mLight->setDirection( Ogre::Vector3( 0.2f, -1.f, -1.f ).normalisedCopy() );

        createDebugTextOverlay();
    }
    
    //-----------------------------------------------------------------------------------
    Ogre::CompositorWorkspace* GraphicsSystem::SetupCompositor( void )
    {
        Ogre::CompositorManager2* compositorManager = mRoot->getCompositorManager2();
        const Ogre::String workspaceName( "Typhoon Workspace" );
        if ( !compositorManager->hasWorkspaceDefinition( workspaceName ) )
        {
            compositorManager->createBasicWorkspaceDef( workspaceName, mBackgroundColour, Ogre::IdString() );
        }
        return compositorManager->addWorkspace(
            mSceneManager,
            mRenderWindow,
            mCamera,
            workspaceName,
            true
        );
    }
       
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::StopCompositor( void )
    {
        if ( mWorkspace )
        {
            Ogre::CompositorManager2* compositorManager = mRoot->getCompositorManager2();
            compositorManager->removeWorkspace( mWorkspace );
            mWorkspace = nullptr;
        }
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::RestartCompositor( void )
    {
        StopCompositor();
        mWorkspace = SetupCompositor();
    }
    
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    struct GameEntityCmp
    {
        bool operator () ( const GraphicsObject* _l, const Ogre::Matrix4* RESTRICT_ALIAS _r ) const
        {
            const Ogre::Transform& transform = _l->mSceneNode->_getTransform();
            return &transform.mDerivedTransform[ transform.mIndex ]<_r;
        }

        bool operator () ( const Ogre::Matrix4* RESTRICT_ALIAS _r, const GraphicsObject* _l ) const
        {
            const Ogre::Transform& transform = _l->mSceneNode->_getTransform();
            return _r<&transform.mDerivedTransform[ transform.mIndex ];
        }

        bool operator () ( const GraphicsObject* _l, const GraphicsObject* _r ) const
        {
            const Ogre::Transform& lTransform = _l->mSceneNode->_getTransform();
            const Ogre::Transform& rTransform = _r->mSceneNode->_getTransform();
            return &lTransform.mDerivedTransform[ lTransform.mIndex ]<&rTransform.mDerivedTransform[ rTransform.mIndex ];
        }
    };
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::GameEntityAdded( const GraphicsObjectManager::CreatedGameEntity* cge )
    {
        Ogre::SceneNode* sceneNode = mSceneManager->getRootSceneNode( cge->m_GameEntity->mType )->
            createChildSceneNode( cge->m_GameEntity->mType,
                cge->m_InitialTransform.vPos,
                cge->m_InitialTransform.qRot );

        sceneNode->setScale( cge->m_InitialTransform.vScale );

        cge->m_GameEntity->mSceneNode = sceneNode;

        switch ( cge->m_GameEntity->mMoDefinition->moType )
        {
        case MoTypeItem:
        {
            const ItemDefinition* def = dynamic_cast< const ItemDefinition* >( cge->m_GameEntity->mMoDefinition );
            if ( def )
            {
                Ogre::Item* item = mSceneManager->createItem( def->meshName, def->resourceGroup, cge->m_GameEntity->mType );

                Ogre::StringVector materialNames = def->submeshMaterials;
                size_t minMaterials = std::min( materialNames.size(), item->getNumSubItems() );

                for ( size_t i = 0; i<minMaterials; ++i )
                {
                    item->getSubItem( i )->setDatablockOrMaterialName( materialNames[ i ], def->resourceGroup );
                }

                cge->m_GameEntity->mMovableObject = item;
            }
        }
        break;
        case MoTypeLight:
        {
            const LightDefinition* def = dynamic_cast< const LightDefinition* >( cge->m_GameEntity->mMoDefinition );
            if ( def )
            {
                Ogre::Light* item = mSceneManager->createLight();
                item->setType( static_cast< Ogre::Light::LightTypes >( def->TypeIdx ) );
                cge->m_GameEntity->mMovableObject = item;
            }
        }
        break;
        case MoTypeCamera:
        {
            const CameraDefinition* def = dynamic_cast< const CameraDefinition* >( cge->m_GameEntity->mMoDefinition );
            if ( def )
            {
                Ogre::Camera* item = mSceneManager->createCamera(def->Name);
                cge->m_GameEntity->mMovableObject = item;
            }
        }
        break;
        }

        sceneNode->attachObject( cge->m_GameEntity->mMovableObject );

        //Keep them sorted on how Ogre's internal memory manager assigned them memory,
        //to avoid false cache sharing when we update the nodes concurrently.
        const Ogre::Transform& transform = sceneNode->_getTransform();
        GameEntityVec::iterator itGameEntity = std::lower_bound(
            mGameEntities[ cge->m_GameEntity->mType ].begin(),
            mGameEntities[ cge->m_GameEntity->mType ].end(),
            &transform.mDerivedTransform[ transform.mIndex ],
            GameEntityCmp() );
        mGameEntities[ cge->m_GameEntity->mType ].insert( itGameEntity, cge->m_GameEntity );
    }
    
    //----------------------------------------------------------------------------------
    void GraphicsSystem::GameEntityRemoved( GraphicsObject* toRemove )
    {
        const Ogre::Transform& transform = toRemove->mSceneNode->_getTransform();
        GameEntityVec::iterator itGameEntity = std::lower_bound(
            mGameEntities[ toRemove->mType ].begin(),
            mGameEntities[ toRemove->mType ].end(),
            &transform.mDerivedTransform[ transform.mIndex ],
            GameEntityCmp() );

        assert( itGameEntity!=mGameEntities[ toRemove->mType ].end()&&*itGameEntity==toRemove );
        mGameEntities[ toRemove->mType ].erase( itGameEntity );

        toRemove->mSceneNode->getParentSceneNode()->removeAndDestroyChild( toRemove->mSceneNode );
        toRemove->mSceneNode = nullptr;

        assert( dynamic_cast< Ogre::Item* >( toRemove->mMovableObject ) );

        mSceneManager->destroyItem( static_cast< Ogre::Item* >( toRemove->mMovableObject ) );
        toRemove->mMovableObject = nullptr;
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::UpdateGameEntities( const GameEntityVec& gameEntities, float weight )
    {
        mThreadGameEntityToUpdate = &gameEntities;
        mThreadWeight = weight;

        //Note: You could execute a non-blocking scalable task and do something else, you should
        //wait for the task to finish right before calling renderOneFrame or before trying to
        //execute another UserScalableTask (you would have to be careful, but it could work).
        mSceneManager->executeUserScalableTask( this, true );
    }
    
    //-----------------------------------------------------------------------------------
    void GraphicsSystem::execute( size_t threadId, size_t numThreads )
    {
        size_t currIdx = mCurrentTransformIdx;
        size_t prevIdx = ( mCurrentTransformIdx+NUM_GAME_ENTITY_BUFFERS-1 ) % NUM_GAME_ENTITY_BUFFERS;

        const size_t objsPerThread = ( mThreadGameEntityToUpdate->size() + ( numThreads-1 ) ) / numThreads;
        const size_t toAdvance = std::min( threadId*objsPerThread, mThreadGameEntityToUpdate->size() );

        GameEntityVec::const_iterator itor = mThreadGameEntityToUpdate->begin() + toAdvance;
        GameEntityVec::const_iterator end = mThreadGameEntityToUpdate->begin() + std::min( toAdvance+objsPerThread, mThreadGameEntityToUpdate->size() );
        while ( itor!=end )
        {
            GraphicsObject* gEnt = *itor;
            Ogre::Vector3 interpVec = Ogre::Math::lerp( gEnt->mTransform[ prevIdx ]->vPos, gEnt->mTransform[ currIdx ]->vPos, mThreadWeight );
            gEnt->mSceneNode->setPosition( interpVec );

            interpVec = Ogre::Math::lerp( gEnt->mTransform[ prevIdx ]->vScale, gEnt->mTransform[ currIdx ]->vScale, mThreadWeight );
            gEnt->mSceneNode->setScale( interpVec );

            Ogre::Quaternion interpQ = Ogre::Math::lerp( gEnt->mTransform[ prevIdx ]->qRot, gEnt->mTransform[ currIdx ]->qRot, mThreadWeight );
            gEnt->mSceneNode->setOrientation( interpQ );

            ++itor;
        }
    }
}
