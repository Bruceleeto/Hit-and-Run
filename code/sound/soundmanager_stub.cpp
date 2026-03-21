// Stub SoundManager for RAD_NO_AUDIO builds.
// Provides the static singleton pointer so the linker is happy.

#ifdef RAD_NO_AUDIO

#include <sound/soundmanager.h>

SoundManager* SoundManager::spInstance = 0;

#endif // RAD_NO_AUDIO
