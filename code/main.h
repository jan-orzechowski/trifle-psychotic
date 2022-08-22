#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <stdio.h>
#include <string>

#include "jormath.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

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