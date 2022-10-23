#pragma once

#include "main.h"

void initialize_level_state(level_state* state, static_game_data* static_data, string_ref level_name, memory_arena* arena);
void load_static_game_data(static_game_data* data, memory_arena* arena, memory_arena* transient_arena);

void save_completed_levels(static_game_data* data, memory_arena* transient_arena);
void mark_level_as_completed(static_game_data* data, string_ref name);
void load_completed_levels(static_game_data* data);

entity_type* get_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type);