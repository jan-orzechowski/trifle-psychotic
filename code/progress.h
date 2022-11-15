#pragma once

#include "main.h"

void save_checkpoint(level_state* state, checkpoint* check);
void restore_checkpoint(level_state* state, checkpoint* check);
void save_completed_levels(platform_api* platform, static_game_data* data, memory_arena* transient_arena);
void mark_level_as_completed(static_game_data* data, string_ref name);
void load_completed_levels(platform_api* platform, static_game_data* data);