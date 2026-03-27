//===========================================================================
// Copyright (C) 2002 Radical Entertainment Ltd.  All rights reserved.
//
// Component:   DCPlatform
//
// Description: Abstracts the differences for setting up and shutting down
//              the different platforms.
//
//===========================================================================

//========================================
// System Includes
//========================================
#include <kos.h>
// Standard Lib
#include <stdlib.h>
#include <string.h>
// Pure 3D
#include <p3d/anim/compositedrawable.hpp>
#include <p3d/anim/expression.hpp>
#include <p3d/anim/multicontroller.hpp>
#include <p3d/anim/polyskin.hpp>
#include <p3d/anim/sequencer.hpp>
#include <p3d/anim/skeleton.hpp>
#include <p3d/camera.hpp>
#include <p3d/gameattr.hpp>
#include <p3d/image.hpp>
#include <p3d/imagefont.hpp>
#include <p3d/light.hpp>
#include <p3d/locator.hpp>
#include <p3d/platform.hpp>
#include <p3d/scenegraph/scenegraph.hpp>
#include <p3d/sprite.hpp>
#include <p3d/utility.hpp>
#include <p3d/texture.hpp>
#include <p3d/file.hpp>
#include <p3d/shader.hpp>
#include <p3d/matrixstack.hpp>
#include <p3d/memory.hpp>
#include <p3d/bmp.hpp>
#include <p3d/png.hpp>
#include <p3d/targa.hpp>
#include <p3d/font.hpp>
#include <p3d/texturefont.hpp>
#include <p3d/unicode.hpp>
// Pure 3D: Loader-specific
#include <render/Loaders/GeometryWrappedLoader.h>
#include <render/Loaders/StaticEntityLoader.h>
#include <render/Loaders/StaticPhysLoader.h>
#include <render/Loaders/TreeDSGLoader.h>
#include <render/Loaders/FenceLoader.h>
#include <render/Loaders/IntersectLoader.h>
#include <render/Loaders/AnimCollLoader.h>
#include <render/Loaders/AnimDSGLoader.h>
#include <render/Loaders/DynaPhysLoader.h>
#include <render/Loaders/InstStatPhysLoader.h>
#include <render/Loaders/InstStatEntityLoader.h>
#include <render/Loaders/WorldSphereLoader.h>
#include <loading/roaddatasegmentloader.h>
#include <render/Loaders/BillboardWrappedLoader.h>
#include <render/Loaders/instparticlesystemloader.h>
#include <render/Loaders/breakableobjectloader.h>
#include <render/Loaders/AnimDynaPhysLoader.h>
#include <render/Loaders/LensFlareLoader.h>
#include <p3d/shadow.hpp>
#include <p3d/anim/animatedobject.hpp>
#include <p3d/effects/particleloader.hpp>
#include <p3d/effects/opticloader.hpp>
#include <p3d/anim/vertexanimkey.hpp>
#include <stateprop/statepropdata.hpp>

// Foundation Tech
#include <raddebug.hpp>
#include <radthread.hpp>
#include <radplatform.hpp>
#include <radtime.hpp>
#include <radmemorymonitor.hpp>
#include <raddebugcommunication.hpp>
#include <raddebugwatch.hpp>
#include <radfile.hpp>
//This is so we can get the name of the file that's failing.
#include <../src/radfile/common/requests.hpp>

// sim - for InstallSimLoaders
#include <simcommon/simutility.hpp>

//========================================
// Project Includes
//========================================
#include <input/inputmanager.h>
#include <main/dcplatform.h>
#include <main/commandlineoptions.h>
#include <main/game.h>
#include <render/RenderManager/RenderManager.h>
#include <render/RenderFlow/renderflow.h>
#include <render/Loaders/AllWrappers.h>
#include <memory/srrmemory.h>

#include <loading/locatorloader.h>
#include <loading/cameradataloader.h>
#include <loading/roadloader.h>
#include <loading/pathloader.h>
#include <loading/intersectionloader.h>
#include <loading/roaddatasegmentloader.h>
#include <atc/atcloader.h>
#include <data/gamedatamanager.h>
#include <debug/debuginfo.h>
#include <constants/srrchunks.h>
#include <gameflow/gameflow.h>
#include <sound/soundmanager.h>
#include <presentation/presentation.h>
#include <presentation/gui/guitextbible.h>
#include <cheats/cheatinputsystem.h>
#include <mission/gameplaymanager.h>

