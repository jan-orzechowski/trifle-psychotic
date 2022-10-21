﻿#pragma once

#include "main.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

struct sdl_data
{
	b32 initialized;
	b32 fullscreen;

	SDL_Window* window;
	SDL_Renderer* renderer;

	SDL_Texture* tileset_texture;
	SDL_Texture* ui_font_texture;
	SDL_Texture* title_font_texture;
	SDL_Texture* charset_texture;
	SDL_Texture* explosion_texture;
	SDL_Texture* background_desert_texture;
	SDL_Texture* background_ice_desert_texture;
	SDL_Texture* background_clouds_texture;
	SDL_Texture* background_red_planet_sky_texture;
	SDL_Texture* background_red_planet_desert_texture;
	SDL_Texture* background_planet_orbit_texture;
	SDL_Texture* background_title_screen_texture;

	string_builder path_buffer;

	string_ref preferences_file_path;
	Mix_Music* music;
};

struct write_to_tile
{
	void* buffer;
	u32 length;
};

read_file_result read_file(const char* path);
void save_file(const char* path, write_to_tile contents);
read_file_result load_prefs();
void save_prefs(write_to_tile contents);

void start_playing_music(string_ref audio_file_name);
void stop_playing_music(int fade_out_ms);

void render_group_to_output(render_group* render_group);