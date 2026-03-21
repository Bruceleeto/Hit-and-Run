//=============================================================================
// Copyright (C) 2002 Radical Entertainment Ltd.  All rights reserved.
//
// File:        soundmanager.h
//
// Description: Manager interface that the other game components use to
//              interact with sound.
//
// History:     01/06/2002 + Created -- Darren
//
//=============================================================================

#ifndef _SOUNDMANAGER_H
#define _SOUNDMANAGER_H

#include <Enums.h>
#include <data/gamedata.h>
#include <events/eventlistener.h>
#include <contexts/contextenum.h>

#ifdef RAD_WIN32
#include <data/config/gameconfig.h>
#endif

struct NISSoundLoadedCallback
{
    virtual void NISSoundLoaded() = 0;
};

struct NISSoundPlaybackCompleteCallback
{
    virtual void NISSoundPlaybackComplete() = 0;
};

enum SoundMode
{
    SOUND_MONO,
    SOUND_STEREO,
    SOUND_SURROUND
};

class Vehicle;
class Character;
class SoundFileHandler;

#ifdef RAD_NO_AUDIO

// Stub SoundManager — all methods are no-ops when audio is disabled.

#include <radkey.hpp>
#include <radmath/radmath.hpp>
namespace Scrooby { enum XLLanguage; }

#define SOUNDDEBUG_RENDER()

class SoundLoader;
class SoundDebugDisplay;
class DialogCoordinator;

class SoundManager : public EventListener,
                     #ifdef RAD_PC
                     public GameConfigHandler,
                     #endif
                     public GameDataHandler
{
    public:
        static SoundManager* CreateInstance( bool, bool, bool, bool )
        {
            if( !spInstance ) spInstance = new SoundManager();
            return spInstance;
        }
        static SoundManager* GetInstance() { return spInstance; }
        static void DestroyInstance() { delete spInstance; spInstance = 0; }
        void Initialize() {}

        void HandleEvent( EventEnum, void* ) {}
        void LoadSoundFile( const char*, SoundFileHandler* ) {}
        void Update() {}
        void UpdateOncePerFrame( unsigned int, ContextEnum, bool = true, bool = false ) {}
        void QueueLevelSoundLoads() {}
        void LoadCarSound( Vehicle*, bool ) {}
        void OnBootupStart() {}
        void OnBootupComplete() {}
        void OnFrontEndStart() {}
        void OnFrontEndEnd() {}
        void OnGameplayStart() {}
        void OnGameplayEnd( bool ) {}
        void OnPauseStart() {}
        void OnPauseEnd() {}
        void OnStoreScreenStart( bool = true ) {}
        void OnStoreScreenEnd() {}
        void DuckEverythingButMusicBegin( bool = false ) {}
        void DuckEverythingButMusicEnd( bool = false ) {}
        void OnMissionBriefingStart() {}
        void OnMissionBriefingEnd() {}
        void DuckForInGameCredits() {}
        void StopForMovie() {}
        void ResumeAfterMovie() {}
        bool IsStoppedForMovie() { return true; }
        void MuteNISPlayers() {}
        void UnmuteNISPlayers() {}
        void RestartSupersprintMusic() {}
        void SetSoundMode( SoundMode ) {}
        SoundMode GetSoundMode() { return SOUND_STEREO; }
        float GetBeatValue() { return 0.0f; }
        static bool IsFoodCharacter( Character* ) { return false; }
        void SetMasterVolume( float ) {}
        float GetMasterVolume() { return 0.0f; }
        void SetSfxVolume( float ) {}
        float GetSfxVolume() { return 0.0f; }
        void SetCarVolume( float ) {}
        float GetCarVolume() { return 0.0f; }
        void SetMusicVolume( float ) {}
        float GetMusicVolume() { return 0.0f; }
        void SetAmbienceVolume( float ) {}
        float GetAmbienceVolume() { return 0.0f; }
        void SetDialogueVolume( float ) {}
        float GetDialogueVolume() { return 1.0f; }
        float GetCalculatedAmbienceVolume() { return 0.0f; }
        void PlayCarOptionMenuStinger() {}
        void PlayDialogueOptionMenuStinger() {}
        void PlayMusicOptionMenuStinger() {}
        void PlaySfxOptionMenuStinger() {}
        void ResetDucking() {}
        void LoadNISSound( radKey32, NISSoundLoadedCallback* = 0 ) {}
        void PlayNISSound( radKey32, rmt::Box3D*, NISSoundPlaybackCompleteCallback* = 0 ) {}
        void StopAndDumpNISSound( radKey32 ) {}
        void SetDialogueLanguage( Scrooby::XLLanguage ) {}
        void DebugRender() {}
        SoundLoader* GetSoundLoader() { return 0; }
        SoundDebugDisplay* GetDebugDisplay() { return 0; }
        void SetMusicVolumeWithoutTuner( float ) {}
        void SetAmbienceVolumeWithoutTuner( float ) {}
        virtual void LoadData( const GameDataByte*, unsigned int ) {}
        virtual void SaveData( GameDataByte*, unsigned int ) {}
        virtual void ResetData() {}
        #ifdef RAD_PC
        virtual const char* GetConfigName() const { return "Sound"; }
        virtual int GetNumProperties() const { return 0; }
        virtual void LoadDefaults() {}
        virtual void LoadConfig( ConfigString& ) {}
        virtual void SaveConfig( ConfigString& ) {}
        #endif

        DialogCoordinator* m_dialogCoordinator;

    private:
        SoundManager() : m_dialogCoordinator( 0 ) {}
        ~SoundManager() {}
        static SoundManager* spInstance;
};

