#pragma once

#include "jorstring.h"
#include "map_types.h"
#include "graphics_types.h"

// important: _LAST value must be actually last, and UNKNOWN - first! 
// it allows for iterating through the enum in a for loop
typedef enum entity_type_enum
{
    ENTITY_TYPE_UNKNOWN,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_ENEMY_SENTRY,
    ENTITY_TYPE_ENEMY_BIG_SENTRY,
    ENTITY_TYPE_ENEMY_GUARDIAN,
    ENTITY_TYPE_ENEMY_FLYING_BOMB,
    ENTITY_TYPE_ENEMY_ROBOT,
    ENTITY_TYPE_ENEMY_MESSENGER,
    ENTITY_TYPE_ENEMY_YELLOW_FATHER,
    ENTITY_TYPE_GATE_SILVER,
    ENTITY_TYPE_GATE_GOLD,
    ENTITY_TYPE_GATE_RED,
    ENTITY_TYPE_GATE_GREEN,
    ENTITY_TYPE_SWITCH_SILVER,
    ENTITY_TYPE_SWITCH_GOLD,
    ENTITY_TYPE_SWITCH_RED,
    ENTITY_TYPE_SWITCH_GREEN,
    ENTITY_TYPE_POWER_UP_INVINCIBILITY,
    ENTITY_TYPE_POWER_UP_HEALTH,
    ENTITY_TYPE_POWER_UP_SPEED,
    ENTITY_TYPE_POWER_UP_DAMAGE,
    ENTITY_TYPE_POWER_UP_SPREAD,
    ENTITY_TYPE_MOVING_PLATFORM_HORIZONTAL_SILVER,
    ENTITY_TYPE_MOVING_PLATFORM_HORIZONTAL_GOLD,
    ENTITY_TYPE_MOVING_PLATFORM_HORIZONTAL_RED,
    ENTITY_TYPE_MOVING_PLATFORM_HORIZONTAL_GREEN,
    ENTITY_TYPE_MOVING_PLATFORM_VERTICAL_SILVER,
    ENTITY_TYPE_MOVING_PLATFORM_VERTICAL_GOLD,
    ENTITY_TYPE_MOVING_PLATFORM_VERTICAL_RED,
    ENTITY_TYPE_MOVING_PLATFORM_VERTICAL_GREEN,
    ENTITY_TYPE_NEXT_LEVEL_TRANSITION,
    ENTITY_TYPE_MESSAGE_DISPLAY,
    ENTITY_TYPE_CHECKPOINT,
    _LAST_ENTITY_TYPE
} entity_type_enum;

typedef enum entity_flags
{
    ENTITY_FLAG_BLOCKS_MOVEMENT =               (1 << 0),
    ENTITY_FLAG_PLAYER =                        (1 << 1),
    ENTITY_FLAG_ENEMY =                         (1 << 2),
    ENTITY_FLAG_GATE =                          (1 << 3),
    ENTITY_FLAG_SWITCH =                        (1 << 4),
    ENTITY_FLAG_MOVING_PLATFORM_VERTICAL =      (1 << 5),
    ENTITY_FLAG_MOVING_PLATFORM_HORIZONTAL =    (1 << 6),
    ENTITY_FLAG_INDESTRUCTIBLE =                (1 << 7),
    ENTITY_FLAG_DAMAGES_PLAYER =                (1 << 8),
    ENTITY_FLAG_PLAYER_RECOIL_ON_CONTACT =      (1 << 9),
    ENTITY_FLAG_DESTRUCTION_ON_PLAYER_CONTACT = (1 << 10),
    ENTITY_FLAG_WALKS_HORIZONTALLY =            (1 << 11),
    ENTITY_FLAG_FLIES_HORIZONTALLY =            (1 << 12),
    ENTITY_FLAG_FLIES_VERTICALLY =              (1 << 13),
    ENTITY_FLAG_FLIES_TOWARDS_PLAYER =          (1 << 14),
    ENTITY_FLAG_POWER_UP =                      (1 << 15),
    ENTITY_FLAG_TINTED_DISPLAY =                (1 << 16)
} entity_flags;

typedef enum detection_type
{
    DETECT_NOTHING = 0,
    DETECT_180_DEGREES_IN_FRONT,
    DETECT_90_DEGREES_IN_FRONT,
    DETECT_360_DEGREES,
    DETECT_180_DEGREES_BELOW,
    DETECT_360_DEGREES_IGNORE_WALLS
} detection_type;

typedef struct entity_to_spawn entity_to_spawn;
struct entity_to_spawn
{
    tile_position position;
    entity_type_enum type;
    v4 color;
    string_ref message;
    entity_to_spawn* next;
};

typedef struct entity_type entity_type;
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

    v4 color; // used for switches and gates
    string_ref message;
};

typedef struct entity_type_dictionary
{
    entity_type** type_ptrs;
    u32 type_ptrs_count;
} entity_type_dictionary;

typedef struct entity
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
} entity;

typedef struct explosion
{
    world_position position;
    animation* animation;
    r32 animation_duration;
    sprite_effect* visual_effect;
} explosion;

typedef struct bullet
{
    world_position position;
    v2 velocity;
    entity_type* type;
} bullet;

typedef struct gate_dictionary_entry gate_dictionary_entry;
struct gate_dictionary_entry
{
    entity* entity;
    gate_dictionary_entry* next;
};

typedef struct gate_dictionary
{
    gate_dictionary_entry* entries;
    u32 entries_count;
} gate_dictionary;
