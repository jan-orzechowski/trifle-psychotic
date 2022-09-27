#include "main.h"
#include "map.h"
#include "gates.h"
#include "collision.h"

b32 are_flags_set(entity_flags* flags, entity_flags flag_values_to_check)
{
	b32 result = are_flags_set((u32*)flags, (u32)flag_values_to_check);
	return result;
}

void set_flags(entity_flags* flags, entity_flags flag_values_to_check)
{
	set_flags((u32*)flags, (u32)flag_values_to_check);
}

void unset_flags(entity_flags* flags, entity_flags flag_values_to_check)
{
	unset_flags((u32*)flags, (u32)flag_values_to_check);
}

b32 are_entity_flags_set(entity* entity, entity_flags flag_values)
{
	b32 result = are_flags_set(&entity->type->flags, flag_values);
	return result;
}

entity* add_entity(level_state* level, world_position position, entity_type* type)
{
	assert(type);
	assert(level->entities_count + 1 < level->entities_max_count);

	entity* new_entity = &level->entities[level->entities_count];
	level->entities_count++;
	new_entity->position = renormalize_position(position);
	new_entity->type = type;
	new_entity->health = type->max_health;
	return new_entity;
}

entity* add_entity(level_state* level, tile_position position, entity_type* type)
{
	entity* result = add_entity(level, get_world_position(position), type);
	return result;
}

void remove_entity(level_state* level, u32 entity_index)
{
	assert(level->entities_count > 1);
	assert(entity_index != 0);
	assert(entity_index < level->entities_max_count);

	// compact array - działa też w przypadku entity_index == entities_count - 1
	entity last_entity = level->entities[level->entities_count - 1];
	level->entities[entity_index] = last_entity;
	level->entities_count--;
}

entity_type_dictionary create_entity_types_dictionary(memory_arena* arena)
{
	entity_type_dictionary result = {};
	result.type_ptrs_count = (i32)entity_type_enum::_LAST + 1;
	result.type_ptrs = push_array(arena, result.type_ptrs_count, entity_type*);
	return result;
}

void set_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type, entity_type* enity_type_ptr)
{
	assert((i32)type < dictionary.type_ptrs_count);
	assert(dictionary.type_ptrs[(i32)type] == NULL);
	dictionary.type_ptrs[(i32)type] = enity_type_ptr;
}

entity_type* get_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type)
{
	assert((i32)type < dictionary.type_ptrs_count);
	entity_type* result = dictionary.type_ptrs[(i32)type];
	return result;
}

void fire_bullet(level_state* level, entity_type* bullet_type, world_position bullet_starting_position,
	v2 bullet_offset, v2 velocity)
{
	if (level->bullets_count < level->bullets_max_count)
	{
		bullet* bul = &level->bullets[level->bullets_count];
		bul->type = bullet_type;
		bullet_starting_position.pos_in_chunk += bullet_offset;
		bul->position = renormalize_position(bullet_starting_position);
		bul->velocity = velocity;
		level->bullets_count++;
	}
}

void fire_bullet(level_state* level, entity* entity, b32 cooldown)
{
	assert(entity->type->fired_bullet_type);

	v2 direction = (entity->direction == direction::E
		? get_v2(1.0f, 0.0f)
		: get_v2(-1.0f, 0.0f));
	v2 bullet_offset = (entity->direction == direction::E
		? entity->type->fired_bullet_offset
		: get_v2(-entity->type->fired_bullet_offset.x, entity->type->fired_bullet_offset.y));

	fire_bullet(level, entity->type->fired_bullet_type, entity->position, bullet_offset,
		direction * entity->type->fired_bullet_type->constant_velocity);

	if (cooldown)
	{
		entity->attack_cooldown = entity->type->default_attack_cooldown;
	}
}

void remove_bullet(level_state* level, u32 bullet_index)
{
	assert(level->bullets_count > 0);
	assert(bullet_index < level->bullets_max_count);

	// compact array - działa też w przypadku bullet_index == bullets_count - 1
	bullet last_bullet = level->bullets[level->bullets_count - 1];
	level->bullets[bullet_index] = last_bullet;
	level->bullets_count--;
}

