#pragma once

#include "jorutils.h"
#include "jormath.h"
#include "main.h"

b32 are_flags_set(sprite_effect_flags* flags, sprite_effect_flags flag_values_to_check);
void set_flags(sprite_effect_flags* flags, sprite_effect_flags flag_values_to_check);
void unset_flags(sprite_effect_flags* flags, sprite_effect_flags flag_values_to_check);

r32 get_stage_tint(sprite_effect_stage* stage, r32 total_time);
v4 get_tint(sprite_effect* effect, r32 time);

void start_visual_effect(entity* entity, sprite_effect* effect);
void start_visual_effect(level_state* level, entity* entity, sprite_effects_types type);
void stop_visual_effect(entity* entity, sprite_effect* effect_to_stop);
void stop_visual_effect(level_state* level, entity* entity, sprite_effects_types type);

u32 get_current_animation_frame_index(animation* animation, r32 elapsed_time, r32 frame_duration_modifier = 1.0f);
animation_frame* get_current_animation_frame(entity* entity);
void animate_entity(player_movement* movement, entity* entity, r32 delta_time, r32 frame_duration_modifier = 1.0f);
void render_entity_animation_frame(render_list* render, world_position camera_position, entity* entity);
void start_screen_shake(level_state* level, r32 duration_in_seconds, r32 strength);
void process_fade(render_list* render, r32* percentage, r32 delta_time, b32 fade_in, r32 delta_time_multiplier = 1.5f);
void start_death_animation(level_state* level, entity* entity);
void start_death_animation(level_state* level, bullet* bullet);