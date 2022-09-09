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

#define SCREEN_CENTER get_v2(HALF_SCREEN_WIDTH_IN_TILES + 1, HALF_SCREEN_HEIGHT_IN_TILES + 1)

#define CHUNK_SIDE_IN_TILES 16

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

struct tile_position
{
	i32 x;
	i32 y;
};

struct chunk_position
{
	i32 x;
	i32 y;
};

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

enum direction
{
	NONE = 0,
	N = (1 << 0),
	E = (1 << 1),
	W = (1 << 3),
	S = (1 << 4)
};

struct walking_path
{
	tile_position left_end;
	tile_position right_end;
};

struct level
{
	//string_ref tilemap_source;
	u32 width;
	u32 height;
	i32* tiles;
	u32 tiles_count;
};

struct sprite_effect_stage
{
	r32 amplitude;
	r32 phase_shift;
	r32 vertical_shift;
	r32 period; // jeśli 0, to mamy stałą funkcję
	r32 stage_duration;
	b32 ignore_negatives;
};

struct sprite_effect
{
	SDL_Color color;
	sprite_effect_stage* stages;
	u32 stages_count;
	r32 total_duration;
};

struct sprite_part
{
	SDL_Texture* texture;
	SDL_Rect texture_rect;
	v2 offset;
	direction default_direction;
};

struct sprite
{
	sprite_part* parts;
	u32 parts_count;
};

struct animation_frame
{
	sprite sprite;
	r32 duration;
};

struct animation
{
	animation_frame* frames;
	u32 frames_count;
	r32 total_duration;
};

enum entity_flags
{
	PLAYER =			 (1 << 0),
	COLLIDES =			 (1 << 1),
	ENEMY =				 (1 << 2),
	TAKES_DAMAGE =		 (1 << 3),
	DAMAGES_PLAYER =	 (1 << 4),
	WALKS_HORIZONTALLY = (1 << 5)
};

struct entity_type
{
	v2 collision_rect_dim;
	v2 collision_rect_offset;

	r32 constant_velocity;
	r32 player_acceleration_on_collision;
	r32 velocity_multiplier;
	r32 slowdown_multiplier;

	i32 damage_on_contact;
	i32 max_health;
	r32 default_attack_cooldown;
	
	sprite idle_pose;
	entity_flags flags;

	animation* walk_animation;

	entity_type* fired_bullet_type;
};

struct world_position
{
	chunk_position chunk_pos;
	v2 pos_in_chunk;
};

struct entity
{
	world_position position;
	v2 velocity;
	v2 acceleration;
	i32 health;
	entity_type* type;
	r32 attack_cooldown;
	sprite_effect* visual_effect;
	r32 visual_effect_duration;
	direction direction;
	animation* current_animation;
	r32 animation_duration;

	b32 has_walking_path;
	walking_path path;
	u32 goal_path_point;
};

struct bullet
{
	world_position position;
	v2 velocity;
	entity_type* type;
};

struct collision
{
	direction collided_wall;
	v2 collided_wall_normal;
	r32 possible_movement_perc;
};

struct entity_collision_data
{
	v2 position;
	v2 collision_rect_offset;
	v2 collision_rect_dim;
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
	RECOIL
};

struct player_movement
{
	movement_mode current_mode;
	u32 frame_duration; // 0 oznacza bieżącą klatkę
	movement_mode previous_mode;
	u32 previous_mode_frame_duration;

	r32 recoil_timer;
	r32 recoil_acceleration_timer;
	v2 recoil_acceleration;
};

struct game_data
{	
	input_buffer input;
	player_movement player_movement;
	r32 player_invincibility_cooldown;
	r32 default_player_invincibility_cooldown;

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

	sprite_effect* visual_effects;
	u32 visual_effects_count;
};

SDL_Rect get_tile_rect(u32 tile_id);
entity* add_entity(game_data* game, tile_position tile, entity_type* type);