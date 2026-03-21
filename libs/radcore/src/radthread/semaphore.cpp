//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#include "pch.hpp"

#include <radthread.hpp>
#include <radmemorymonitor.hpp>
#include "semaphore.hpp"
#include "system.hpp"

void radThreadCreateSemaphore
(
    IRadThreadSemaphore** ppSemaphore,
    unsigned int count,
    radMemoryAllocator allocator
)
{
    *ppSemaphore = new( allocator ) radThreadSemaphore( count );
}

radThreadSemaphore::radThreadSemaphore( unsigned int count )
    :
    m_ReferenceCount( 1 )
{
    radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "radThreadSemaphore" );
    sem_init( &m_Semaphore, 0, count );
}

radThreadSemaphore::~radThreadSemaphore( void )
{
    sem_destroy( &m_Semaphore );
}

void radThreadSemaphore::Wait( void )
{
    sem_wait( &m_Semaphore );
}

void radThreadSemaphore::Signal( void )
{
    sem_post( &m_Semaphore );
}

void radThreadSemaphore::AddRef( void )
{
    radThreadInternalLock( );
	m_ReferenceCount++;
    radThreadInternalUnlock( );
}

void radThreadSemaphore::Release( void )
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
void radThreadSemaphore::Dump( char* pStringBuffer, unsigned int bufferSize )
{
    sprintf( pStringBuffer, "Object: [radThreadSemaphore] At Memory Location:[%p]\n", this );
}
#endif
