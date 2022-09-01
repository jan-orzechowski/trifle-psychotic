#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <stdio.h>
#include <string>
#include "jorutils.h"
#include "jormath.h"
#include "jorstring.h"

#define TILESET_WIDTH 64
#define TILE_SIDE_IN_PIXELS 16

#define SCALING_FACTOR 2

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SCREEN_WIDTH_IN_TILES (SCREEN_WIDTH / (TILE_SIDE_IN_PIXELS * SCALING_FACTOR))
#define SCREEN_HEIGHT_IN_TILES (SCREEN_HEIGHT / (TILE_SIDE_IN_PIXELS * SCALING_FACTOR))

#define HALF_SCREEN_WIDTH_IN_TILES (SCREEN_WIDTH_IN_TILES / 2)
#define HALF_SCREEN_HEIGHT_IN_TILES (SCREEN_HEIGHT_IN_TILES / 2)

struct sdl_game_data
{
	bool initialized;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* tileset_texture;
	SDL_Texture* bullets_texture;
	SDL_Texture* player_texture;
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
	key_press fire;
};

struct level
{
	//string_ref tilemap_source;
	u32 width;
	u32 height;
	i32* tiles;
	u32 tiles_count;
};

enum entity_flags
{
	PLAYER =         (1 << 0),
	COLLIDES =       (1 << 1),
	ENEMY =		     (1 << 2),
	TAKES_DAMAGE =   (1 << 3),
	DAMAGES_PLAYER = (1 << 4)
};

struct entity_type
{
	v2 collision_rect_dim;
	v2 collision_rect_offset;

	r32 constant_velocity;
	r32 velocity_multiplier;
	r32 slowdown_multiplier;

	i32 damage;
	i32 max_health;
	r32 default_attack_cooldown;
	
	SDL_Rect graphics;
	entity_flags flags;

	entity_type* fired_bullet_type;
};

struct entity
{
	v2 position;
	v2 velocity;
	v2 acceleration;
	i32 health;
	entity_type* type;
	r32 attack_cooldown;
};

struct bullet
{
	v2 position;
	v2 velocity;
	entity_type* type;
};

struct input_buffer
{
	game_input* buffer;
	u32 size;
	u32 current_index;
};

enum movement_mode
{
	JUMP,
	WALK,
	FALL
};

struct player_movement
{
	movement_mode current_mode;
	u32 frame_duration; // 0 oznacza bieżącą klatkę
	movement_mode previous_mode;
	u32 previous_mode_frame_duration;
};

struct game_data
{	
	input_buffer input;
	player_movement player_movement;

	level current_level;
	level collision_reference;

	entity_type* entity_types;
	u32 entity_types_count;
	entity* entities; // compact array
	u32 entities_max_count;
	u32 entities_count;

	entity_type* bullet_types;
	u32 bullet_types_count;
	bullet* bullets; // compact array
	u32 bullets_max_count;
	u32 bullets_count;
};

struct tile_position
{
	u32 x;
	u32 y;
};
