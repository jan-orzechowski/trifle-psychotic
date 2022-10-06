#pragma once

#include <stdio.h>
#include <string>
#include "jorutils.h"
#include "jormath.h"
#include "jorstring.h"

#define TILESET_WIDTH 47
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

#define MAX_LEVEL_NAME_LENGTH 100

enum class textures
{
	NONE,
	TILESET,
	FONT,
	CHARSET,
	EXPLOSION
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
	i32 mouse_x;
	i32 mouse_y;
	key_press up;
	key_press down;
	key_press left;
	key_press right;
	key_press fire;
};

enum class direction
{
	NONE = 0,
	E = 1,
	SE = 2,
	S = 3,
	SW = 4,
	W = 5,
	NW = 6,
	N = 7,
	NE = 8
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
	NEXT_LEVEL_TRANSITION,
	MESSAGE_DISPLAY,
	ENEMY_SENTRY,
	ENEMY_GUARDIAN,
	ENEMY_FLYING_BOMB,
	ENEMY_ROBOT,
	ENEMY_CULTIST,
	GATE_BLUE,
	GATE_GREY,
	GATE_RED,
	GATE_GREEN,
	SWITCH_BLUE,
	SWITCH_GREY,
	SWITCH_RED,
	SWITCH_GREEN,
	POWER_UP_INVINCIBILITY,
	POWER_UP_HEALTH,
	POWER_UP_SPEED,
	POWER_UP_DAMAGE,
	POWER_UP_SPREAD,
	MOVING_PLATFORM_HORIZONTAL_BLUE,
	MOVING_PLATFORM_HORIZONTAL_GREY,
	MOVING_PLATFORM_HORIZONTAL_RED,
	MOVING_PLATFORM_HORIZONTAL_GREEN,
	MOVING_PLATFORM_VERTICAL_BLUE,
	MOVING_PLATFORM_VERTICAL_GREY,
	MOVING_PLATFORM_VERTICAL_RED,
	MOVING_PLATFORM_VERTICAL_GREEN,
	_LAST
};

enum class detection_type
{
	DETECT_NOTHING = 0,
	DETECT_180_DEGREES_IN_FRONT,
	DETECT_90_DEGREES_IN_FRONT,
	DETECT_360_DEGREES,
	DETECT_180_DEGREES_BELOW
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
	POWER_UP               = (1 << 9),
	MESSAGE_DISPLAY        = (1 << 10),
	PLAYER_RECOIL_ON_CONTACT = (1 << 11),
	DESTRUCTION_ON_PLAYER_CONTACT = (1 << 12),
	FLIES_HORIZONTALLY = (1 << 13),
	FLIES_VERTICALLY = (1 << 14),
	FLIES_TOWARDS_PLAYER = (1 << 15),
	MOVING_PLATFORM_VERTICAL = (1 << 16),
	MOVING_PLATFORM_HORIZONTAL = (1 << 17)
};

struct entity_to_spawn
{
	tile_position position;
	entity_type_enum type;
	v4 color;
	string_ref message;
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

// polegamy na tym, by _FIRST było pierwsze, a _LAST ostatnie, dla policzenia ile potrzebujemy miejsc w słowniku
enum class sprite_effects_types
{
	_FIRST,
	OTHER,
	DEATH,
	SHOCK,
	RECOIL,
	BULLET_HIT,
	GATE_DISPLAY_ACTIVE,
	GATE_DISPLAY_INACTIVE,
	INVINCIBILITY,
	_LAST 
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
	sprite_effects_types type;
	v4 color;
	sprite_effect_stage* stages;
	u32 stages_count;
	r32 total_duration;
	sprite_effect_flags flags;
};

struct sprite_part
{
	textures texture;
	rect texture_rect;
	v2 offset_in_pixels;
	direction default_direction;
};

struct sprite
{
	sprite_part* parts;
	u32 parts_count;
	b32 flip_horizontally;
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

struct power_up_state
{
	r32 time_remaining;
};

union power_ups 
{
	struct
	{
		power_up_state invincibility;
		power_up_state speed;
		power_up_state damage;
		power_up_state spread;
	};	 
	power_up_state states[4];
};

struct shooting_rotation_sprites
{
	sprite up;
	v2 up_bullet_offset;
	sprite right_up;
	v2 right_up_bullet_offset;
	sprite right;
	v2 right_bullet_offset;
	sprite right_down;
	v2 right_down_bullet_offset;
	sprite down;
	v2 down_bullet_offset;
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

	detection_type detection_type;
	r32 detection_distance;
	r32 stop_movement_distance;
	r32 forget_detection_distance;
	v2 looking_position_offset;

	r32 damage_on_contact;
	r32 max_health;
	r32 default_attack_cooldown;
	r32 default_attack_series_duration;
	r32 default_attack_bullet_interval_duration;

	v2 fired_bullet_offset;
	animation_frame idle_pose;
	entity_flags flags;

	shooting_rotation_sprites* rotation_sprites;
	animation* walk_animation;
	
	v2 death_animation_offset;
	animation** death_animation_variants;
	u32 death_animation_variants_count;

	entity_type* fired_bullet_type;

	v4 color; // używane w przypadku bram i przełączników
	string_ref message;
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
	b32 used;
	entity_type* type;
	
	world_position position;
	v2 velocity;
	v2 acceleration;
	
	r32 health;
	r32 attack_cooldown;
	r32 attack_series_duration;
	r32 attack_bullet_interval_duration;
	
	sprite_effect* visual_effect;
	r32 visual_effect_duration;
	direction direction;

