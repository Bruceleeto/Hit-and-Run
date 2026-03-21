//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

//=============================================================================
//
// File:        sdldrive.cpp
//
// Subsystem:   Radical Drive System
//
// Description:	POSIX implementation of the radDrive interface.
//
//=============================================================================

//=============================================================================
// Include Files
//=============================================================================

#include "pch.hpp"
#include <algorithm>
#include <limits.h>
#include <ctype.h>
#include <string>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "sdldrive.hpp"

//=============================================================================
// Helpers
//=============================================================================

static void strupr_inplace( char* s )
{
    for( ; *s; s++ ) *s = (char)toupper( (unsigned char)*s );
}

static void strlwr_inplace( char* s )
{
    for( ; *s; s++ ) *s = (char)tolower( (unsigned char)*s );
}

static unsigned int filesize_from_fp( FILE* fp )
{
    long cur = ftell( fp );
    fseek( fp, 0, SEEK_END );
    long size = ftell( fp );
    fseek( fp, cur, SEEK_SET );
    return (unsigned int)( size >= 0 ? size : 0 );
}

//=============================================================================
// Directory iterator state for FindFirst/FindNext/FindClose
//=============================================================================

struct DirIterState
{
    DIR*        dir;
    std::string basePath;
};

//=============================================================================
// Public Functions
//=============================================================================

void radSdlDriveFactory
(
    radDrive**         ppDrive,
    const char*        pDriveName,
    radMemoryAllocator alloc
)
{
    *ppDrive = new( alloc ) radSdlDrive( pDriveName, alloc );
    rAssert( *ppDrive != NULL );
}

//=============================================================================
// Public Member Functions
//=============================================================================

radSdlDrive::radSdlDrive( const char* pdrivespec, radMemoryAllocator alloc )
    :
    radDrive( ),
    m_OpenFiles( 0 ),
    m_pMutex( NULL )
{
    radThreadCreateMutex( &m_pMutex, alloc );
    rAssert( m_pMutex != NULL );

    m_pDriveThread = new( alloc ) radDriveThread( m_pMutex, alloc );
    rAssert( m_pDriveThread != NULL );

    m_DrivePath[0] = '\0';

    radGetDefaultDrive( m_DriveName );
    if ( strcmp( m_DriveName, pdrivespec ) != 0 )
    {
        strncpy( m_DriveName, pdrivespec, radFileDrivenameMax );
        strncpy( m_DrivePath, pdrivespec, radFileFilenameMax );
        m_DriveName[radFileDrivenameMax] = '\0';
        m_DrivePath[radFileFilenameMax] = '\0';
        strupr_inplace( m_DriveName );
        strlwr_inplace( m_DrivePath );
    }

    if( !m_DrivePath[0] )
    {
        getcwd( m_DrivePath, radFileFilenameMax );
        strncat( m_DrivePath, "/", radFileFilenameMax );
        m_DrivePath[radFileFilenameMax] = '\0';
    }

    m_Capabilities = ( radDriveEnumerable | radDriveWriteable | radDriveDirectory | radDriveFile );
}

radSdlDrive::~radSdlDrive( void )
{
    m_pMutex->Release( );
    m_pDriveThread->Release( );
}

void radSdlDrive::Lock( void )
{
    m_pMutex->Lock( );
}

void radSdlDrive::Unlock( void )
{
    m_pMutex->Unlock( );
}

unsigned int radSdlDrive::GetCapabilities( void )
{
    return m_Capabilities;
}

const char* radSdlDrive::GetDriveName( void )
{
    return m_DriveName;
}

radDrive::CompletionStatus radSdlDrive::Initialize( void )
{
    SetMediaInfo();
    m_LastError = Success;
    return Complete;
}

//=============================================================================
// Function:    radSdlDrive::OpenFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::OpenFile
(
    const char*         fileName,
    radFileOpenFlags    flags,
    bool                writeAccess,
    radFileHandle*      pHandle,
    unsigned int*       pSize
)
{
    char fullName[ radFileFilenameMax + 1 ];
    BuildFileSpec( fileName, fullName, radFileFilenameMax + 1 );

    const char* mode;
    switch( flags )
    {
    case OpenExisting:
        mode = writeAccess ? "rb+" : "rb";
        break;
    case OpenAlways:
        mode = "ab+";
        break;
    case CreateAlways:
        mode = "wb+";
        break;
    default:
        rAssertMsg( false, "radFileSystem: sdldrive: attempting to open file with unknown flag" );
        return Error;
    }

    FILE* fp = fopen( fullName, mode );

    if ( fp )
    {
        m_OpenFiles++;
        *pSize = filesize_from_fp( fp );
        *pHandle = (radFileHandle)fp;
        m_LastError = Success;
        return Complete;
    }
    else
    {
        m_LastError = FileNotFound;
        return Error;
    }
}

