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
	SDL_Texture* bullets_texture;
	SDL_Texture* ui_texture;
	SDL_Texture* font_texture;
	SDL_Texture* player_texture;
	SDL_Texture* misc_texture;
	SDL_Texture* gates_texture;
};

read_file_result read_file(std::string path);
SDL_Color get_sdl_color(v4 color);
SDL_Rect get_sdl_rect(rect rect);
void print_sdl_error();
void print_sdl_image_error();