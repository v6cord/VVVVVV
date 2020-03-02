#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

#if defined(__SWITCH__)
	#include <SDL2/SDL_mixer.h>
#elif defined(__ANDROID__)
	#include <SDL_mixer.h>
#else
	#include <SDL2/SDL_mixer_ext.h>
#endif

class MusicTrack
{
public:
	MusicTrack(const char* fileName);
	MusicTrack(SDL_RWops *rw);
        MusicTrack(MusicTrack&& moved);
        MusicTrack& operator=(const MusicTrack& other) = default;
	MusicTrack() = default;
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
	SoundSystem();
        void init();
	void playMusic(MusicTrack* music);
};

#endif /* SOUNDSYSTEM_H */