#else // !RAD_NO_AUDIO

#ifndef RAD_RELEASE
#define SOUNDDEBUG_ENABLED
#endif

#ifndef SOUNDDEBUG_ENABLED
#define SOUNDDEBUG_RENDER()
#else
#define SOUNDDEBUG_RENDER()  GetSoundManager()->DebugRender()
#endif

#include <sound/soundloader.h>
#include <sound/avatar/avatarsoundplayer.h>
#include <sound/listener.h>
#include <sound/nis/nissoundplayer.h>

class MusicPlayer;
class DialogCoordinator;
class SoundDebugDisplay;
class MovingSoundManager;
class SoundEffectPlayer;
struct IRadSoundClip;
struct IRadSoundClipPlayer;

//=============================================================================
//
// Synopsis:    SoundManager class declaration
//
//=============================================================================

class SoundManager : public EventListener,
                     #ifdef RAD_PC
                     public GameConfigHandler,  //ziemek: this is ugly..doh
                     #endif
                     public GameDataHandler
{
    public:
        //
        // Singleton accessors
        //
        static SoundManager* CreateInstance( bool muteSound, bool noMusic,
                                             bool noEffects, bool noDialogue );
        static SoundManager* GetInstance();
        static void DestroyInstance();
        void Initialize();

        //
        // EventListener interface
        //
        void HandleEvent( EventEnum id, void* pEventData );

        //
        // All-purpose sound file loading.  Coordinates the sound system
        // with the loading system
        //
        void LoadSoundFile( const char* filename, SoundFileHandler* callbackObj );

        //
        // Update routines.
        //
        void Update();
        // This one is for expensive stuff like positional sound calculations
        // that we can get away with doing once per frame
        void UpdateOncePerFrame( unsigned int elapsedTime,
                                 ContextEnum context,
                                 bool useContext = true,
                                 bool isPausedForErrors = false );

        // Prepare to load level sounds
        void QueueLevelSoundLoads();

        // Load the sounds associated with a car
        void LoadCarSound( Vehicle* theCar, bool unloadOtherCars );

        // Called when bootup context starts and ends
        void OnBootupStart();
        void OnBootupComplete();

        // Called when front end starts and ends
        void OnFrontEndStart();
        void OnFrontEndEnd();

        // Called when gameplay starts and ends
        void OnGameplayStart();
        void OnGameplayEnd( bool goingToFE );

        // Called when pause menu starts and ends (ends going back to gameplay,
        // not loading)
        void OnPauseStart();
        void OnPauseEnd();

        // Called for pseudo-pause stuff like clothing and phone booth
        //
        void OnStoreScreenStart( bool playMusic = true );
        void OnStoreScreenEnd();

        // Called when we want to kill everything but music (e.g. loading,
        // minigame standings)
        void DuckEverythingButMusicBegin( bool playMuzak = false );
        void DuckEverythingButMusicEnd( bool playMuzak = false );

        // Called when mission briefing screen starts and ends
        void OnMissionBriefingStart();
        void OnMissionBriefingEnd();

        // Called when in-game credits start
        //
        void DuckForInGameCredits();

        // Call these before and after FMVs
        void StopForMovie();
        void ResumeAfterMovie();
        bool IsStoppedForMovie();

        // Hack!  Need to mute gags during conversations
        void MuteNISPlayers();
        void UnmuteNISPlayers();

        //
        // Supersprint
        //
        void RestartSupersprintMusic();

        // Surround sound control
        void SetSoundMode( SoundMode mode );
        SoundMode GetSoundMode();

        // Function for getting that funky beat.  Values from 0.0f to 4.0f, assuming
        // that Marc doesn't write any waltzes
        float GetBeatValue();

        // Special case dialog handling
        static bool IsFoodCharacter( Character* theGuy );

        //
        // Volume controls.  Values range from 0.0f to 1.0f
        //
        void SetMasterVolume( float volume );
        float GetMasterVolume();

        void SetSfxVolume( float volume );
        float GetSfxVolume();

        void SetCarVolume( float volume );
        float GetCarVolume();

        void SetMusicVolume( float volume );
        float GetMusicVolume();

        void SetAmbienceVolume( float volume );
        float GetAmbienceVolume();

        void SetDialogueVolume( float volume );
        float GetDialogueVolume();

        float GetCalculatedAmbienceVolume();

        //
        // Option menu stinger stuff
        //
        void PlayCarOptionMenuStinger();
        void PlayDialogueOptionMenuStinger();
        void PlayMusicOptionMenuStinger();
        void PlaySfxOptionMenuStinger();

        //
        // Ducking stuff
        //
        void ResetDucking();

        //
        // NIS Interface
        //
        void LoadNISSound( radKey32 NISSoundID, NISSoundLoadedCallback* callback = NULL );
        void PlayNISSound( radKey32 NISSoundID, rmt::Box3D* boundingBox, NISSoundPlaybackCompleteCallback* callback = NULL );
        void StopAndDumpNISSound( radKey32 NISSoundID );

        //
        // Language interface
        //
        void SetDialogueLanguage( Scrooby::XLLanguage language );

        //
        // Sound debug functions
        //
        void DebugRender();

        //
        // TODO: move these functions, they're not intended for use outside
        // of the sound system
        //
        SoundLoader* GetSoundLoader() { return( m_soundLoader ); }
        SoundDebugDisplay* GetDebugDisplay() { return( m_debugDisplay ); }

        //
        // This should NOT be called outside the sound system.  Unfortunately,
        // to keep things clean, what I should do is split the MusicPlayer class
        // and move a low-level controller into the soundrenderer layer.  I don't
        // have time for this.  Things to do for the next round.
        //
        void SetMusicVolumeWithoutTuner( float volume );
        void SetAmbienceVolumeWithoutTuner( float volume );

        // Implements GameDataHandler
        //
        virtual void LoadData( const GameDataByte* dataBuffer, unsigned int numBytes );
        virtual void SaveData( GameDataByte* dataBuffer, unsigned int numBytes );
        virtual void ResetData();

        #ifdef RAD_PC
        // Implementation of the GameConfigHandler interface
        virtual const char* GetConfigName() const;
        virtual int GetNumProperties() const;
        virtual void LoadDefaults();
        virtual void LoadConfig( ConfigString& config );
        virtual void SaveConfig( ConfigString& config );
        #endif

        DialogCoordinator* m_dialogCoordinator;

    protected:
        //
        // Hide the SoundManager constructor and destructor so everyone
        // is forced to use singleton accessors
        //
        SoundManager( bool noSound, bool noMusic, bool noEffects, bool noDialogue );
        ~SoundManager();

    private:
        //Prevent wasteful constructor creation.
        SoundManager( const SoundManager& original );
        SoundManager& operator=( const SoundManager& rhs );

        //
        // Hack!
        //
        void prepareStartupSounds();
        void playStartupAcceptSound();
        void playStartupScrollSound();
        void dumpStartupSounds();

        // Pointer to the one and only instance of this singleton.
        static SoundManager* spInstance;

        struct SoundSettings
        {
            float musicVolume;
            float sfxVolume;
            float carVolume;
            float dialogVolume;
            bool isSurround;
        };

        // Sound loading subsystem
        SoundLoader* m_soundLoader;

        // Avatar sound subsystem
        AvatarSoundPlayer m_avatarSoundPlayer;

        // Music player subsystem
        MusicPlayer* m_musicPlayer;

        // Sound effect subsystem
        SoundEffectPlayer* m_soundFXPlayer;

        // Dialog subsystem


        // NIS subsystem
        NISSoundPlayer* m_NISPlayer;

        // RadSound listener update
        Listener m_listener;

        // AI Vehicle sound subsystem
        MovingSoundManager* m_movingSoundManager;

        // Sound debug display subsystem
        SoundDebugDisplay* m_debugDisplay;

        // Mute options
        bool m_isMuted;

        bool m_noMusic;
        bool m_noEffects;
        bool m_noDialogue;

        // Pointer to sound rendering interface
        Sound::daSoundRenderingManager* m_pSoundRenderMgr;

        // [ps] avoid hammering on pause.
        bool m_stoppedForMovie;

        //
        // Hack for stinky pre-script-loading menu sounds
        //
        IRadSoundClip* m_selectSoundClip;
        IRadSoundClip* m_scrollSoundClip;

        IRadSoundClipPlayer* m_selectSoundClipPlayer;
        IRadSoundClipPlayer* m_scrollSoundClipPlayer;


        SoundMode m_soundMode;
};

#endif // RAD_NO_AUDIO

// A little syntactic sugar for getting at this singleton.
inline SoundManager* GetSoundManager() { return( SoundManager::GetInstance() ); }

#endif //SOUNDMANAGER_H

