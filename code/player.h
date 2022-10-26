#pragma once

#include "main.h"
#include "animation.h"
#include "entities.h"
#include "input.h"

entity* get_player(level_state* level);
b32 ignore_player_and_enemy_collisions(level_state* level);
void damage_player(level_state* level, r32 damage_amount, b32 ignore_after_damage_invincibility);
void update_power_up_timers(level_state* level, r32 delta_time);
b32 is_power_up_active(power_up_state power_up);
void apply_power_up(level_state* level, entity* player, entity* power_up);
void change_player_movement_mode(player_movement* movement, player_movement_mode new_mode);
world_position process_input(level_state* level, input_buffer* input_buffer, entity* player, r32 delta_time);
void handle_player_and_enemy_collision(level_state* level, entity* player, entity* enemy);
void handle_player_and_switch_collision(level_state* level, collision_result collision);