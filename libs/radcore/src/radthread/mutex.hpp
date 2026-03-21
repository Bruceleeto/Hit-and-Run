//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#ifndef	MUTEX_HPP
#define MUTEX_HPP

#include <radobject.hpp>
#include <radmemory.hpp>
#include <radthread.hpp>
#include <pthread.h>

class radThreadMutex : public IRadThreadMutex,
                       public radObject
{
    public:

    radThreadMutex( void );
    ~radThreadMutex( void );

    virtual void Lock( void );
    virtual void Unlock( void );

    virtual void AddRef( void );
    virtual void Release( void );

    #ifdef RAD_DEBUG
    virtual void Dump( char* pStringBuffer, unsigned int bufferSize );
    #endif

    private:

    unsigned int m_ReferenceCount;

    pthread_mutex_t m_Mutex;
};

#endif
