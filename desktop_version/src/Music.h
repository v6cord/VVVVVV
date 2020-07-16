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

	void play(int t, const double position_sec = 0.0, const int fadein_ms = 3000);
	void play(int t, int fadeintime);
	void resume(const int fadein_ms = 0);
	void haltdasmusik();
	void silencedasmusik();
	void fadeMusicVolumeIn(int ms);
	void fadeout();
	void fadein();
	void processmusicfadein();
	void processmusic();
	void niceplay(int t);

	void changemusicarea(int x, int y);

	int currentsong = 0;
	int resumesong = false;

	void playfile(const char* t, std::string track, int loops, bool internal = false);
	void stopfile(std::string track);

	void playef(int t);

        std::vector<SoundTrack> soundTracks;
        std::vector<MusicTrack> musicTracks;
	SoundSystem soundSystem;
	bool safeToProcessMusic = true;

	int nicechange = 0;
	int nicefade = 0;

	bool m_doFadeInVol = false;
	int FadeVolAmountPerFrame = 0;
	int musicVolume = 0;

	int fadeoutqueuesong = -1; // -1 if no song queued
	bool dontquickfade = false;

	// MMMMMM mod settings
	bool mmmmmm = false;
	bool usingmmmmmm = false;
        bool loaded = false;

	int num_pppppp_tracks = 0;
	int num_mmmmmm_tracks = 0;

	Uint64 songStart = 0;
	Uint64 songEnd = 0;

        std::unordered_map<std::string, SoundTrack> custom_files;
        std::unordered_map<std::string, int> custom_file_channels;
        std::unordered_map<int, std::string> custom_channel_paths;
        std::unordered_map<std::string, std::string> custom_file_paths;
        std::unordered_map<std::string, int> custom_file_loops;
};

extern musicclass music;

#endif /* MUSIC_H */
