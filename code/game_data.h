#pragma once

#include "main.h"

void load_game_data(sdl_game_data* sdl_game, game_data* game, memory_arena* arena, memory_arena* transient_arena);
entity_type* get_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type);