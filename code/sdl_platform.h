#pragma once

#include "main.h"
#include <SDL.h>
#include <SDL_image.h>

struct sdl_data
{
	bool initialized;

	u32 debug_frame_counter;
	r32 debug_elapsed_work_ms;

	SDL_Window* window;
	SDL_Renderer* renderer;

	SDL_Texture* tileset_texture;
	SDL_Texture* font_texture;
	SDL_Texture* charset_texture;
	SDL_Texture* explosion_texture;
	SDL_Texture* backdrop_texture;
};

read_file_result read_file(std::string path);
SDL_Color get_sdl_color(v4 color);
SDL_Rect get_sdl_rect(rect rect);
void print_sdl_error();
void print_sdl_image_error();

void render_group_to_output(render_group* render_group);