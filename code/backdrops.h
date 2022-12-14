#pragma once

#include "main.h"
#include "rendering.h"
#include "map.h"

void update_backdrops_movement(backdrop_properties* backdrop, v2* backdrop_offset, entity* player, r32 delta_time);
void render_backdrops(render_list* render, level_state* level, world_position camera_position);
