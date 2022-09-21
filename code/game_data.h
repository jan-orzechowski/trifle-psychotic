#pragma once

#include "main.h"

level load_level(string_ref map_name, memory_arena* arena, memory_arena* transient_arena);
void initialize_level_state(level_state* state, static_game_data* static_data, input_buffer* input_buffer, string_ref level_name, memory_arena* arena);
void load_static_game_data(static_game_data* data, memory_arena* arena, memory_arena* transient_arena);
entity_type* get_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type);