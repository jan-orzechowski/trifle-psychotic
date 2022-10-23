#pragma once

#include "main.h"

void save_checkpoint(game_state* game);
void restore_checkpoint(game_state* game);
void save_completed_levels(static_game_data* data, memory_arena* transient_arena);
void mark_level_as_completed(static_game_data* data, string_ref name);
void load_completed_levels(static_game_data* data);