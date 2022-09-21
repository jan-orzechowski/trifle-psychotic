#pragma once

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

#define SCREEN_CENTER_IN_TILES get_v2(HALF_SCREEN_WIDTH_IN_TILES, HALF_SCREEN_HEIGHT_IN_TILES)
#define SCREEN_CENTER_IN_PIXELS get_v2((SCREEN_WIDTH / SCALING_FACTOR) / 2, (SCREEN_HEIGHT / SCALING_FACTOR) / 2)

#define CHUNK_SIDE_IN_TILES 16

// do usunięcia - ostatecznie mamy mieć jedną teksturę
enum class temp_texture_enum
{
	NONE, // to był głównie trik przy bramach - trzeba to czymś zastąpić
	TILESET_TEXTURE,
	BULLETS_TEXTURE,
	UI_TEXTURE,
	FONT_TEXTURE,
	PLAYER_TEXTURE,
	MISC_TEXTURE,
	GATES_TEXTURE
};

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
	b32 is_left_mouse_key_held;
	key_press up;
	key_press down;
	key_press left;
	key_press right;
	key_press fire;
};

enum class direction
{
	NONE = 0,
	N = (1 << 0),
	E = (1 << 1),
	W = (1 << 3),
	S = (1 << 4)
};

struct tile_range
{
	tile_position start;
	tile_position end;
};

// istotne - _LAST musi być ostatnie, a UNKNOWN pierwsze! 
// to pozwala na iterację po wszystkich enumach w pętli for
// w tym enumie nie możemy przypisywać konkretnych wartości liczbowych
enum class entity_type_enum
{
	UNKNOWN,
	PLAYER,
	STATIC_ENEMY,
	MOVING_ENEMY,
	GATE,
	SWITCH,
	POWER_UP_INVINCIBILITY,
	POWER_UP_HEALTH,
	POWER_UP_SPEED,
	POWER_UP_DAMAGE,
	POWER_UP_GRANADES,
	NEXT_LEVEL_TRANSITION,
	_LAST
};

enum class entity_flags
{
	PLAYER   		       = (1 << 0),
	BLOCKS_MOVEMENT        = (1 << 1),
	ENEMY                  = (1 << 2),
	INDESTRUCTIBLE         = (1 << 3),
	DAMAGES_PLAYER         = (1 << 4),
	WALKS_HORIZONTALLY     = (1 << 5),
	GATE			       = (1 << 6),
	SWITCH                 = (1 << 7),
	TINTED_DISPLAY         = (1 << 8),
	POWER_UP               = (1 << 9)
};

struct entity_to_spawn
{
	tile_position position;
	entity_type_enum type;
	v4 color;
};

struct map
{
	u32 width;
	u32 height;
	i32* tiles;
	u32 tiles_count;
	entity_to_spawn* entities_to_spawn;
	u32 entities_to_spawn_count;
	tile_position starting_tile;
	string_ref next_map;
};

struct sprite_effect_stage
{
	r32 amplitude;
	r32 phase_shift;
	r32 vertical_shift;
	r32 period; // jeśli 0, to mamy stałą funkcję
	r32 stage_duration;
};

enum class sprite_effect_flags
{
	REPEATS = (1 << 0),
	ADDITIVE_MODE = (1 << 1),
	REVERSE_VALUES = (1 << 2)
};

struct sprite_effect
{
	v4 color;
	sprite_effect_stage* stages;
	u32 stages_count;
	r32 total_duration;
	sprite_effect_flags flags;
};

struct sprite_part
{
	temp_texture_enum texture;
	rect texture_rect;
	v2 offset_in_pixels;
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
	u32 current_frame_index;
};

struct power_up_state
{
	r32 time_remaining;
};

union power_ups 
{
	struct
	{
		power_up_state invincibility;
		//power_up_state health;
		power_up_state speed;
		power_up_state damage;
		//power_up_state granades;
	};	 
	power_up_state states[3];
};

struct entity_type
{
	entity_type_enum type_enum;

	v2 collision_rect_dim;
	v2 collision_rect_offset;

	r32 constant_velocity;
	r32 player_acceleration_on_collision;
	r32 velocity_multiplier;
	r32 slowdown_multiplier;

	r32 player_detecting_distance;
	r32 damage_on_contact;
	r32 max_health;
	r32 default_attack_cooldown;
	
	v2 fired_bullet_offset;
	animation_frame idle_pose;
	entity_flags flags;

	animation* walk_animation;

	entity_type* fired_bullet_type;

	v4 color; // używane w przypadku bram i przełączników
};

struct entity_type_dictionary
{
	entity_type** type_ptrs;
	u32 type_ptrs_count;
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
	r32 health;
	entity_type* type;
	r32 attack_cooldown;
	sprite_effect* visual_effect;
	r32 visual_effect_duration;
	direction direction;
	animation* current_animation;
	r32 animation_duration;

