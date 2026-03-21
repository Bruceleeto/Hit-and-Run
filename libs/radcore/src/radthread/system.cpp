//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#include "pch.hpp"

#include <pthread.h>

#include <raddebug.hpp>
#include <radthread.hpp>
#include "system.hpp"
#include "thread.hpp"

static bool g_SystemInitialized = false;

static pthread_mutex_t g_ExclusionObject = PTHREAD_MUTEX_INITIALIZER;

void radThreadInitialize( unsigned int milliseconds )
{
    rAssertMsg( !g_SystemInitialized, "radThread system already initialized\n");

    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &g_ExclusionObject, &attr );
    pthread_mutexattr_destroy( &attr );

    g_SystemInitialized = true;

    radThread::Initialize( milliseconds );
}

void radThreadTerminate( void )
{
    rAssertMsg( g_SystemInitialized, "radThread system has not been initialized\n");

    radThread::Terminate( );

    pthread_mutex_destroy( &g_ExclusionObject );

    g_SystemInitialized = false;
}

void radThreadInternalLock( void )
{
    rAssertMsg( g_SystemInitialized, "radThread system has not been initialized\n");

    pthread_mutex_lock( &g_ExclusionObject );
}

void radThreadInternalUnlock( void )
{
    rAssertMsg( g_SystemInitialized, "radThread system has not been initialized\n");

    pthread_mutex_unlock( &g_ExclusionObject );
}
