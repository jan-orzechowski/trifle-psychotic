#pragma once

#include "main.h"

entity_collision_data get_entity_collision_data(chunk_position reference_chunk, entity* entity);
b32 is_standing_on_ground(level_state* level, entity* entity_to_check);
b32 check_if_sight_line_is_obstructed(level_state* level, world_position start, world_position end);
collision check_minkowski_collision(entity_collision_data a, entity_collision_data b, v2 movement_delta, r32 min_movement_perc);
collision_result move(level_state* level, entity* moving_entity, world_position target_pos);
b32 move_bullet(level_state* level, bullet* moving_bullet, u32 bullet_index, world_position target_pos);