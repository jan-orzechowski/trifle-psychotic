#pragma once

void open_gates_with_given_color(level_state* level, v4 color);
void add_gate_entity(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn, b32 is_switch);

void add_moving_platform_entity(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn);
b32 is_entity_moving_type_entity(entity* entity);

void add_next_level_transition_entity(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn);
void add_message_display_entity(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn);
void add_checkpoint_entity(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn);