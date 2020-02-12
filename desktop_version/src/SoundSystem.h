#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

#ifndef NO_SDL_MIXER
#if defined(STRICT_SDL_PATH) && !defined(__ANDROID__)
	#include <SDL2/SDL_mixer.h>
#else
	#include <SDL_mixer.h>
#endif
#else
#define Mix_Chunk void
#define Mix_FadeInMusic(...) 0
#define Mix_FadeOutChannel(...) 0
#define Mix_FadeOutMusic(...) 0
#define Mix_FadingMusic(...) 0
#define Mix_FreeChunk(...) 0
#define Mix_FreeMusic(...) 0
#define Mix_GetError(...) "stubbed on 3DS"
#define Mix_HaltChannel(...) 0
#define Mix_HaltMusic(...) 0
#define Mix_LoadMUS(...) 0
#define Mix_LoadMUS_RW(...) 0
#define Mix_LoadWAV_RW(...) 0
#define Mix_Music void
#define Mix_OpenAudio(...) 0
#define Mix_Pause(...) 0
#define Mix_PauseMusic(...) 0
#define Mix_PlayChannel(...) 0
#define Mix_PlayingMusic(...) 0
#define Mix_PlayMusic(...) 0
#define Mix_Resume(...) 0
#define Mix_ResumeMusic(...) 0
#define Mix_Volume(...) 0
#define Mix_VolumeMusic(...) 0
#define MIX_FADING_OUT 0
#define MIX_MAX_VOLUME 0
#define MIX_NO_FADING 0
#endif

class MusicTrack
{
public:
	MusicTrack(const char* fileName);
	MusicTrack(SDL_RWops *rw);
        MusicTrack(MusicTrack&& moved);
        MusicTrack& operator=(const MusicTrack& other) = default;
        ~MusicTrack();
	Mix_Music *m_music;
	bool m_isValid = false;
};

class SoundTrack
{
public:
	SoundTrack(const char* fileName);
        SoundTrack(SoundTrack&& moved);
        SoundTrack& operator=(const SoundTrack& other) = default;
        SoundTrack() = default;
        ~SoundTrack();
	Mix_Chunk *sound;
        bool isValid = false;
};

class SoundSystem
{
public:
	SoundSystem() = default;
        void init();
	void playMusic(MusicTrack* music);
};

#endif /* SOUNDSYSTEM_H */
