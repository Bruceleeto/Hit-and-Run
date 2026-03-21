//=============================================================================
// Copyright (C) 2002 Radical Entertainment Ltd.  All rights reserved.
//
// File:        .h
//
// Description: Blahblahblah
//
// History:     17/04/2002 + Created -- NAME
//
//=============================================================================

#ifndef FMVPLAYER_H
#define FMVPLAYER_H

//========================================
// Nested Includes
//========================================

#include <presentation/animplayer.h>
#ifndef RAD_NO_AUDIO
#include <radmovie2.hpp>
#endif

//========================================
// Forward References
//========================================

class FMVUserInputHandler;
#ifdef RAD_NO_AUDIO
struct IRadMoviePlayer2;
#endif

//=============================================================================
//
// Synopsis:    Blahblahblah
//
//=============================================================================

class FMVPlayer : public AnimationPlayer,
#ifndef RAD_NO_AUDIO
                  public IRadMovieRenderLoop,
                  public IRadDriveCompletionCallback,
#endif
                  public radRefCount

{
    IMPLEMENT_REFCOUNTED( "FMVPlayer" )

public:

    FMVPlayer();
    virtual ~FMVPlayer();

    // playback control
    virtual void Play();
	virtual void Abort(void);
    virtual void Stop();
    virtual void Pause();
    virtual void UnPause();

#ifdef RAD_WIN32
    void ForceStop();
#endif

    // loading
    virtual void PreLoad(void) { SetState(ANIM_LOADING);}
    virtual void LoadData( const char* fileName, bool bInInventory, void* pUserData );

    // animation updating (doesn't need any)
    virtual void Update( unsigned int elapsedTime ) {};

#ifndef RAD_NO_AUDIO
    // IRadMovieRenderLoop interface, called by radMovie service eac time a frame is ready
    void IterateLoop( IRadMoviePlayer2* pIRadMoviePlayer );
#endif

    // reset all internal data
    virtual void ClearData();

    // How long the FMV has played in seconds. 0 if the movie hasn't played yet.
    float GetElapsedTime() { return mElapsedTime; }

    inline FMVUserInputHandler* GetUserInputHandler() const
    {
        return m_UserInputHandler;
    }

protected:
#ifndef RAD_NO_AUDIO
    void Initialize( radMemoryAllocator Allocator );
#endif

    // AnimationPlayer interface
    virtual void DoLoaded() {}; // Movies aren't loader by main game loader, so don't need to handle this
    virtual void DoRender(); // render a frame (if one is avilible)

#ifndef RAD_NO_AUDIO
    // Implements IRadDriveCompletionCallback. We'll be called this when the movie finishes streaming.
    //We need to wait for the drive to finish so the internal memory in radmovie is freed.
    virtual void OnDriveOperationsComplete( void* pUserData );
#endif
	void FadeScreen(float Alpha);

private:
    //Prevent wasteful constructor creation.
    FMVPlayer( const FMVPlayer& fmvPlayer );
    FMVPlayer& operator=( const FMVPlayer& fmvPlayer );

    FMVUserInputHandler* m_UserInputHandler;
#ifndef RAD_NO_AUDIO
    ref< IRadMoviePlayer2 > m_refIRadMoviePlayer;
#endif

    bool mFrameReady;
    float mElapsedTime; // Elapsed playing time. So you know if you should let the player skip the movie.
    bool mDriveFinished;
	float mFadeOut;
	float mMovieVolume;
};


#endif //FMVPLAYER_H