tile_range find_walking_path_for_enemy(map level, map collision_ref, tile_position start_tile)
{
	b32 found_good_start_pos = false;
	tile_position good_start_tile = {};

	tile_position test_tile = start_tile;
	i32 distance_checking_limit = 10;
	for (i32 distance = 0; distance < distance_checking_limit; distance++)
	{
		test_tile.y = start_tile.y + distance;
		if (is_good_for_walk_path(level, collision_ref, test_tile.x, test_tile.y))
		{
			good_start_tile = test_tile;
			found_good_start_pos = true;
			break;
		}
	}

	if (good_start_tile.x == 0 && good_start_tile.y == 0)
	{
		return {};
	}

	tile_position left_end = good_start_tile;
	tile_position right_end = good_start_tile;

	test_tile = good_start_tile;
	for (i32 distance = 0; distance <= distance_checking_limit; distance++)
	{
		test_tile.x = good_start_tile.x - distance;
		if (is_good_for_walk_path(level, collision_ref, test_tile.x, test_tile.y))
		{
			left_end = test_tile;
		}
		else
		{
			break;
		}
	}

	test_tile = good_start_tile;
	for (i32 distance = 0; distance <= distance_checking_limit; distance++)
	{
		test_tile.x = good_start_tile.x + distance;
		if (is_good_for_walk_path(level, collision_ref, test_tile.x, test_tile.y))
		{
			right_end = test_tile;
		}
		else
		{
			break;
		}
	}

	tile_range result = {};
	result.start = left_end;
	result.end = right_end;
	return result;
}

// domyślnie trójkąt jest zwrócony podstawą w prawo
// można podać x i y odwrotnie dla trójkątów zwróconych podstawą w górę lub w dół
b32 is_point_within_right_triangle(r32 triangle_height, r32 relative_x, r32 relative_y, b32 invert_sign)
{
	b32 result = false;
	relative_x = (invert_sign ? -relative_x : relative_x);
	if (relative_x < triangle_height)
	{
		// trzeba pamiętać, że y idą do dołu ekranu
		r32 max_y = relative_x; // bok wyrażony f(x) = x
		r32 min_y = -relative_x; // bok wyrażony f(x) = -x
		if (relative_y <= max_y && relative_y >= min_y)
		{
			// znajdujemy się w trójkącie
			result = true;
		}
	}

	return result;
}

b32 is_point_visible_from_point(world_position looking_point, direction looking_direction, r32 max_looking_distance, world_position point_to_check)
{
	assert(looking_direction != direction::NONE);

	// sprawdzamy trójkąt z wierzchołkiem umiejscowionym w patrzącym entity
	// kąt przy tym wierzchołku jest prosty
	// a więc boki trójkąta można określić funkcjami f(x)=x, f(x)=-1
	b32 result = false;
	r32 triangle_height = max_looking_distance;

	v2 relative_pos = get_position_difference(point_to_check, looking_point);
	if (looking_direction == direction::E && relative_pos.x > 0.0f)
	{
		result = is_point_within_right_triangle(triangle_height, relative_pos.x, relative_pos.y, false);
	}
	else
	if (looking_direction == direction::W && relative_pos.x < 0.0f)
	{
	result = is_point_within_right_triangle(triangle_height, relative_pos.x, relative_pos.y, true);
	}
	else // odwracamy x i y
	if (looking_direction == direction::N && relative_pos.y > 0.0f)
	{
	result = is_point_within_right_triangle(triangle_height, relative_pos.y, relative_pos.x, false);
	}
	else
	if (looking_direction == direction::S && relative_pos.y < 0.0f)
	{
		result = is_point_within_right_triangle(triangle_height, relative_pos.y, relative_pos.x, true);
	}

	return result;
}