#include <radload/radload.hpp>

#include <main/errorsWIN32.h>

#include <strings.h>
#define _stricmp strcasecmp
#define WIN32_SECTION "WIN32_SECTION"
#define TIMER_LEAVE 1

//******************************************************************************
//
// Global Data, Local Data, Local Classes
//
//******************************************************************************

// Static pointer to instance of singleton.
DCPlatform* DCPlatform::spInstance = NULL;

// Other static members.
bool DCPlatform::mShowCursor = true;

//The Adlib font.  <sigh>
unsigned char gFont[] =
#include <font/defaultfont.h>

//
// Define the starting resolution.
//
static const DCPlatform::Resolution StartingResolution = DCPlatform::Res_640x480;
static const int StartingBPP = 16;

// This specifies the PDDI DLL to use.
#ifdef RAD_DEBUG
static const char pddiLibraryName[] = "pddi%sd.dll";
#endif
#ifdef RAD_TUNE
static const char pddiLibraryName[] = "pddi%st.dll";
#endif
#ifdef RAD_RELEASE
static const char pddiLibraryName[] = "pddi%sr.dll";
#endif

// Name of the application.
static const char ApplicationName[] = "The Simpsons: Hit & Run";

void LoadMemP3DFile( unsigned char* buffer, unsigned int size, tEntityStore* store )
{
    tFileMem* file = new tFileMem(buffer,size);
    file->AddRef();
    file->SetFilename("memfile.p3d");
    p3d::loadManager->GetP3DHandler()->Load( file, p3d::inventory );
    file->Release();
}

//******************************************************************************
//
// Public Member Functions
//
//******************************************************************************

DCPlatform* DCPlatform::CreateInstance()
{
MEMTRACK_PUSH_GROUP( "DCPlatform" );
    rAssert( spInstance == NULL );

    spInstance = new(GMA_PERSISTENT) DCPlatform();
    rAssert( spInstance );
MEMTRACK_POP_GROUP( "DCPlatform" );

    return spInstance;
}

DCPlatform* DCPlatform::GetInstance()
{
    rAssert( spInstance != NULL );
    return spInstance;
}

void DCPlatform::DestroyInstance()
{
    rAssert( spInstance != NULL );
    operator delete( spInstance, GMA_PERSISTENT );
    spInstance = NULL;
}

bool DCPlatform::InitializeWindow()
{
    // Dreamcast: no window; display is fullscreen via KOS.
    (void)StartingResolution;
    return true;
}

void DCPlatform::InitializeFoundation()
{
    DCPlatform::InitializeMemory();

    //
    // Register an out-of-memory display handler in case something goes bad
    // while allocating the heaps
    //
    ::radMemorySetOutOfMemoryCallback( PrintOutOfMemoryMessage, NULL );

    //
    // Initialize memory monitor by JamesCo. TM.
    //
    if( CommandLineOptions::Get( CLO_MEMORY_MONITOR ) )
    {
        const int KB = 1024;
        ::radMemoryMonitorInitialize( 64 * KB, GMA_DEBUG );
    }

    // Setup the memory heaps
    HeapMgr()->PrepareHeapsStartup ();

    // Seed the heap stack
    HeapMgr()->PushHeap (GMA_PERSISTENT);

    //
    // Initialize the platform system
    //
    ::radPlatformInitialize( );

    //
    // Initialize the timer system
    //
    ::radTimeInitialize();

    //
    // Initialize the debug communication system.
    //
    ::radDbgComTargetInitialize( WinSocket,
        radDbgComDefaultPort, // Default
        NULL,                 // Default
        GMA_DEBUG );

    //
    // Initialize the Watcher.
    //
    ::radDbgWatchInitialize( "SRR2",
                             32 * 16384, // 2 * Default
                             GMA_DEBUG );

    //
    // Initialize the file system.
    //
    ::radFileInitialize( 50, // Default
        32, // Default
        GMA_PERSISTENT );

    ::radLoadInitialize();

    ::radDriveMount( NULL, GMA_PERSISTENT);

    HeapMgr()->PopHeap (GMA_PERSISTENT);
}

