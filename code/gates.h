#pragma once

void open_gates_with_given_color(game_data* game, v4 color);
void add_gate_entity(game_data* game, memory_arena* arena, entity_to_spawn* new_entity_to_spawn, b32 is_switch);