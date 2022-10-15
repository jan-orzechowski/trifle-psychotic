#pragma once

#include "main.h"

#if TRIFLE_DEBUG

void debug_render_tile(render_group* render, tile_position tile_pos, v4 color, world_position camera_pos);
void debug_render_player_information(game_state* game, level_state* level);
void debug_render_tile_collision_boxes(render_group* render, level_state* level, world_position camera_pos);
void debug_render_entity_collision_boxes(render_group* render, level_state* level, world_position camera_pos);
void debug_render_bullet_collision_boxes(render_group* render, level_state* level, world_position camera_pos);

#endif