#pragma once

#include "main.h"

level load_level(const char* map_name, memory_arena* arena, memory_arena* transient_arena);
void initialize_game_data(game_data* game, static_game_data* static_data, memory_arena* arena);
save save_game_state(game_data* game);
void restore_game_state(game_data* game, save save);
void load_static_game_data(sdl_game_data* sdl_game, static_game_data* game, memory_arena* arena, memory_arena* transient_arena);
entity_type* get_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type);