	b32 has_walking_path;
	tile_range path;
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

struct collision_result
{
	collision collision_data;
	entity* collided_enemy;
	entity* collided_switch;
	entity* collided_transition;
	entity* collided_power_up;
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

enum class movement_mode
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

struct gate_dictionary_entry
{
	entity* entity;
	gate_dictionary_entry* next;
};

struct gate_dictionary
{
	gate_dictionary_entry* entries;
	u32 entries_count;
};

struct sprite_effect_dictionary
{
	sprite_effect** sprite_effects;
	u32 sprite_effects_count;
	u32 probing_jump;
};

struct static_game_data
{
	map collision_reference;

	r32 default_player_invincibility_cooldown;

	entity_type_dictionary entity_types_dict;
	entity_type* entity_types;
	u32 entity_types_count;

	entity_type* bullet_types;
	u32 bullet_types_count;

	sprite_effect* visual_effects;
	u32 visual_effects_count;

	sprite_part gate_sprite;
	sprite_part gate_frame_upper_sprite;
	sprite_part gate_frame_lower_sprite;
	sprite_part gate_display_upper_sprite;
	sprite_part gate_display_lower_sprite;
	sprite_part switch_frame_left_sprite;
	sprite_part switch_frame_middle_sprite;
	sprite_part switch_frame_right_sprite;
	sprite_part switch_display_left_sprite;
	sprite_part switch_display_middle_sprite;
	sprite_part switch_display_right_sprite;

	string_ref menu_new_game_str;
	string_ref menu_continue_str;
	string_ref menu_credits_str;
	string_ref menu_exit_str;
};

struct level_state
{
	string_ref current_map_name;
	b32 current_map_initialized;
	map current_map;
	
	input_buffer input;
	player_movement player_movement;
	r32 player_invincibility_cooldown;
	
	entity* entities; // compact array
	u32 entities_max_count;
	u32 entities_count;

	bullet* bullets; // compact array
	u32 bullets_max_count;
	u32 bullets_count;

	gate_dictionary gates_dict;
	sprite_effect_dictionary gate_tints_dict;

	static_game_data* static_data;

	power_ups power_ups;
};

struct save
{
	string_ref map_name;
	u32 granades_count;
	u32 player_max_health;
};

enum class scene
{
	GAME,
	MAIN_MENU,
	DEATH,
	CREDITS,
	EXIT
};

struct scene_change
{
	b32 change_scene;
	scene new_scene;
	string_ref map_to_load;
};

struct render_piece
{

};

struct render_buffer
{
	render_piece* pieces;
	u32 pieces_count;
};

struct render_group
{
	uint32 piece_count;

	u32 max_push_buffer_size;
	u32 push_buffer_size;
	u8* push_buffer_base;
};

struct game_state
{
	b32 initialized;
	scene current_scene;
	
	b32 first_game_run_initialized;
	level_state* level_state;
	memory_arena* arena;
	memory_arena* transient_arena;	
	
	temporary_memory game_level_memory;	

	render_group render;
};

rect get_tile_rect(u32 tile_id);
tile_position get_tile_position(i32 tile_x, i32 tile_y);

input_buffer initialize_input_buffer(memory_arena* arena);
b32 are_flags_set(entity_flags*	 flags, entity_flags flag_values_to_check);
void set_flags(entity_flags* flags, entity_flags flag_values_to_check);
void unset_flags(entity_flags* flags, entity_flags flag_values_to_check);
b32 are_entity_flags_set(entity* entity, entity_flags flag_values);
entity* add_entity(level_state* level, world_position position, entity_type* type);
entity* add_entity(level_state* level, tile_position position, entity_type* type);

void render_entity_sprite(render_group* render, world_position camera_position, world_position entity_position, direction entity_direction,
	sprite_effect* visual_effect, r32 visual_effect_duration, sprite sprite);

tile_range find_horizontal_range_of_free_tiles(map level, map collision_ref, tile_position starting_tile, u32 length_limit);
tile_range find_vertical_range_of_free_tiles(map level, map collision_ref, tile_position starting_tile, u32 length_limit);

void write_to_input_buffer(input_buffer* buffer, game_input* new_input);
void main_game_loop(game_state* game, static_game_data* static_data, input_buffer* input_buffer, r32 delta_time);

enum class render_group_entry_type
{
	CLEAR,
	FADE,
	BITMAP,
	BITMAP_WITH_EFFECTS,
	DEBUG_RECTANGLE,
};

struct render_group_entry_header
{
	render_group_entry_type type;
};

struct render_group_entry_clear
{
	v4 color;
};

struct render_group_entry_fade
{
	v4 color;
	r32 percentage;
};

struct render_group_entry_bitmap
{
	rect source_rect;
	rect destination_rect;
	temp_texture_enum texture;
};

struct render_group_entry_bitmap_with_effects
{
	rect source_rect;
	rect destination_rect;
	temp_texture_enum texture;

	v4 tint_color;
	b32 render_in_additive_mode;
	b32 flip_horizontally;
};

struct render_group_entry_debug_rectangle
{
	v4 color;
	b32 render_outline_only;
	rect destination_rect;
};

void render_bitmap(render_group* group, temp_texture_enum texture, rect source_rect, rect destination_rect);