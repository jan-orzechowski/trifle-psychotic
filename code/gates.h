#pragma once

void open_gates_with_given_color(level_state* game, v4 color);
void add_gate_entity(level_state* game, memory_arena* arena, entity_to_spawn* new_entity_to_spawn, b32 is_switch);