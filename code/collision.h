#pragma once

#include "main.h"
#include "map_types.h"

typedef struct collision
{
    direction collided_wall;
    v2 collided_wall_normal;
    r32 possible_movement_perc;
} collision;

typedef struct collision_result
{
    collision collision_data;
    entity* collided_enemy;
    entity* collided_switch;
    entity* collided_transition;
    entity* collided_power_up;
    entity* collided_message_display;
    entity* collided_checkpoint;
    entity* collided_platform;
} collision_result;

typedef struct entity_collision_data
{
    v2 position;
    v2 collision_rect_offset;
    v2 collision_rect_dim;
} entity_collision_data;

entity_collision_data get_tile_collision_data(chunk_position reference_chunk, tile_position tile_pos);
entity_collision_data get_entity_collision_data(chunk_position reference_chunk, entity* entity);
v2 get_collision_dim_from_tile_range(tile_range path);
b32 is_standing_on_ground(level_state* level, entity* entity_to_check, collision_result* collision_to_fill);
b32 check_if_sight_line_is_obstructed(level_state* level, world_position start, world_position end);
collision check_minkowski_collision(entity_collision_data a, entity_collision_data b, v2 movement_delta, r32 min_movement_perc);
tile_range find_path_fragment_not_blocked_by_entities(level_state* level, tile_range path);
collision_result move(level_state* level, entity* moving_entity, world_position target_pos);
b32 move_bullet(level_state* level, bullet* moving_bullet, world_position target_pos);
void move_entity_towards_player(level_state* level, entity* entity_to_move, entity* player, r32 delta_time);