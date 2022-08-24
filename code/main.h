#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <stdio.h>
#include <string>
#include "jorutils.h"
#include "jormath.h"
#include "jorstring.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

struct sdl_game_data
{
	bool initialized;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* tileset_texture;
	TTF_Font* font;
};

void print_sdl_error();
void print_sdl_image_error();
void print_sdl_ttf_error();

struct read_file_result
{
	void* contents;
	int size;
};

struct key_press
{
	int number_of_presses;
};

struct game_input
{
	key_press up;
	key_press down;
	key_press left;
	key_press right;
};

struct level
{
	//string_ref tilemap_source;
	u32 width;
	u32 height;
	i32* tiles;
	u32 tiles_count;
};