void DCPlatform::InitializeMemory()
{
    //
    // Only do this once!
    //
    if( gMemorySystemInitialized == true )
    {
        return;
    }

    gMemorySystemInitialized = true;

    //
    // Initialize the thread system.
    //
    ::radThreadInitialize();

    //
    // Initialize the memory system.
    //
    ::radMemoryInitialize();
}

void DCPlatform::ShutdownMemory()
{
    if( gMemorySystemInitialized )
    {
        gMemorySystemInitialized = false;
        ::radThreadTerminate();
    }
}

void DCPlatform::InitializePlatform()
{
    HeapMgr()->PushHeap (GMA_PERSISTENT);

    //
    // Rendering is good.
    //
    InitializePure3D();

    //
    // Add anything here that needs to be before the drive is opened.
    //
    DisplaySplashScreen( Error ); // blank screen

    //
    // Opening the drive is SLOW...
    //
    InitializeFoundationDrive();

    //
    // Initialize the controller.
    //
    GetInputManager()->Init();

    HeapMgr()->PopHeap (GMA_PERSISTENT);
}

void DCPlatform::ShutdownPlatform()
{
    ShutdownPure3D();
    ShutdownFoundation();
}

void DCPlatform::LaunchDashboard()
{
    GetLoadingManager()->CancelPendingRequests();
    GetPresentationManager()->StopAll();
    p3d::loadManager->CancelAll();
}

void DCPlatform::ResetMachine()
{
    arch_exit();
}

void DCPlatform::DisplaySplashScreen( SplashScreen screenID,
                                       const char* overlayText,
                                       float fontScale,
                                       float textPosX,
                                       float textPosY,
                                       tColour textColour,
                                       int fadeFrames )
{
    HeapMgr()->PushHeap( GMA_TEMP );

    p3d::inventory->PushSection();
    p3d::inventory->AddSection( WIN32_SECTION );
    p3d::inventory->SelectSection( WIN32_SECTION );

    P3D_UNICODE unicodeText[256];

    // Save the current Projection mode so I can restore it later
    pddiProjectionMode pm = p3d::pddi->GetProjectionMode();
    p3d::pddi->SetProjectionMode(PDDI_PROJECTION_DEVICE);

    pddiCullMode cm = p3d::pddi->GetCullMode();
    p3d::pddi->SetCullMode(PDDI_CULL_NONE);

    //CREATE THE FONT
    tTextureFont* thisFont = NULL;

    LoadMemP3DFile( gFont, DEFAULTFONT_SIZE, p3d::inventory );

    thisFont = p3d::find<tTextureFont>("adlibn_20");
    rAssert( thisFont );

    thisFont->AddRef();
    tShader* fontShader = thisFont->GetShader();

    p3d::AsciiToUnicode( overlayText, unicodeText, 256 );

    // Make the missing letter into something I can see
    thisFont->SetMissingLetter(p3d::ConvertCharToUnicode('j'));

    int a = 0;

    do
    {
        p3d::pddi->SetColourWrite(true, true, true, true);
        p3d::pddi->SetClearColour( pddiColour(0,0,0) );
        p3d::pddi->BeginFrame();
        p3d::pddi->Clear(PDDI_BUFFER_COLOUR);

        int bright = 255;
        if (a < fadeFrames) bright = (a * 255) / fadeFrames;
        if ( bright > 255 ) bright = 255;

        //Display font
        if (overlayText != NULL)
        {
            tColour colour = textColour;
            colour.SetAlpha( bright );

            thisFont->SetColour( colour );

            p3d::pddi->SetProjectionMode(PDDI_PROJECTION_ORTHOGRAPHIC);
            p3d::stack->Push();
            p3d::stack->LoadIdentity();

            p3d::stack->Translate( textPosX, textPosY, 1.0f);
            float scaleSize = 1.0f / 480.0f;
            p3d::stack->Scale(scaleSize * fontScale, scaleSize* fontScale , 1.0f);

            if ( textPosX != 0.0f || textPosY != 0.0f )
            {
                thisFont->DisplayText( unicodeText );
            }
            else
            {
                thisFont->DisplayText( unicodeText, 3 );
            }

            p3d::stack->Pop();
        }

        p3d::pddi->EndFrame();
        p3d::context->SwapBuffers();

        ++a;

    } while (a <= fadeFrames + 1);

    p3d::pddi->SetCullMode(cm);
    p3d::pddi->SetProjectionMode(pm);

    thisFont->Release();

    p3d::inventory->RemoveSectionElements(WIN32_SECTION);
    p3d::inventory->DeleteSection(WIN32_SECTION);
    p3d::inventory->PopSection();

    HeapMgr()->PopHeap( GMA_TEMP );
}

