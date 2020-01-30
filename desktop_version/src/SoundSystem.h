#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

#include <SDL_mixer.h>

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
	SoundSystem();
	void playMusic(MusicTrack* music);
};

#endif /* SOUNDSYSTEM_H */
