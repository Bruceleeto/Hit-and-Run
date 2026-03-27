//=============================================================================
// Copyright (C) 2002 Radical Entertainment Ltd.  All rights reserved.
//
// File:        dcmain.cpp
//
// Description: Dreamcast main entry point.
//
//=============================================================================

//========================================
// System Includes
//========================================
#include <string.h>
#include <stdio.h>
#include <raddebug.hpp>
#include <radobject.hpp>
#include <kos.h>

//========================================
// Project Includes
//========================================
#include <main/game.h>
#include <main/dcplatform.h>
#include <main/singletons.h>
#include <main/commandlineoptions.h>
#include <memory/memoryutilities.h>
#include <memory/srrmemory.h>
#include <p3d/entity.hpp>

//========================================
// Forward Declarations
//========================================
static void ProcessCommandLineArguments( int argc, char *argv[] );
static void ProcessCommandLineArgumentsFromFile();

//=============================================================================
// Function:    main
//=============================================================================
KOS_INIT_FLAGS(INIT_DEFAULT | INIT_CONTROLLER);

extern "C" int main( int argc, char *argv[] )
{
    CommandLineOptions::InitDefaults();
    ProcessCommandLineArguments( argc, argv );
    ProcessCommandLineArgumentsFromFile();

    if( !DCPlatform::InitializeWindow() )
    {
        return 0;
    }
    DCPlatform::InitializeFoundation();

    srand( Game::GetRandomSeed() );

#ifndef RAD_RELEASE
    tName::SetAllocator( GMA_DEBUG );
#endif

    HeapMgr()->PushHeap( GMA_PERSISTENT );

    CreateSingletons();

    DCPlatform* pPlatform = DCPlatform::CreateInstance();
    rAssert( pPlatform != NULL );

    Game* pGame = Game::CreateInstance( pPlatform );
    rAssert( pGame != NULL );

    pGame->Initialize();

    HeapMgr()->PopHeap( GMA_PERSISTENT );

    pGame->Run();

    pGame->Terminate();

    DestroySingletons();

    Game::DestroyInstance();

    pPlatform->ShutdownPlatform();

    DCPlatform::DestroyInstance();

    DCPlatform::ShutdownMemory();

#ifndef RAD_RELEASE
    tName::SetAllocator( RADMEMORY_ALLOC_DEFAULT );
#endif

    arch_exit();

    return 0;
}

void ProcessCommandLineArguments( int argc, char* argv[] )
{
    rDebugPrintf( "*************************************************************************\n" );
    rDebugPrintf( "Command Line Args:\n" );

    for( int i = 1; i < argc; i++ )
    {
        rDebugPrintf( "arg%d: %s\n", i, argv[i] );
        CommandLineOptions::HandleOption( argv[i] );
    }

    rDebugPrintf( "*************************************************************************\n" );
}

void ProcessCommandLineArgumentsFromFile()
{
#ifndef FINAL
    FILE* pfile = fopen( "command.txt", "r" );

    if( pfile != NULL )
    {
        int ret = fseek( pfile, 0, SEEK_END );
        rAssert( ret == 0 );

        int len = ftell( pfile );
        rAssertMsg( len < 256, "Command line file too large to process." );

        rewind( pfile );

        if( len > 0 && len < 256 )
        {
            char commandlinestring[256] = {0};
            fgets( commandlinestring, 256, pfile );

            char* argument = strtok( commandlinestring, " " );
            while( argument != NULL )
            {
                CommandLineOptions::HandleOption( argument );
                argument = strtok( NULL, " " );
            }
        }

        fclose( pfile );
    }
#endif
}