void DCPlatform::DisplaySplashScreen( const char* textureName,
                                       const char* overlayText,
                                       float fontScale,
                                       float textPosX,
                                       float textPosY,
                                       tColour textColour,
                                       int fadeFrames )
{
}

void DCPlatform::OnControllerError(const char *msg)
{
    DisplaySplashScreen( Error, msg, 0.7f, 0.0f, 0.0f, tColour(255, 255, 255), 0 );
    mErrorState = CTL_ERROR;
    mPauseForError = true;
}

bool DCPlatform::OnDriveError( radFileError error, const char* pDriveName, void* pUserData )
{
    GameDataManager* gm = GetGameDataManager();
    if( gm->IsUsingDrive() )
    {
        return gm->OnDriveError( error, pDriveName, pUserData );
    }

    switch ( error )
    {
    case Success:
        {
            if ( mErrorState != NONE )
            {
                DisplaySplashScreen( FadeToBlack );
                mErrorState = NONE;
                mPauseForError = false;
            }
            return true;
            break;
        }
    case FileNotFound:
        {
            rAssert(pUserData != NULL);
            radFileRequest* request = static_cast<radFileRequest*>(pUserData);
            const char* fileName = request->GetFilename();
            char errorString[256];
            snprintf(errorString, sizeof(errorString), "%s:\nDrive: %s\nFile: %s", ERROR_STRINGS[error], pDriveName, fileName);
            fprintf(stderr, "error: %s\n", errorString);
            DisplaySplashScreen(Error, errorString, 1.0f, 0.0f, 0.0f, tColour(255, 255, 255), 0);
            mErrorState = P_ERROR;
            mPauseForError = true;
            return true;
        }
    case NoMedia:
    case MediaNotFormatted:
    case MediaCorrupt:
    case NoFreeSpace:
    case HardwareFailure:
        {
            fprintf(stderr, "error: %s\n", ERROR_STRINGS[error]);
            DisplaySplashScreen( Error, ERROR_STRINGS[error], 1.0f, 0.0f, 0.0f, tColour(255, 255, 255), 0 );
            mErrorState = P_ERROR;
            mPauseForError = true;
            return true;
        }
    default:
        {
            rAssert( false );
        }
    }

    return false;
}

bool DCPlatform::SetResolution( Resolution res, int bpp, bool fullscreen )
{
    if( !mpContext || !IsResolutionSupported( res, bpp ) )
    {
        return false;
    }

    mResolution = res;
    mbpp = bpp;
    mFullscreen = fullscreen;

    InitializeContext();
    ResizeWindow();

    return true;
}

DCPlatform::Resolution DCPlatform::GetResolution() const
{
    return mResolution;
}

int DCPlatform::GetBPP() const
{
    return mbpp;
}

bool DCPlatform::IsFullscreen() const
{
    return mFullscreen;
}

//******************************************************************************
//
// Private Member Functions
//
//******************************************************************************

DCPlatform::DCPlatform() :
    mpPlatform( NULL ),
    mpContext( NULL ),
    mResolution( StartingResolution ),
    mbpp( StartingBPP ),
    mRenderer( "gl" )
{
    mFullscreen = false;
}

DCPlatform::~DCPlatform()
{
    HeapManager::DestroyInstance();
}

void DCPlatform::InitializeFoundationDrive()
{
    char defaultDrive[ radFileDrivenameMax + 1 ];

    ::radGetDefaultDrive( defaultDrive );

    ::radDriveOpenSync( &mpIRadDrive,
                        defaultDrive,
                        NormalPriority, // Default
                        GMA_PERSISTENT );

    rAssert( mpIRadDrive != NULL );

    mpIRadDrive->RegisterErrorHandler( this, NULL );
}

void DCPlatform::ShutdownFoundation()
{
    mpIRadDrive->Release();
    mpIRadDrive = NULL;

    ::radDriveUnmount( NULL );
    ::radLoadTerminate();
    ::radFileTerminate();
    ::radDbgWatchTerminate();
    if( CommandLineOptions::Get( CLO_MEMORY_MONITOR ) )
    {
        ::radMemoryMonitorTerminate();
    }
    ::radDbgComTargetTerminate();
    ::radTimeTerminate();
    ::radPlatformTerminate();
}

