//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#include "pch.hpp"

#include <radthread.hpp>
#include <radmemorymonitor.hpp>
#include "mutex.hpp"
#include "system.hpp"

void radThreadCreateMutex
(
    IRadThreadMutex**   ppMutex,
    radMemoryAllocator  allocator
)
{
    *ppMutex = new( allocator ) radThreadMutex( );
}

radThreadMutex::radThreadMutex( void )
    :
    m_ReferenceCount( 1 )
{
    radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "radThreadMutex" );
    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &m_Mutex, &attr );
    pthread_mutexattr_destroy( &attr );
}

radThreadMutex::~radThreadMutex( void )
{
    pthread_mutex_destroy( &m_Mutex );
}

void radThreadMutex::Lock( void )
{
    pthread_mutex_lock( &m_Mutex );
}

void radThreadMutex::Unlock( void )
{
    pthread_mutex_unlock( &m_Mutex );
}

void radThreadMutex::AddRef( void )
{
    radThreadInternalLock( );
	m_ReferenceCount++;
    radThreadInternalUnlock( );
}

void radThreadMutex::Release( void )
{
    radThreadInternalLock( );

	m_ReferenceCount--;

	if ( m_ReferenceCount == 0 )
	{
        radThreadInternalUnlock( );
		delete this;
	}
    else
    {
        radThreadInternalUnlock( );
    }
}

#ifdef RAD_DEBUG
void radThreadMutex::Dump( char* pStringBuffer, unsigned int bufferSize )
{
    sprintf( pStringBuffer, "Object: [radThreadMutex] At Memory Location:[%p]\n", this );
}
#endif
