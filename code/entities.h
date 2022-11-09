#pragma once

#include "main.h"
#include "map.h"
#include "collision.h"

typedef struct shooting_rotation
{
    v2 bullet_offset;
    sprite rotated_sprite;
    b32 flip_horizontally;
} shooting_rotation;

b32 are_entity_flags_set(entity_flags* flags, entity_flags flag_values_to_check);
void set_entity_flags(entity_flags* flags, entity_flags flag_values_to_check);
void unset_entity_flags(entity_flags* flags, entity_flags flag_values_to_check);
b32 has_entity_flags_set(entity* entity, entity_flags flag_values);

entity* add_entity_at_world_position(level_state* level, world_position position, entity_type* type);
entity* add_entity_at_tile_position(level_state* level, tile_position position, entity_type* type);
void remove_entity(entity* entity_to_remove);

void fire_bullet(level_state* level, entity_type* bullet_type, world_position bullet_starting_position, v2 bullet_offset, v2 velocity);
void remove_bullet(level_state* level, i32* bullet_index);

entity* add_explosion(level_state* level, world_position position, animation* explosion_animation);
void remove_explosion(level_state* level, i32* explosion_index);

entity_type_dictionary create_entity_types_dictionary(memory_arena* arena);
void set_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type, entity_type* enity_type_ptr);
entity_type* get_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type);

void find_walking_path_for_enemy(level_state* level, entity* entity);
void find_flying_path_for_enemy(level_state* level, entity* entity, b32 vertical);

b32 is_point_visible_within_90_degrees(world_position looking_point, direction looking_direction, r32 max_looking_distance, world_position point_to_check);
b32 is_point_visible_for_entity(level_state* level, entity* looking_entity, world_position point);

void enemy_attack(level_state* level, entity* enemy, entity* player, r32 delta_time);
void handle_entity_and_bullet_collision(level_state* level, bullet* moving_bullet, entity* hit_entity);
shooting_rotation get_entity_shooting_rotation(shooting_rotation_sprites* rotation_sprites, v2 shooting_direction);
void set_entity_rotated_graphics(entity* entity, world_position* target);

void enemy_fire_bullet(level_state* level, entity* enemy, entity* target, v2 target_offset);

void process_entity_movement(level_state* level, entity* entity_to_move, entity* player, r32 delta_time);