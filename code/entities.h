﻿#pragma once

#include "main.h"
#include "map.h"
#include "gates.h"
#include "collision.h"

struct shooting_sprite_result
{
	v2 bullet_offset;
	sprite rotated_sprite;
	b32 flip_horizontally;
};

b32 are_flags_set(entity_flags* flags, entity_flags flag_values_to_check);
void set_flags(entity_flags* flags, entity_flags flag_values_to_check);
void unset_flags(entity_flags* flags, entity_flags flag_values_to_check);
b32 are_entity_flags_set(entity* entity, entity_flags flag_values);

entity* add_entity(level_state* level, world_position position, entity_type* type);
entity* add_entity(level_state* level, tile_position position, entity_type* type);
void remove_entity(level_state* level, i32* entity_index);

void fire_bullet(level_state* level, entity_type* bullet_type, world_position bullet_starting_position, v2 bullet_offset, v2 velocity);
void fire_bullet(level_state* level, entity* entity, b32 cooldown);
void remove_bullet(level_state* level, i32* bullet_index);

entity* add_explosion(level_state* level, world_position position, animation* explosion_animation);
void remove_explosion(level_state* level, i32* explosion_index);

entity_type_dictionary create_entity_types_dictionary(memory_arena* arena);
void set_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type, entity_type* enity_type_ptr);
entity_type* get_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type);

tile_range find_walking_path_for_enemy(map level, map collision_ref, tile_position start_tile);

b32 is_point_visible_from_point(world_position looking_point, direction looking_direction, r32 max_looking_distance, world_position point_to_check);
b32 is_point_visible_for_entity(level_state* level, entity* looking_entity, world_position point, r32 max_distance);
//void add_next_level_transition(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn);
//void add_message_display_entity(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn);
void initialize_current_map(game_state* game, level_state* level);

shooting_sprite_result get_shooting_sprite_based_on_direction(shooting_rotation_sprites* rotation_sprites, v2 shooting_direction);
void enemy_fire_bullet(level_state* level, entity* enemy, entity* target, v2 target_offset);