void DCPlatform::InitializePure3D()
{
MEMTRACK_PUSH_GROUP( "DCPlatform" );

    mpPlatform = tPlatform::Create( NULL );
    rAssert( mpPlatform != NULL );

    InitializeContext();

    P3DASSERT(p3d::context);
    tP3DFileHandler* p3d = new(GMA_PERSISTENT) tP3DFileHandler;
    p3d::context->GetLoadManager()->AddHandler(p3d, "p3d");
    p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tPNGHandler, "png");

    if( CommandLineOptions::Get( CLO_FE_UNJOINED ) )
    {
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tBMPHandler, "bmp");
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tTargaHandler, "tga");
    }
    else
    {
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tBMPHandler, "p3d");
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tPNGHandler, "p3d");
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tTargaHandler, "p3d");
    }

    GeometryWrappedLoader* pGWL =
        (GeometryWrappedLoader*)GetAllWrappers()->mpLoader( AllWrappers::msGeometry );
    pGWL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pGWL );

    StaticEntityLoader* pSEL =
        (StaticEntityLoader*)GetAllWrappers()->mpLoader( AllWrappers::msStaticEntity );
    pSEL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pSEL );

    StaticPhysLoader* pSPL =
        (StaticPhysLoader*)GetAllWrappers()->mpLoader( AllWrappers::msStaticPhys );
    pSPL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pSPL );

    TreeDSGLoader* pTDL =
        (TreeDSGLoader*)GetAllWrappers()->mpLoader( AllWrappers::msTreeDSG );
    pTDL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pTDL );

    FenceLoader* pFL =
        (FenceLoader*)GetAllWrappers()->mpLoader( AllWrappers::msFenceEntity );
    pFL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pFL );

    IntersectLoader* pIL =
        (IntersectLoader*)GetAllWrappers()->mpLoader( AllWrappers::msIntersectDSG );
    pIL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pIL );

    AnimCollLoader* pACL =
        (AnimCollLoader*)GetAllWrappers()->mpLoader( AllWrappers::msAnimCollEntity );
    pACL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pACL );

    AnimDSGLoader* pAnimDSGLoader =
        (AnimDSGLoader*)GetAllWrappers()->mpLoader( AllWrappers::msAnimEntity );
    pAnimDSGLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pAnimDSGLoader );

    DynaPhysLoader* pDPL =
        (DynaPhysLoader*)GetAllWrappers()->mpLoader( AllWrappers::msDynaPhys );
    pDPL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pDPL );

    InstStatPhysLoader* pISPL =
        (InstStatPhysLoader*)GetAllWrappers()->mpLoader( AllWrappers::msInstStatPhys );
    pISPL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pISPL );

    InstStatEntityLoader* pISEL =
        (InstStatEntityLoader*)GetAllWrappers()->mpLoader( AllWrappers::msInstStatEntity );
    pISEL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pISEL );

    LocatorLoader* pLL =
        (LocatorLoader*)GetAllWrappers()->mpLoader( AllWrappers::msLocator);
    pLL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pLL );

    RoadLoader* pRL =
        (RoadLoader*)GetAllWrappers()->mpLoader( AllWrappers::msRoadSegment);
    pRL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pRL );

    PathLoader* pPL =
        (PathLoader*)GetAllWrappers()->mpLoader( AllWrappers::msPathSegment);
    pPL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pPL );

    WorldSphereLoader* pWSL =
        (WorldSphereLoader*)GetAllWrappers()->mpLoader( AllWrappers::msWorldSphere);
    pWSL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pWSL );

    LensFlareLoader* pLSL =
        (LensFlareLoader*)GetAllWrappers()->mpLoader( AllWrappers::msLensFlare);
    pLSL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pLSL );

    BillboardWrappedLoader* pBWL =
        (BillboardWrappedLoader*)GetAllWrappers()->mpLoader( AllWrappers::msBillboard);
    pBWL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pBWL );

    InstParticleSystemLoader* pInstParticleSystemLoader =
        (InstParticleSystemLoader*) GetAllWrappers()->mpLoader( AllWrappers::msInstParticleSystem);
    pInstParticleSystemLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pInstParticleSystemLoader );

    BreakableObjectLoader* pBreakableObjectLoader =
        (BreakableObjectLoader*) GetAllWrappers()->mpLoader( AllWrappers::msBreakableObject);
    pBreakableObjectLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pBreakableObjectLoader );

    AnimDynaPhysLoader* pAnimDynaPhysLoader =
        (AnimDynaPhysLoader*) GetAllWrappers()->mpLoader( AllWrappers::msAnimDynaPhys);
    pAnimDynaPhysLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pAnimDynaPhysLoader );

    AnimDynaPhysWrapperLoader* pAnimWrapperLoader =
        (AnimDynaPhysWrapperLoader*) GetAllWrappers()->mpLoader( AllWrappers::msAnimDynaPhysWrapper);
    pAnimWrapperLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pAnimWrapperLoader );

    p3d->AddHandler(new(GMA_PERSISTENT) tTextureLoader);
    p3d->AddHandler( new(GMA_PERSISTENT) tSetLoader );
    p3d->AddHandler(new(GMA_PERSISTENT) tShaderLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tCameraLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tGameAttrLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tLightLoader);

    p3d->AddHandler(new(GMA_PERSISTENT) tLocatorLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tLightGroupLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tImageLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tTextureFontLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tImageFontLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tSpriteLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tSkeletonLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tPolySkinLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tCompositeDrawableLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tVertexAnimKeyLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tAnimationLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tFrameControllerLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tMultiControllerLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tAnimatedObjectFactoryLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tAnimatedObjectLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tParticleSystemFactoryLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tParticleSystemLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tLensFlareGroupLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) sg::Loader);

    p3d->AddHandler(new(GMA_PERSISTENT) tExpressionGroupLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tExpressionMixerLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tExpressionLoader);

    //ATCloader
    p3d->AddHandler(new(GMA_PERSISTENT) ATCLoader);

    tSEQFileHandler* sequencerFileHandler = new(GMA_PERSISTENT) tSEQFileHandler;
    p3d::loadManager->AddHandler(sequencerFileHandler, "seq");

    // sim lib
    sim::InstallSimLoaders();

    p3d->AddHandler(new(GMA_PERSISTENT) CameraDataLoader, SRR2::ChunkID::FOLLOWCAM);
    p3d->AddHandler(new(GMA_PERSISTENT) CameraDataLoader, SRR2::ChunkID::WALKERCAM);
    p3d->AddHandler(new(GMA_PERSISTENT) IntersectionLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) RoadDataSegmentLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) CStatePropDataLoader);
