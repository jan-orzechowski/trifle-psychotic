#pragma once

#include "jorstring.h"
#include "map_types.h"
#include "graphics_types.h"

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
	ENEMY_BIG_SENTRY,
	ENEMY_GUARDIAN,
	ENEMY_FLYING_BOMB,
	ENEMY_ROBOT,
	ENEMY_MESSENGER,
	GATE_SILVER,
	GATE_GOLD,
	GATE_RED,
	GATE_GREEN,
	SWITCH_SILVER,
	SWITCH_GOLD,
	SWITCH_RED,
	SWITCH_GREEN,
	POWER_UP_INVINCIBILITY,
	POWER_UP_HEALTH,
	POWER_UP_SPEED,
	POWER_UP_DAMAGE,
	POWER_UP_SPREAD,
	MOVING_PLATFORM_HORIZONTAL_SILVER,
	MOVING_PLATFORM_HORIZONTAL_GOLD,
	MOVING_PLATFORM_HORIZONTAL_RED,
	MOVING_PLATFORM_HORIZONTAL_GREEN,
	MOVING_PLATFORM_VERTICAL_SILVER,
	MOVING_PLATFORM_VERTICAL_GOLD,
	MOVING_PLATFORM_VERTICAL_RED,
	MOVING_PLATFORM_VERTICAL_GREEN,
	_LAST
};

enum class entity_flags
{
	PLAYER = (1 << 0),
	BLOCKS_MOVEMENT = (1 << 1),
	ENEMY = (1 << 2),
	INDESTRUCTIBLE = (1 << 3),
	DAMAGES_PLAYER = (1 << 4),
	WALKS_HORIZONTALLY = (1 << 5),
	GATE = (1 << 6),
	SWITCH = (1 << 7),
	TINTED_DISPLAY = (1 << 8),
	POWER_UP = (1 << 9),
	MESSAGE_DISPLAY = (1 << 10),
	PLAYER_RECOIL_ON_CONTACT = (1 << 11),
	DESTRUCTION_ON_PLAYER_CONTACT = (1 << 12),
	FLIES_HORIZONTALLY = (1 << 13),
	FLIES_VERTICALLY = (1 << 14),
	FLIES_TOWARDS_PLAYER = (1 << 15),
	MOVING_PLATFORM_VERTICAL = (1 << 16),
	MOVING_PLATFORM_HORIZONTAL = (1 << 17)
};

enum class detection_type
{
	DETECT_NOTHING = 0,
	DETECT_180_DEGREES_IN_FRONT,
	DETECT_90_DEGREES_IN_FRONT,
	DETECT_360_DEGREES,
	DETECT_180_DEGREES_BELOW
};

struct entity_to_spawn
{
	tile_position position;
	entity_type_enum type;
	v4 color;
	string_ref message;
	entity_to_spawn* next;
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
	b32 has_path;
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
