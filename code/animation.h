#pragma once

#include "jorutils.h"
#include "jormath.h"
#include "main.h"

b32 are_flags_set(sprite_effect_flags* flags, sprite_effect_flags flag_values_to_check);
void set_flags(sprite_effect_flags* flags, sprite_effect_flags flag_values_to_check);
void unset_flags(sprite_effect_flags* flags, sprite_effect_flags flag_values_to_check);
r32 get_stage_tint(sprite_effect_stage* stage, r32 total_time);
v4 get_tint(sprite_effect* effect, r32 time);
void start_visual_effect(entity* entity, sprite_effect* effect, b32 override_current);
void start_visual_effect(level_state* game, entity* entity, u32 sprite_effect_index, b32 override_current);
void stop_visual_effect(entity* entity, sprite_effect* effect_to_stop);
u32 get_current_animation_frame_index(animation* animation, r32 elapsed_time);
animation_frame* get_current_animation_frame(entity* entity);
void animate_entity(player_movement* movement, entity* entity, r32 delta_time);
void render_entity_animation_frame(render_group* render, world_position camera_position, entity* entity);