MEMTRACK_POP_GROUP( "DCPlatform" );
}

void DCPlatform::ShutdownPure3D()
{
    p3d::inventory->RemoveAllElements();
    p3d::inventory->DeleteAllSections();

    if( mpContext != NULL )
    {
        mpPlatform->DestroyContext( mpContext );
        mpContext = NULL;
    }

    if( mpPlatform != NULL )
    {
        tPlatform::Destroy( mpPlatform );
        mpPlatform = NULL;
    }
}

void DCPlatform::InitializeContext()
{
    tContextInitData init;

    init.displayMode = PDDI_DISPLAY_FULLSCREEN;

    init.bufferMask = PDDI_BUFFER_COLOUR | PDDI_BUFFER_DEPTH;
    init.enableSnapshot = false;

    TranslateResolution( mResolution, init.xsize, init.ysize );

    init.bpp = mbpp;
    init.lockToVsync = false;

    if( mpContext == NULL )
    {
        snprintf(init.PDDIlib, 128, pddiLibraryName, mRenderer);

        mpContext = mpPlatform->CreateContext( &init );
        rAssert( mpContext != NULL );

        mpPlatform->SetActiveContext( mpContext );
        p3d::pddi->EnableZBuffer( true );
    }
    else
    {
        mpContext->GetDisplay()->InitDisplay( &init );
    }
}

void DCPlatform::TranslateResolution( Resolution res, int&x, int&y )
{
    x = 640;
    y = 480;
}

bool DCPlatform::IsResolutionSupported( Resolution res, int bpp ) const
{
    return ( res == Res_640x480 && bpp == 16 );
}

void DCPlatform::ResizeWindow()
{
}

void DCPlatform::ShowTheCursor( bool show )
{
}
