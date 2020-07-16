#include <SDL.h>
#include <stdio.h>
#include <utility>
#include <physfs.h>
#include "Music.h"
#include "Script.h"
#include "BinaryBlob.h"
#include "Map.h"

void songend();

void musicclass::init()
{
	if (!loaded) soundSystem.init();

	soundTracks.clear();
	musicTracks.clear();

	soundTracks.push_back(std::move(SoundTrack( "sounds/jump.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/jump2.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/hurt.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/souleyeminijingle.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/coin.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/save.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/crumble.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/vanish.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/blip.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/preteleport.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/teleport.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/crew1.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/crew2.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/crew3.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/crew4.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/crew5.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/crew6.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/terminal.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/gamesaved.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/crashing.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/blip2.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/countdown.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/go.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/crash.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/combine.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/newrecord.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/trophy.wav" )));
	soundTracks.push_back(std::move(SoundTrack( "sounds/rescue.wav" )));

#ifdef VVV_COMPILEMUSIC
	binaryBlob musicWriteBlob;
#define FOREACH_TRACK(track_name) musicWriteBlob.AddFileToBinaryBlob(track_name);
	TRACK_NAMES
#undef FOREACH_TRACK

	musicWriteBlob.writeBinaryBlob("data/BinaryMusic.vvv");
#endif

	binaryBlob musicReadBlob;
	num_mmmmmm_tracks = 0;
	num_pppppp_tracks = 0;

	if (!musicReadBlob.unPackBinary("mmmmmm.vvv"))
	{
		mmmmmm = false;
		if (!loaded) usingmmmmmm=false;
		bool ohCrap = musicReadBlob.unPackBinary("vvvvvvmusic.vvv");
		SDL_assert(ohCrap && "Music not found!");
	}
	else
	{
		mmmmmm = true;
		if (!loaded) usingmmmmmm = true;
		int index;
		SDL_RWops *rw;

#define FOREACH_TRACK(track_name) \
	index = musicReadBlob.getIndex(track_name); \
	rw = SDL_RWFromMem(musicReadBlob.getAddress(index), musicReadBlob.getSize(index)); \
	musicTracks.push_back(MusicTrack( rw ));

		TRACK_NAMES

		num_mmmmmm_tracks += 16;

		const std::vector<int> extra = musicReadBlob.getExtra();
		for (size_t i = 0; i < extra.size(); i++)
		{
			const int& index = extra[i];
			rw = SDL_RWFromMem(musicReadBlob.getAddress(index), musicReadBlob.getSize(index));
			musicTracks.push_back(MusicTrack( rw ));

			num_mmmmmm_tracks++;
		}

		bool ohCrap = musicReadBlob.unPackBinary("vvvvvvmusic.vvv");
		SDL_assert(ohCrap && "Music not found!");
	}

	int index;
	SDL_RWops *rw;

	TRACK_NAMES

#undef FOREACH_TRACK

	num_pppppp_tracks += 16;

	const std::vector<int> extra = musicReadBlob.getExtra();
	for (size_t i = 0; i < extra.size(); i++)
	{
		const int& index = extra[i];
		rw = SDL_RWFromMem(musicReadBlob.getAddress(index), musicReadBlob.getSize(index));
		musicTracks.push_back(MusicTrack( rw ));

		num_pppppp_tracks++;
	}

	safeToProcessMusic= false;
	m_doFadeInVol = false;
	musicVolume = 128;
	FadeVolAmountPerFrame = 0;

	currentsong = 0;
	nicechange = 0;
	nicefade = 0;
	resumesong = 0;
	fadeoutqueuesong = -1;
	dontquickfade = false;

	songStart = 0;
	songEnd = 0;

	Mix_HookMusicFinished(&songend);

	loaded = true;
}

void songend()
{
	music.songEnd = SDL_GetPerformanceCounter();
	music.currentsong = -1;
}

void musicclass::play(int t, int fadeintime) {
	play(t, 0, fadeintime);
}

void musicclass::play(int t, const double position_sec /*= 0.0*/, const int fadein_ms /*= 3000*/)
{
	// No need to check if num_tracks is greater than 0, we wouldn't be here if it wasn't
	if (mmmmmm && usingmmmmmm)
	{
		t %= num_mmmmmm_tracks;
	}
	else
	{
		t %= num_pppppp_tracks;
	}

	if(mmmmmm && !usingmmmmmm)
	{
		t += num_mmmmmm_tracks;
	}
	safeToProcessMusic = true;
	Mix_VolumeMusic(128);
	if (currentsong !=t)
	{
		haltdasmusik();
		if (t != -1)
		{
			currentsong = t;
			if (currentsong == 0 || currentsong == 7 || (!map.custommode && (currentsong == 0+num_pppppp_tracks || currentsong == 7+num_pppppp_tracks)))
			{
				// Level Complete theme, no fade in or repeat
				if(Mix_FadeInMusicPos(musicTracks[t].m_music, 0, 0, position_sec)==-1)
				{
					printf("Mix_FadeInMusicPos: %s\n", Mix_GetError());
				}
			}
			else
			{
				if (Mix_FadingMusic() == MIX_FADING_OUT) {
					// We're already fading out
					fadeoutqueuesong = t;
					currentsong = -1;
					if (!dontquickfade)
						Mix_FadeOutMusic(500); // fade out quicker
					else
						dontquickfade = false;
				}
				else if(Mix_FadeInMusicPos(musicTracks[t].m_music, -1, fadein_ms, position_sec)==-1)
				{
					printf("Mix_FadeInMusicPos: %s\n", Mix_GetError());
				}
			}

			songStart = SDL_GetPerformanceCounter();
		}
		else
		{
			currentsong = -1;
		}
	}
}

