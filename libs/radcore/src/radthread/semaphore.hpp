//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#ifndef	SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include <radobject.hpp>
#include <radmemory.hpp>
#include <radthread.hpp>
#include <semaphore.h>

class radThreadSemaphore : public IRadThreadSemaphore,
                           public radObject
{
    public:

    radThreadSemaphore( unsigned int count );
    ~radThreadSemaphore( void );

    virtual void Wait( void );
    virtual void Signal( void );

    virtual void AddRef( void );
    virtual void Release( void );

    #ifdef RAD_DEBUG
    virtual void Dump( char* pStringBuffer, unsigned int bufferSize );
    #endif

    private:

    unsigned int m_ReferenceCount;

    sem_t m_Semaphore;
};

#endif