	animation* current_animation;
	u32 current_frame_index;
	r32 animation_duration;
	sprite shooting_sprite;

	b32 player_detected;
	b32 has_walking_path;
	tile_range path;
	u32 goal_path_point;
};

struct explosion
{
	world_position position;
	animation* animation;
	r32 animation_duration;
	sprite_effect* visual_effect;
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
	entity* collided_message_display;
	entity* collided_platform;
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

struct standing_history
{
	b32* buffer;
	u32 current_index;
	u32 buffer_size;
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

	standing_history standing_history;
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

struct gate_graphics
{
	sprite_part gate;
	sprite_part frame_upper;
	sprite_part frame_lower;
};

struct moving_platform_graphics
{
	sprite_part left;
	sprite_part middle;
	sprite_part right;
};

struct display_graphics
{
	sprite_part gate_upper_display;
	sprite_part gate_lower_display;
	sprite_part switch_left_display;
	sprite_part switch_middle_display;
	sprite_part switch_right_display;
};

struct switch_graphics
{
	sprite_part frame_left;
	sprite_part frame_middle;
	sprite_part frame_right;
};

struct ui_graphics
{
	rect healthbar_icon;
	rect healthbar_empty_bar;
	rect healthbar_red_bar;
	rect healthbar_white_bar;

	rect msgbox_frame_upper_left; 
	rect msgbox_frame_upper;
	rect msgbox_frame_upper_right;
	rect msgbox_frame_right;
	rect msgbox_frame_lower_right;
	rect msgbox_frame_lower;
	rect msgbox_frame_lower_left;
	rect msgbox_frame_left;
	rect msgbox_frame_background;

	rect msgbox_dots_1;
	rect msgbox_dots_2;
	rect msgbox_dots_3;

	rect crosshair;
	rect menu_indicator;	
};

struct gates_graphics
{
	gate_graphics blue;
	gate_graphics grey;
	gate_graphics red;
	gate_graphics green;
};

struct switches_graphics
{
	switch_graphics blue;
	switch_graphics grey;
	switch_graphics red;
	switch_graphics green;	
};

struct moving_platforms_graphics
{
	moving_platform_graphics blue;
	moving_platform_graphics grey;
	moving_platform_graphics red;
	moving_platform_graphics green;
};

struct font
{
	u32 width_in_pixels;
	u32 height_in_pixels;
	i32 letter_spacing;
	i32 line_spacing;
	textures texture;
};

struct explosions
{
	animation* size_16x16_variant_1;
	animation* size_16x16_variant_2;
	animation* size_16x16_variant_3;
	animation* size_24x24;
	animation* size_32x32;
	animation* size_48x48;
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

	entity_type* player_normal_bullet_type;
	entity_type* player_power_up_bullet_type;

	font ui_font;
	font menu_font;
	ui_graphics ui_gfx;
	gates_graphics gates_gfx;
	switches_graphics switches_gfx;
	display_graphics display_gfx;	
	moving_platforms_graphics platforms_gfx;

	string_ref menu_new_game_str;
	string_ref menu_continue_str;
	string_ref menu_credits_str;
	string_ref menu_exit_str;

	explosions explosion_animations;
};

enum class scene
{
	GAME,
	MAIN_MENU,
	DEATH,
	CREDITS,
	MAP_ERRORS,
	EXIT
};

struct scene_change
{
	b32 change_scene;
	scene new_scene;
	b32 restore_checkpoint;
	string_ref map_to_load;
	r32 fade_out_speed;
};

struct level_state
{
	string_ref current_map_name;
	b32 current_map_initialized;
	map current_map;

	player_movement player_movement;
	r32 player_invincibility_cooldown;
	
	entity* entities;
	u32 entities_max_count;
	u32 entities_count;

	bullet* bullets; // compact array
	u32 bullets_max_count;
	u32 bullets_count;

	entity* explosions; // compact array
	u32 explosions_max_count;
	u32 explosions_count;

	gate_dictionary gates_dict;
	sprite_effect_dictionary gate_tints_dict;
	entity_type* moving_platform_types[8];

	static_game_data* static_data;

	power_ups power_ups;

	r32 screen_shake_duration;
	r32 screen_shake_multiplier;

	b32 show_message;
	string_ref message_to_show;
	r32 min_message_timer;
	b32 stop_player_movement;

	scene_change active_scene_change;
	r32 fade_out_perc;
	r32 fade_in_perc;
};

struct save
{
	b32 used;
	string_ref map_name;
	u32 player_max_health;
};

struct render_group
{
	u32 max_push_buffer_size;
	u32 push_buffer_size;
	u8* push_buffer_base;
};

struct main_menu_state
{
	i32 option_index;
	i32 option_max_index;
	r32 option_change_timer;
	r32 fade_percentage;
};

struct death_screen_state
{
	b32 initialized;
	string_ref prompt;
	r32 timer;
	b32 transition_to_game;
	r32 fade_out_perc;
	r32 fade_in_perc;
};

struct game_state
{
	b32 initialized;
	scene current_scene;
	
	b32 level_initialized;
	level_state* level_state;
	memory_arena* arena;
	memory_arena* transient_arena;	
	
	save checkpoint;
	char* level_name_buffer;
	main_menu_state main_menu;
	death_screen_state death_screen;

	i32 message_dots_index;
	r32 message_dots_timer;

	input_buffer input_buffer;

	string_ref map_errors;
	temporary_memory game_level_memory;	

	render_group render;
};

void main_game_loop(game_state* game, static_game_data* static_data, r32 delta_time);