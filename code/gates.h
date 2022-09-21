#pragma once

void open_gates_with_given_color(level_state* level, v4 color);
void add_gate_entity(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn, b32 is_switch);