#pragma once

#include "main.h"
#include "rendering.h"
#include "map.h"

void update_backdrops_movement(backdrop_properties* backdrop, v2* backdrop_offset, r32 delta_time, v2 player_velocity);
void render_backdrops(render_group* render, level_state* level, world_position camera_position);
