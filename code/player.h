#pragma once

#include "main.h"
#include "animation.h"
#include "entities.h"
#include "input.h"

entity* get_player(level_state* level);
b32 damage_player(level_state* level, r32 damage_amount);
void update_power_up_timers(level_state* level, r32 delta_time);
b32 is_power_up_active(power_up_state power_up);
void apply_power_up(level_state* level, entity* player, entity* power_up);
void change_player_movement_mode(player_movement* movement, movement_mode mode);
void player_fire_bullet(level_state* level, game_input* input, world_position player_position, entity_type* bullet_type);
world_position process_input(level_state* level, input_buffer* input_buffer, entity* player, r32 delta_time);
void handle_player_and_enemy_collision(level_state* level, entity* player, entity* enemy);
save* save_player_state(memory_arena* arena, level_state* level);
void restore_player_state(level_state* level, save* save);