b32 is_point_visible_for_entity(level_state* level, entity* looking_entity, world_position point, r32 max_distance)
{
	b32 result = is_point_visible_from_point(looking_entity->position, looking_entity->direction, max_distance, point);

	if (result)
	{
		if (check_if_sight_line_is_obstructed(level, looking_entity->position, point))
		{
			result = false;
		}
	}

	return result;
};

void add_next_level_transition(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn)
{
	entity_type* transition_type = push_struct(arena, entity_type);
	transition_type->type_enum = entity_type_enum::NEXT_LEVEL_TRANSITION;

	tile_range occupied_tiles = find_vertical_range_of_free_tiles(
		level->current_map, level->static_data->collision_reference, new_entity_to_spawn->position, 20);
	transition_type->collision_rect_dim = get_length_from_tile_range(occupied_tiles);

	world_position new_position = add_to_position(
		get_world_position(occupied_tiles.start),
		get_position_difference(occupied_tiles.end, occupied_tiles.start) / 2);

	set_flags(&transition_type->flags, entity_flags::INDESTRUCTIBLE);

	add_entity(level, new_position, transition_type);
}

void add_message_display_entity(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn)
{
	entity_type* new_type = push_struct(arena, entity_type);

	u32 max_size = 10;

	tile_range occupied_tiles = find_vertical_range_of_free_tiles(
		level->current_map, level->static_data->collision_reference, new_entity_to_spawn->position, max_size);
	v2 collision_rect_dim = get_length_from_tile_range(occupied_tiles);

	new_type->collision_rect_dim = collision_rect_dim;
	new_type->message = new_entity_to_spawn->message;

	world_position new_position = add_to_position(
		get_world_position(occupied_tiles.start),
		get_position_difference(occupied_tiles.end, occupied_tiles.start) / 2);

	set_flags(&new_type->flags, entity_flags::INDESTRUCTIBLE);
	set_flags(&new_type->flags, entity_flags::MESSAGE_DISPLAY);

	entity* new_entity = add_entity(level, new_position, new_type);
}

void initialize_current_map(game_state* game, level_state* level)
{
	assert(false == level->current_map_initialized);

	temporary_memory memory_for_initialization = begin_temporary_memory(game->transient_arena);

	add_entity(level, level->current_map.starting_tile,
		get_entity_type_ptr(level->static_data->entity_types_dict, entity_type_enum::PLAYER));

	level->gates_dict.entries_count = 100;
	level->gates_dict.entries = push_array(game->arena, level->gates_dict.entries_count, gate_dictionary_entry);

	level->gate_tints_dict.sprite_effects_count = 100;
	level->gate_tints_dict.sprite_effects = push_array(game->arena, level->gate_tints_dict.sprite_effects_count, sprite_effect*);
	level->gate_tints_dict.probing_jump = 7;

	for (u32 entity_index = 0;
		entity_index < level->current_map.entities_to_spawn_count;
		entity_index++)
	{
		entity_to_spawn* new_entity = level->current_map.entities_to_spawn + entity_index;
		switch (new_entity->type)
		{
			case entity_type_enum::GATE:
			{
				add_gate_entity(level, game->arena, new_entity, false);
			}
			break;
			case entity_type_enum::SWITCH:
			{
				add_gate_entity(level, game->arena, new_entity, true);
			}
			break;
			case entity_type_enum::NEXT_LEVEL_TRANSITION:
			{
				add_next_level_transition(level, game->arena, new_entity);
			}
			break;
			case entity_type_enum::MESSAGE_DISPLAY:
			{
				add_message_display_entity(level, game->arena, new_entity);
			}
			break;
			case entity_type_enum::UNKNOWN:
			{
				// ignorujemy
			}
			break;
			default:
			{
				add_entity(level, get_world_position(new_entity->position),
					get_entity_type_ptr(level->static_data->entity_types_dict, new_entity->type));
			}
			break;
		}
	}

	end_temporary_memory(memory_for_initialization);

	level->current_map_initialized = true;
}