//=============================================================================
// Function:    radSdlDrive::CloseFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::CloseFile( radFileHandle handle, const char* fileName )
{
    fclose( (FILE*)handle );
    m_OpenFiles--;
    return Complete;
}

//=============================================================================
// Function:    radSdlDrive::ReadFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::ReadFile
(
    radFileHandle   handle,
    const char*     fileName,
    IRadFile::BufferedReadState buffState,
    unsigned int    position,
    void*           pData,
    unsigned int    bytesToRead,
    unsigned int*   bytesRead,
    radMemorySpace  pDataSpace
)
{
    rAssertMsg( pDataSpace == radMemorySpace_Local,
                "radFileSystem: radSdlDrive: External memory not supported for reads." );

    FILE* fp = (FILE*)handle;

    if ( fseek( fp, (long)position, SEEK_SET ) == 0 )
    {
        size_t result = fread( pData, 1, bytesToRead, fp );
        if ( result > 0 )
        {
            *bytesRead = (unsigned int)result;
            m_LastError = Success;
            return Complete;
        }
    }

    m_LastError = FileNotFound;
    return Error;
}

//=============================================================================
// Function:    radSdlDrive::WriteFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::WriteFile
(
    radFileHandle     handle,
    const char*       fileName,
    IRadFile::BufferedReadState buffState,
    unsigned int      position,
    const void*       pData,
    unsigned int      bytesToWrite,
    unsigned int*     bytesWritten,
    unsigned int*     pSize,
    radMemorySpace    pDataSpace
)
{
    if ( !( m_Capabilities & radDriveWriteable ) )
    {
        rWarningMsg( m_Capabilities & radDriveWriteable, "This drive does not support the WriteFile function." );
        return Error;
    }

    rAssertMsg( pDataSpace == radMemorySpace_Local,
                "radFileSystem: radSdlDrive: External memory not supported for writes." );

    FILE* fp = (FILE*)handle;

    if ( fseek( fp, (long)position, SEEK_SET ) == 0 )
    {
        *bytesWritten = (unsigned int)fwrite( pData, 1, bytesToWrite, fp );
        if ( *bytesWritten == bytesToWrite )
        {
            *pSize = filesize_from_fp( fp );
            m_LastError = Success;
            return Complete;
        }
    }

    m_LastError = FileNotFound;
    return Error;
}

//=============================================================================
// Function:    radSdlDrive::FindFirst
//=============================================================================

radDrive::CompletionStatus radSdlDrive::FindFirst
(
    const char*                 searchSpec,
    IRadDrive::DirectoryInfo*   pDirectoryInfo,
    radFileDirHandle*           pHandle,
    bool                        firstSearch
)
{
    // Split searchSpec into directory path and filename pattern
    std::string spec( searchSpec );
    std::replace( spec.begin(), spec.end(), '\\', '/' );

    std::string dirPath = m_DrivePath;
    size_t lastSlash = spec.rfind( '/' );
    if ( lastSlash != std::string::npos )
    {
        dirPath += spec.substr( 0, lastSlash );
    }

    DIR* dir = opendir( dirPath.c_str() );
    if ( !dir )
    {
        m_LastError = FileNotFound;
        return Error;
    }

    DirIterState* state = new DirIterState;
    state->dir = dir;
    state->basePath = dirPath;
    *pHandle = (radFileDirHandle)state;

    // Read the first entry
    return FindNext( pHandle, pDirectoryInfo );
}

//=============================================================================
// Function:    radSdlDrive::FindNext
//=============================================================================