void musicclass::resume(const int fadein_ms /*= 0*/)
{
	const double offset = static_cast<double>(songEnd - songStart);
	const double frequency = static_cast<double>(SDL_GetPerformanceFrequency());

	const double position_sec = offset / frequency;

	play(resumesong, position_sec, fadein_ms);
}

void musicclass::fadein()
{
	resume(3000); // 3000 ms fadein
}

void musicclass::haltdasmusik()
{
	Mix_HaltMusic();
	resumesong = currentsong;
	currentsong = -1;
	for (auto&& [id, channel] : custom_file_channels) {
		Mix_HaltChannel(channel);
	}
	custom_file_channels.clear();
	custom_file_paths.clear();
}

void musicclass::silencedasmusik()
{
	Mix_VolumeMusic(0) ;
	musicVolume = 0;
	for (auto&& [id, channel] : custom_file_channels) {
		Mix_Volume(channel, 0);
	}
}

void musicclass::fadeMusicVolumeIn(int ms)
{
	m_doFadeInVol = true;
	FadeVolAmountPerFrame =  MIX_MAX_VOLUME / (ms / 33);
}

void musicclass::fadeout()
{
	Mix_FadeOutMusic(2000);
	resumesong = currentsong;
	currentsong = -1;
	for (auto&& [id, channel] : custom_file_channels) {
		Mix_FadeOutChannel(channel, 2000);
	}
	custom_file_channels.clear();
	custom_file_paths.clear();
}

void musicclass::processmusicfadein()
{
	musicVolume += FadeVolAmountPerFrame;
	Mix_VolumeMusic(musicVolume);
	for (auto&& [id, channel] : custom_file_channels) {
		Mix_Volume(channel, musicVolume);
	}
	if (musicVolume >= MIX_MAX_VOLUME)
	{
		m_doFadeInVol = false;
	}
}

void musicclass::processmusic()
{
	if(!safeToProcessMusic)
	{
		return;
	}

	if (fadeoutqueuesong != -1 && Mix_PlayingMusic() == 0) {
		play(fadeoutqueuesong);
		fadeoutqueuesong = -1;
	}

	if (nicefade == 1 && Mix_PlayingMusic() == 0)
	{
		play(nicechange);
		nicechange = -1; nicefade = 0;
	}

	if(m_doFadeInVol)
	{
		processmusicfadein();
	}

	auto iter = custom_channel_paths.begin();
	while (iter != custom_channel_paths.end()) {
		if (!Mix_Playing(iter->first)) {
			script.setvar("path", iter->second);
			script.callback("on_custom_sfx_end");
			iter = custom_channel_paths.erase(iter);
		} else {
			++iter;
		}
	}
}


void musicclass::niceplay(int t)
{
	// important: do nothing if the correct song is playing!
	if((!mmmmmm && currentsong!=t) || (mmmmmm && usingmmmmmm && currentsong!=t) || (mmmmmm && !usingmmmmmm && currentsong!=t+16))
	{
		if(currentsong!=-1)
		{
			dontquickfade = true;
			fadeout();
		}
		nicefade = 1;
		nicechange = t;
	}
}

void musicclass::changemusicarea(int x, int y)
{
	switch(musicroom(x, y))
	{
	case musicroom(11, 4):
		niceplay(2);
		break;

	case musicroom(2, 4):
	case musicroom(7, 15):
		niceplay(3);
		break;

	case musicroom(18, 1):
	case musicroom(15, 0):
		niceplay(12);
		break;

	case musicroom(0, 0):
	case musicroom(0, 16):
	case musicroom(2, 11):
	case musicroom(7, 9):
	case musicroom(8, 11):
	case musicroom(13, 2):
	case musicroom(17, 12):
	case musicroom(14, 19):
	case musicroom(17, 17):
		niceplay(4);
		break;

	default:
		niceplay(1);
		break;
	}
}

void musicclass::playfile(const char* t, std::string track, int loops, bool internal /*= false*/)
{
	std::string temp;
	if (internal) {
		--t; // weird pointer bug?
		temp = t;
		temp += ".ogg";
		t = temp.c_str();
		if (PHYSFS_getRealDir(t) != PHYSFS_getRealDir("VVVVVV.png")) return;
	}

	int channel = 0;

	auto[pair, inserted] = custom_files.insert(std::make_pair(t, SoundTrack()));
	if (inserted) {
		SoundTrack track(t);
		pair->second.sound = track.sound;
		pair->second.isValid = track.isValid;
		track.isValid = false;
	}

	if (track != "") {
		custom_file_loops[track] = loops;
	}

	if (loops != -1) {
		--loops;
	}

	if (track != "") {
		if (custom_file_paths[track] != t) {
			stopfile(track);
			custom_file_paths[track] = t;
		}
	} else {
		channel = Mix_PlayChannel(-1, pair->second.sound, loops);
	}

	if (channel == -1) {
		fprintf(stderr, "Unable to play WAV file: %s\n", Mix_GetError());
		return;
	} else if (track != "") {
		custom_file_channels[track] = channel;
	}

	if (track == "" || loops >= 0) {
		custom_channel_paths[channel] = t;
	}
}

void musicclass::stopfile(std::string track) {
	auto iter = custom_file_channels.find(track);
	if (iter != custom_file_channels.end()) {
		Mix_FadeOutChannel(iter->second, 100);
		custom_file_channels.erase(iter);
	}
	custom_file_paths.erase(track);
}

void musicclass::playef(int t)
{
	if (t < 0 || t >= (int) soundTracks.size())
	{
		return;
	}
	int channel;

	channel = Mix_PlayChannel(-1, soundTracks[t].sound, 0);
	if(channel == -1)
	{
		fprintf(stderr, "Unable to play WAV file: %s\n", Mix_GetError());
	}
}
