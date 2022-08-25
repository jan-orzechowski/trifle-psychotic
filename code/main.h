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

#define TILESET_WIDTH 64
#define TILE_SIDE_IN_PIXELS 16

struct sdl_game_data
{
	bool initialized;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* tileset_texture;
	TTF_Font* font;

	u32 debug_frame_counter;
	r32 debug_elapsed_work_ms;
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

struct entity_type
{
	v2 collision_rect_dim;
	SDL_Rect graphics;
	b32 collides;
};

struct entity
{
	v2 position;
	entity_type* type;
};

struct game_data
{
	v2 player_pos;
	r32 player_speed;
	v2 player_collision_rect_dim;

	level current_level;
	level collision_reference;

	entity_type* entity_types;
	u32 entity_types_count;

	entity* entities;
	u32 entities_max_count;
	u32 entities_count;
};

struct tile_position
{
	u32 x;
	u32 y;
};