radDrive::CompletionStatus radSdlDrive::FindNext( radFileDirHandle* pHandle, IRadDrive::DirectoryInfo* pDirectoryInfo )
{
    if ( *pHandle == NULL )
    {
        m_LastError = FileNotFound;
        return Error;
    }

    DirIterState* state = (DirIterState*)*pHandle;
    struct dirent* entry;

    while ( ( entry = readdir( state->dir ) ) != NULL )
    {
        // Skip . and ..
        if ( strcmp( entry->d_name, "." ) == 0 || strcmp( entry->d_name, ".." ) == 0 )
            continue;

        strncpy( pDirectoryInfo->m_Name, entry->d_name, radFileFilenameMax );
        pDirectoryInfo->m_Name[ radFileFilenameMax ] = '\0';

        // Determine type via stat
        std::string fullPath = state->basePath + "/" + entry->d_name;
        struct stat st;
        if ( stat( fullPath.c_str(), &st ) == 0 && S_ISDIR( st.st_mode ) )
        {
            pDirectoryInfo->m_Type = IRadDrive::DirectoryInfo::IsDirectory;
        }
        else
        {
            pDirectoryInfo->m_Type = IRadDrive::DirectoryInfo::IsFile;
        }

        m_LastError = Success;
        return Complete;
    }

    // No more entries
    pDirectoryInfo->m_Name[0] = '\0';
    pDirectoryInfo->m_Type = IRadDrive::DirectoryInfo::IsDone;
    m_LastError = Success;
    return Complete;
}

//=============================================================================
// Function:    radSdlDrive::FindClose
//=============================================================================

radDrive::CompletionStatus radSdlDrive::FindClose( radFileDirHandle* pHandle )
{
    if ( *pHandle != NULL )
    {
        DirIterState* state = (DirIterState*)*pHandle;
        closedir( state->dir );
        delete state;
        *pHandle = NULL;
    }

    return Complete;
}

//=============================================================================
// Function:    radSdlDrive::CreateDir
//=============================================================================

radDrive::CompletionStatus radSdlDrive::CreateDir( const char* pName )
{
    rWarningMsg( m_Capabilities & radDriveDirectory,
        "This drive does not support the CreateDir function." );

    char fullSpec[ radFileFilenameMax + 1 ];
    BuildFileSpec( pName, fullSpec, radFileFilenameMax + 1 );

    if ( mkdir( fullSpec, 0755 ) == 0 )
    {
        m_LastError = Success;
        return Complete;
    }
    else
    {
        m_LastError = FileNotFound;
        return Error;
    }
}

//=============================================================================
// Function:    radSdlDrive::DestroyDir
//=============================================================================

radDrive::CompletionStatus radSdlDrive::DestroyDir( const char* pName )
{
    rWarningMsg( m_Capabilities & radDriveDirectory,
        "This drive does not support the DestroyDir function." );

    char fullSpec[ radFileFilenameMax + 1 ];
    BuildFileSpec( pName, fullSpec, radFileFilenameMax + 1 );

    if ( rmdir( fullSpec ) == 0 )
    {
        m_LastError = Success;
        return Complete;
    }
    else
    {
        m_LastError = FileNotFound;
        return Error;
    }
}

//=============================================================================
// Function:    radSdlDrive::DestroyFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::DestroyFile( const char* filename )
{
    rWarningMsg( m_Capabilities & radDriveWriteable, "This drive does not support the DestroyFile function." );

    char fullSpec[ radFileFilenameMax + 1 ];
    BuildFileSpec( filename, fullSpec, radFileFilenameMax + 1 );

    if ( remove( fullSpec ) == 0 )
    {
        m_LastError = Success;
        return Complete;
    }
    else
    {
        m_LastError = FileNotFound;
        return Error;
    }
}

//=============================================================================
// Private Member Functions
//=============================================================================

void radSdlDrive::SetMediaInfo( void )
{
    const char* realDriveName = m_DriveName;

    strcpy( m_MediaInfo.m_VolumeName, realDriveName );

    m_MediaInfo.m_SectorSize = SDL_DEFAULT_SECTOR_SIZE;
    m_MediaInfo.m_MediaState = IRadDrive::MediaInfo::MediaPresent;
    m_MediaInfo.m_FreeSpace = UINT_MAX;
    m_MediaInfo.m_FreeFiles = m_MediaInfo.m_FreeSpace / m_MediaInfo.m_SectorSize;
    m_LastError = Success;
}

void radSdlDrive::BuildFileSpec( const char* fileName, char* fullName, unsigned int size )
{
    std::string path( m_DrivePath );
    path += fileName;
    std::replace( path.begin(), path.end(), '\\', '/' );

    strncpy( fullName, path.c_str(), size - 1 );
    fullName[ size - 1 ] = '\0';
}
