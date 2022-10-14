#pragma once

#include "main.h"
#include <SDL.h>
#include <SDL_image.h>

struct sdl_data
{
	b32 initialized;
	b32 fullscreen;

	SDL_Window* window;
	SDL_Renderer* renderer;

	SDL_Texture* tileset_texture;
	SDL_Texture* font_texture;
	SDL_Texture* charset_texture;
	SDL_Texture* explosion_texture;
	SDL_Texture* background_desert_texture;
	SDL_Texture* background_ice_desert_texture;
	SDL_Texture* background_clouds_texture;
	SDL_Texture* background_red_planet_sky_texture;
	SDL_Texture* background_red_planet_desert_texture;
	SDL_Texture* background_planet_orbit_texture;
};

read_file_result read_file(std::string path);
SDL_Color get_sdl_color(v4 color);
SDL_Rect get_sdl_rect(rect rect);
void print_sdl_error();
void print_sdl_image_error();

void render_group_to_output(render_group* render_group);