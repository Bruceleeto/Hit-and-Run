//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

//=============================================================================
//
// File:        sdldrive.hpp
//
// Subsystem:	Radical Drive System
//
// Description:	This file contains all definitions and classes relevant to
//              the POSIX physical drive.
//
//=============================================================================

#ifndef	SDLDRIVE_HPP
#define SDLDRIVE_HPP

//=============================================================================
// Include Files
//=============================================================================
#include "../common/drive.hpp"
#include "../common/drivethread.hpp"

//=============================================================================
// Defines
//=============================================================================

#define SDL_DEFAULT_SECTOR_SIZE  512

//=============================================================================
// Public Functions
//=============================================================================

void radSdlDriveFactory( radDrive** ppDrive, const char* driveSpec, radMemoryAllocator alloc );

//=============================================================================
// Class Declarations
//=============================================================================

class radSdlDrive : public radDrive
{
public:

    radSdlDrive( const char* pdrivespec, radMemoryAllocator alloc );
    virtual ~radSdlDrive( void );

    void Lock( void );
    void Unlock( void );

    unsigned int GetCapabilities( void );

    const char* GetDriveName( void );

    CompletionStatus Initialize( void );

    CompletionStatus OpenFile( const char*        fileName,
                               radFileOpenFlags   flags,
                               bool               writeAccess,
                               radFileHandle*     pHandle,
                               unsigned int*      pSize );

    CompletionStatus CloseFile( radFileHandle handle, const char* fileName );

    CompletionStatus ReadFile( radFileHandle      handle,
                               const char*        fileName,
                               IRadFile::BufferedReadState buffState,
                               unsigned int       position,
                               void*              pData,
                               unsigned int       bytesToRead,
                               unsigned int*      bytesRead,
                               radMemorySpace     pDataSpace );

    CompletionStatus WriteFile( radFileHandle     handle,
                                const char*       fileName,
                                IRadFile::BufferedReadState buffState,
                                unsigned int      position,
                                const void*       pData,
                                unsigned int      bytesToWrite,
                                unsigned int*     bytesWritten,
                                unsigned int*     size,
                                radMemorySpace    pDataSpace );

    CompletionStatus FindFirst( const char*                 searchSpec,
                                IRadDrive::DirectoryInfo*   pDirectoryInfo,
                                radFileDirHandle*           pHandle,
                                bool                        firstSearch );

    CompletionStatus FindNext( radFileDirHandle* pHandle, IRadDrive::DirectoryInfo* pDirectoryInfo );

    CompletionStatus FindClose( radFileDirHandle* pHandle );

    CompletionStatus CreateDir( const char* pName );

    CompletionStatus DestroyDir( const char* pName );

    CompletionStatus DestroyFile( const char* filename );

private:
    void SetMediaInfo( void );
    void BuildFileSpec( const char* fileName, char* fullName, unsigned int size );

    unsigned int    m_Capabilities;
    unsigned int    m_OpenFiles;
    char            m_DriveName[ radFileDrivenameMax + 1 ];
    char            m_DrivePath[ radFileFilenameMax + 1 ];

    IRadThreadMutex*    m_pMutex;
};

#endif // SDLDRIVE_HPP
