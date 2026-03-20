//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#include <radoptions.hpp>

#ifndef RAD_MOVIEPLAYER_USE_BINK

#include <raddebug.hpp>
#include <radmath/radmath.hpp>
#include "ffmpegmovieplayer.hpp"

//=============================================================================
// FMV playback stubbed out — no FFmpeg dependency
//=============================================================================

const char * radMovieDebugChannel2 = "radMovie";
unsigned int const radMovie_NoAudioTrack = 0xFFFFFFFF;

template<> radMoviePlayer* radLinkedClass< radMoviePlayer >::s_pLinkedClassHead = NULL;
template<> radMoviePlayer* radLinkedClass< radMoviePlayer >::s_pLinkedClassTail = NULL;

radMoviePlayer::radMoviePlayer( void )
    :
    radRefCount( 0 ),
    m_refIRadMovieRenderStrategy( NULL ),
    m_refIRadMovieRenderLoop( NULL ),
    m_refIRadStopwatch( NULL ),
    m_State( IRadMoviePlayer2::NoData ),
    m_VideoFrameState( VideoFrame_Unlocked ),
    m_VideoTrackIndex( 0 ),
    m_AudioTrackIndex( 0 ),
    m_PresentationTime( 0 ),
    m_PresentationDuration( 0 ),
    m_pFormatCtx( NULL ),
    m_pVideoCtx( NULL ),
    m_pAudioCtx( NULL ),
    m_pSwsCtx( NULL ),
    m_pSwrCtx( NULL ),
    m_pPacket( NULL ),
    m_pVideoFrame( NULL ),
    m_pAudioFrame( NULL ),
    m_AudioSource( 0 )
{
}

radMoviePlayer::~radMoviePlayer( void ) { }

void radMoviePlayer::Initialize( IRadMovieRenderLoop * pIRadMovieRenderLoop, IRadMovieRenderStrategy * pIRadMovieRenderStrategy )
{
    m_refIRadMovieRenderLoop = pIRadMovieRenderLoop;
    m_refIRadMovieRenderStrategy = pIRadMovieRenderStrategy;
    SetState( IRadMoviePlayer2::NoData );
}

bool radMoviePlayer::Render( void ) { return false; }

void radMoviePlayer::Load( const char * pVideoFileName, unsigned int audioTrackIndex )
{
    rDebugPrintf( "radMoviePlayer: FMV disabled, skipping %s\n", pVideoFileName );
    SetState( IRadMoviePlayer2::NoData );
}

void radMoviePlayer::Unload( void )
{
    SetState( IRadMoviePlayer2::NoData );
}

void radMoviePlayer::Play( void ) { }
void radMoviePlayer::Pause( void ) { }
void radMoviePlayer::SetPan( float pan ) { }
float radMoviePlayer::GetPan( void ) { return 0.0f; }
void radMoviePlayer::SetVolume( float volume ) { }
float radMoviePlayer::GetVolume( void ) { return 1.0f; }
IRadMoviePlayer2::State radMoviePlayer::GetState( void ) { return m_State; }

bool radMoviePlayer::GetVideoFrameInfo( VideoFrameInfo * frameInfo )
{
    return false;
}

float radMoviePlayer::GetFrameRate( void ) { return 1000.0f; }
unsigned int radMoviePlayer::GetCurrentFrameNumber( void ) { return m_PresentationTime; }

void radMoviePlayer::Service( void ) { }

void radMoviePlayer::SetState( IRadMoviePlayer2::State state )
{
    m_State = state;
}

void radMoviePlayer::InternalPlay( void ) { }
void radMoviePlayer::FlushAudioQueue( void ) { }

IRadMoviePlayer2 * radMoviePlayerCreate2( radMemoryAllocator alloc )
{
    return new( alloc ) radMoviePlayer( );
}

void radMovieInitialize2( radMemoryAllocator alloc ) { }
void radMovieTerminate2( void ) { }
void radMovieService2( void ) { }

#endif // ! RAD_MOVIEPLAYER_USE_BINK
