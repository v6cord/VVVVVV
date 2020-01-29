#ifndef MUSIC_H
#define MUSIC_H

#include "SoundSystem.h"

#include <vector>
#include <string>
#include <unordered_map>

#define musicroom(rx, ry) ((rx) + ((ry) * 20))

class musicclass
{
public:
	void init();

	void play(int t, int fadeintime = 3000);
	void loopmusic();
	void stopmusic();
	void haltdasmusik();
	void silencedasmusik();
	void fadeMusicVolumeIn(int ms);
	void fadeout();
	void processmusicfade();
	void processmusicfadein();
	void processmusic();
	void niceplay(int t);

	void changemusicarea(int x, int y);

	// public var musicchan:Array = new Array();
	// public var musicchannel:SoundChannel, musicchannel2:SoundChannel;
	// public var currentmusicchan:int, musicchanlen:int, musicchancur:int, musicstopother:int, resumesong:int;
	// public var currentsong:int, musicfade:int, musicfadein:int;
	int currentsong, musicfade, musicfadein = 0;
	int resumesong = false;

	//public var nicefade:int, nicechange:int;

	// Play a sound effect! There are 16 channels, which iterate
	void initefchannels();

	void playfile(const char* t, std::string track);
	void stopfile(std::string track);

	void playef(int t, int offset = 0);

        std::vector<SoundTrack> soundTracks;
        std::vector<MusicTrack> musicTracks;
	SoundSystem soundSystem;
	bool safeToProcessMusic = true;

	int nicechange = 0;
	int nicefade = 0;

	bool m_doFadeInVol = false;
	int FadeVolAmountPerFrame = 0;
	int musicVolume = 0;

	float volume = 1.0;

	bool custompd = false;

	int fadeoutqueuesong = -1; // -1 if no song queued
	bool dontquickfade = false;

	// MMMMMM mod settings
	bool mmmmmm = false;
	bool usingmmmmmm = false;

        bool muted = false;

        std::unordered_map<std::string, SoundTrack> custom_files;
        std::unordered_map<std::string, int> custom_file_channels;
        std::unordered_map<std::string, std::string> custom_file_paths;
};

extern musicclass music;

#endif /* MUSIC_H */
