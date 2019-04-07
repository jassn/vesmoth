/* $Id: sound.cpp 28 2003-09-19 10:21:25Z zas $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "sound.hpp"

#include "SDL_mixer.h"

#include <iostream>
#include <map>

namespace {

bool mix_ok = false;
std::map<std::string,Mix_Chunk*> sound_cache;
std::map<std::string,Mix_Music*> music_cache;

std::string current_music = "";

bool music_off = false;
bool sound_off = false;

}

namespace sound {

manager::manager()
{
	const int res =
	       Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT,2,1024);
	if(res >= 0) {
		mix_ok = true;
		Mix_AllocateChannels(8);
	} else {
		mix_ok = false;
		std::cerr << "Could not initialize audio: " << SDL_GetError() << "\n";
	}
}

manager::~manager()
{
	if(!mix_ok)
		return;

	Mix_HaltMusic();
	Mix_HaltChannel(-1);

	for(std::map<std::string,Mix_Chunk*>::iterator i = sound_cache.begin();
	    i != sound_cache.end(); ++i) {
		Mix_FreeChunk(i->second);
	}

	for(std::map<std::string,Mix_Music*>::iterator j = music_cache.begin();
	    j != music_cache.end(); ++j) {
		Mix_FreeMusic(j->second);
	}

	Mix_CloseAudio();
}

void play_music(const std::string& file)
{
	if(!mix_ok || current_music == file)
		return;

	if(music_off) {
		current_music = file;
		return;
	}

	std::map<std::string,Mix_Music*>::const_iterator itor =
	                                              music_cache.find(file);
	if(itor == music_cache.end()) {
		static const std::string music_prefix = "music/";

		std::string filename;
		Mix_Music* music = NULL;

#ifdef WESNOTH_PATH
		filename = WESNOTH_PATH + std::string("/") + music_prefix + file;
		music = Mix_LoadMUS(filename.c_str());
#endif

		if(music == NULL) {
			filename = music_prefix + file;
			music = Mix_LoadMUS(filename.c_str());
		}

		if(music == NULL) {
			std::cerr << "Could not load music file '" << filename << "': "
			          << SDL_GetError() << "\n";
			return;
		}

		itor = music_cache.insert(std::pair<std::string,Mix_Music*>(
		                                            file,music)).first;
	}

	if(Mix_PlayingMusic()) {
		Mix_FadeOutMusic(500);
	}

	const int res = Mix_FadeInMusic(itor->second,-1,500);
	if(res < 0) {
		std::cerr << "Could not play music: " << SDL_GetError() << "\n";
	}

	current_music = file;
}

void play_sound(const std::string& file)
{
	if(!mix_ok || sound_off)
		return;

	std::map<std::string,Mix_Chunk*>::const_iterator itor =
	                                              sound_cache.find(file);
	if(itor == sound_cache.end()) {
		static const std::string sound_prefix = "sounds/";
		std::string filename;
		Mix_Chunk* sfx = NULL;

#ifdef WESNOTH_PATH
		filename = WESNOTH_PATH + std::string("/") + sound_prefix + file;
		sfx = Mix_LoadWAV(filename.c_str());
#endif

		if(sfx == NULL) {
			filename = sound_prefix + file;
			sfx = Mix_LoadWAV(filename.c_str());
		}

		if(sfx == NULL) {
			std::cerr << "Could not load sound file '" << filename << "': "
			          << SDL_GetError() << "\n";
			return;
		}

		itor = sound_cache.insert(std::pair<std::string,Mix_Chunk*>(
		                                            file,sfx)).first;
	}

	//play on the first available channel
	const int res = Mix_PlayChannel(-1,itor->second,0);
	if(res < 0) {
		std::cerr << "error playing sound effect: " << SDL_GetError() << "\n";
	}
}

void set_music_volume(double vol)
{
	if(!mix_ok)
		return;

	if(vol < 0.05) {
		Mix_HaltMusic();
		music_off = true;
		return;
	}

	Mix_VolumeMusic(int(vol*double(MIX_MAX_VOLUME)));

	//if the music was off completely, start playing it again now
	if(music_off) {
		music_off = false;
		const std::string music = current_music;
		current_music = "";
		if(!music.empty()) {
			play_music(music);
		}
	}
}

void set_sound_volume(double vol)
{
	if(!mix_ok)
		return;

	if(vol < 0.05) {
		sound_off = true;
		return;
	} else {
		sound_off = false;
		Mix_Volume(-1,int(vol*double(MIX_MAX_VOLUME)));
	}
}

}