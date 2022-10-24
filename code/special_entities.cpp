﻿#include "jorutils.h"
#include "jormath.h"
#include "main.h"
#include "animation.h"
#include "map.h"
#include "entities.h"

u32 get_hash_from_color(v4 color)
{
	u32 result = ((u32)color.r * 13 + (u32)color.g * 7 + (u32)color.b * 3);
	return result;
}

void add_gate_to_dictionary(memory_arena* arena, gate_dictionary dict, entity* gate_entity)
{
	v4 color = gate_entity->type->color;
	u32 index = get_hash_from_color(color) % dict.entries_count;
	gate_dictionary_entry* entry = dict.entries + index;
	if (entry->entity == NULL)
	{
		entry->entity = gate_entity;
	}
	else
	{
		while (entry && entry->next)
		{
			entry = entry->next;
		}

		gate_dictionary_entry* new_entry = push_struct(arena, gate_dictionary_entry);
		new_entry->entity = gate_entity;
		entry->next = new_entry;
	}
}

sprite_effect* get_sprite_effect_by_color(sprite_effect_dictionary dict, v4 color)
{
	sprite_effect* result = NULL;
	u32 index_to_check = get_hash_from_color(color) % dict.sprite_effects_count;
	for (u32 attempts = 0; attempts < dict.sprite_effects_count; attempts++)
	{
		sprite_effect* effect = dict.sprite_effects[index_to_check];
		if (effect && effect->color == color)
		{
			result = effect;
			break;
		}

		index_to_check = (index_to_check + dict.probing_jump) % dict.sprite_effects_count;
	}
	return result;
}

void set_sprite_effect_for_color(sprite_effect_dictionary dict, sprite_effect* effect)
{
	u32 index_to_check = get_hash_from_color(effect->color) % dict.sprite_effects_count;
	for (u32 attempts = 0; attempts < dict.sprite_effects_count; attempts++)
	{
		if (dict.sprite_effects[index_to_check] == NULL
			|| is_zero(dict.sprite_effects[index_to_check]->color))
		{
			dict.sprite_effects[index_to_check] = effect;
			break;
		}

		if (dict.sprite_effects[index_to_check]->color == effect->color)
		{
			break;
		}

		index_to_check = (index_to_check + dict.probing_jump) % dict.sprite_effects_count;
	}
}

void invalidate_paths_after_gate_opening(level_state* level)
{
	for (u32 entity_index = 1; entity_index < level->entities_count; entity_index++)
	{
		entity* entity = level->entities + entity_index;
		if (false == entity->used)
		{
			continue;
		}

		// w przypadku przeciwników i platform latających pionowo otworzenie bramy nic nie zmienia
		if (are_entity_flags_set(entity, entity_flags::WALKS_HORIZONTALLY)
			|| are_entity_flags_set(entity, entity_flags::FLIES_HORIZONTALLY)
			|| are_entity_flags_set(entity, entity_flags::MOVING_PLATFORM_HORIZONTAL))
		{
			entity->has_path = false;
		}
	}
}

void open_gates_with_given_color(level_state* level, v4 color)
{
	u32 index = get_hash_from_color(color) % level->gates_dict.entries_count;
	gate_dictionary_entry* entry = &level->gates_dict.entries[index];
	while (entry)
	{
		if (entry->entity != NULL)
		{
			// na wypadek kolizji hashy
			if (entry->entity->type->color == color)
			{
				tile_position pos = get_tile_position(entry->entity->position);
				if (are_entity_flags_set(entry->entity, entity_flags::GATE))
				{
					unset_flags(&entry->entity->type->flags, entity_flags::BLOCKS_MOVEMENT);
					sprite* sprite = &entry->entity->type->idle_pose.sprite;
					for (u32 sprite_index = 1; sprite_index < sprite->parts_count - 1; sprite_index++)
					{
						sprite->parts[sprite_index].texture = textures::NONE;
					}
				
					invalidate_paths_after_gate_opening(level);
				}
				else if (are_entity_flags_set(entry->entity, entity_flags::TINTED_DISPLAY))
				{
					start_visual_effect(level, entry->entity, sprite_effects_types::GATE_DISPLAY_INACTIVE);
				}

				// w ten sposób nie będziemy otwierać bram ponownie
				entry->entity = NULL;
			}
		}

		entry = entry->next;
	}
}

void add_gate_entity(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn, b32 is_switch)
{
	entity_type* new_type = push_struct(arena, entity_type);

	v2 collision_rect_dim;
	tile_range occupied_tiles;
	if (is_switch)
	{
		u32 max_size = 1; // krótsze jednak wyglądają lepiej
		occupied_tiles = find_horizontal_range_of_free_tiles(&level->current_map, 
			new_entity_to_spawn->position, max_size);
		collision_rect_dim = get_collision_dim_from_tile_range(occupied_tiles);
	}
	else
	{
		u32 max_size = 30;
		occupied_tiles = find_vertical_range_of_free_tiles(&level->current_map, 
			new_entity_to_spawn->position, max_size);
		collision_rect_dim = get_collision_dim_from_tile_range(occupied_tiles);
	}

	new_type->collision_rect_dim = collision_rect_dim;
	new_type->color = new_entity_to_spawn->color;

	world_position new_position = add_to_position(
		get_world_position(occupied_tiles.start),
		get_position_difference(occupied_tiles.end, occupied_tiles.start) / 2);

	if (is_switch)
	{
		switch_graphics switch_gfx = level->static_data->switches_gfx.blue;
		switch (new_entity_to_spawn->type)
		{
			case entity_type_enum::SWITCH_SILVER:  switch_gfx = level->static_data->switches_gfx.blue; break;
			case entity_type_enum::SWITCH_GOLD:  switch_gfx = level->static_data->switches_gfx.grey; break;
			case entity_type_enum::SWITCH_RED:   switch_gfx = level->static_data->switches_gfx.red; break;
			case entity_type_enum::SWITCH_GREEN: switch_gfx = level->static_data->switches_gfx.green; break;
		}

		u32 tiles_count = occupied_tiles.end.x - occupied_tiles.start.x + 1;

		animation_frame frame = {};
		frame.sprite.parts_count = tiles_count;
		frame.sprite.parts = push_array(arena, frame.sprite.parts_count, sprite_part);

		for (u32 distance = 0; distance < tiles_count; distance++)
		{
			sprite_part* part = &frame.sprite.parts[distance];
			if (distance == 0)
			{
				*part = switch_gfx.frame_left;
			}
			else if (distance == tiles_count - 1)
			{
				*part = switch_gfx.frame_right;
			}
			else
			{
				*part = switch_gfx.frame_middle;
			}
			part->offset_in_pixels = get_position_difference(
				get_tile_position(occupied_tiles.start.x + distance, occupied_tiles.start.y), new_position)
				* TILE_SIDE_IN_PIXELS;
		}

		new_type->idle_pose = frame;
	}
	else
	{
		gate_graphics gate_gfx = level->static_data->gates_gfx.blue;
		switch (new_entity_to_spawn->type)
		{
			case entity_type_enum::GATE_SILVER:  gate_gfx = level->static_data->gates_gfx.blue; break;
			case entity_type_enum::GATE_GOLD:  gate_gfx = level->static_data->gates_gfx.grey; break;
			case entity_type_enum::GATE_RED:   gate_gfx = level->static_data->gates_gfx.red; break;
			case entity_type_enum::GATE_GREEN: gate_gfx = level->static_data->gates_gfx.green; break;
		}

		u32 tiles_count = occupied_tiles.end.y - occupied_tiles.start.y + 3;

		animation_frame frame = {};
		frame.sprite.parts_count = tiles_count;
		frame.sprite.parts = push_array(arena, frame.sprite.parts_count, sprite_part);

		for (u32 distance = 0; distance < tiles_count; distance++)
		{
			sprite_part* part = &frame.sprite.parts[distance];
			if (distance == 0)
			{
				*part = gate_gfx.frame_upper;
			}
			else if (distance == tiles_count - 1)
			{
				*part = gate_gfx.frame_lower;
			}
			else
			{
				*part = gate_gfx.gate;
			}

			part->offset_in_pixels = get_position_difference(
				get_tile_position(occupied_tiles.start.x, occupied_tiles.start.y - 1 + distance), new_position)
				* TILE_SIDE_IN_PIXELS;
		}

		new_type->idle_pose = frame;
	}

	set_flags(&new_type->flags, entity_flags::BLOCKS_MOVEMENT);
	set_flags(&new_type->flags, entity_flags::INDESTRUCTIBLE);
	set_flags(&new_type->flags, (is_switch ? entity_flags::SWITCH : entity_flags::GATE));

	entity* new_entity = add_entity(level, new_position, new_type);
	add_gate_to_dictionary(arena, level->gates_dict, new_entity);

	// dodanie wyświetlaczy

	entity_type* new_display_type = push_struct(arena, entity_type);
	set_flags(&new_display_type->flags, entity_flags::TINTED_DISPLAY);
	new_display_type->color = new_entity_to_spawn->color;

	display_graphics display_gfx = level->static_data->display_gfx;
	if (is_switch)
	{
		u32 tiles_count = occupied_tiles.end.x - occupied_tiles.start.x + 1;

		animation_frame frame = {};
		frame.sprite.parts_count = tiles_count;
		frame.sprite.parts = push_array(arena, frame.sprite.parts_count, sprite_part);

		for (u32 distance = 0; distance < tiles_count; distance++)
		{
			sprite_part* part = &frame.sprite.parts[distance];
			if (distance == 0)
			{
				*part = display_gfx.switch_left_display;
			}
			else if (distance == tiles_count - 1)
			{
				*part = display_gfx.switch_right_display;
			}
			else
			{
				*part = display_gfx.switch_middle_display;
			}
			part->offset_in_pixels = get_position_difference(
				get_tile_position(occupied_tiles.start.x + distance, occupied_tiles.start.y), new_position)
				* TILE_SIDE_IN_PIXELS;
		}

		new_display_type->idle_pose = frame;
	}
	else
	{
		animation_frame frame = {};
		frame.sprite.parts_count = 2;
		frame.sprite.parts = push_array(arena, frame.sprite.parts_count, sprite_part);

		frame.sprite.parts[0] = display_gfx.gate_upper_display;
		frame.sprite.parts[0].offset_in_pixels = get_position_difference(
			get_tile_position(occupied_tiles.start.x, occupied_tiles.start.y - 1), new_position)
			* TILE_SIDE_IN_PIXELS;
		frame.sprite.parts[1] = display_gfx.gate_lower_display;
		frame.sprite.parts[1].offset_in_pixels = get_position_difference(
			get_tile_position(occupied_tiles.start.x, occupied_tiles.end.y + 1), new_position)
			* TILE_SIDE_IN_PIXELS;

		new_display_type->idle_pose = frame;
	}

	entity* display_entity = add_entity(level, new_position, new_display_type);
	add_gate_to_dictionary(arena, level->gates_dict, display_entity);

	// efekt kolorystyczny

	sprite_effect* tint_effect = get_sprite_effect_by_color(level->gate_tints_dict, new_entity_to_spawn->color);
	if (tint_effect == NULL)
	{
		tint_effect = push_struct(arena, sprite_effect);
		tint_effect->type = sprite_effects_types::GATE_DISPLAY_ACTIVE;
		tint_effect->stages_count = 1;
		tint_effect->stages = push_array(arena, tint_effect->stages_count, sprite_effect_stage);
		tint_effect->color = new_entity_to_spawn->color;
		tint_effect->stages[0].amplitude = 1.0f;
		tint_effect->stages[0].vertical_shift = 0.7f;
		tint_effect->stages[0].phase_shift = 0;
		tint_effect->stages[0].period = 3.0f;
		tint_effect->stages[0].stage_duration = 0.0f;
		tint_effect->total_duration = tint_effect->stages[0].stage_duration;

		set_flags(&tint_effect->flags, sprite_effect_flags::REPEATS);
		set_sprite_effect_for_color(level->gate_tints_dict, tint_effect);
	}

	start_visual_effect(display_entity, tint_effect);
}

u32 get_moving_platform_type_index(entity_type_enum type)
{
	u32 result = 0;
	switch (type)
	{
		case entity_type_enum::MOVING_PLATFORM_HORIZONTAL_SILVER:  result = 0; break;
		case entity_type_enum::MOVING_PLATFORM_VERTICAL_SILVER:    result = 1; break;
		case entity_type_enum::MOVING_PLATFORM_HORIZONTAL_GOLD:  result = 2; break;
		case entity_type_enum::MOVING_PLATFORM_VERTICAL_GOLD:    result = 3; break;
		case entity_type_enum::MOVING_PLATFORM_HORIZONTAL_RED:   result = 4; break;
		case entity_type_enum::MOVING_PLATFORM_VERTICAL_RED:     result = 5; break;
		case entity_type_enum::MOVING_PLATFORM_HORIZONTAL_GREEN: result = 6; break;
		case entity_type_enum::MOVING_PLATFORM_VERTICAL_GREEN:   result = 7; break;
			invalid_default_case;
	}
	return result;
}

void add_moving_platform_entity(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn)
{
	u32 type_index = get_moving_platform_type_index(new_entity_to_spawn->type);
	entity_type* type = level->moving_platform_types[type_index];

	b32 is_horizontal =
		(new_entity_to_spawn->type == entity_type_enum::MOVING_PLATFORM_HORIZONTAL_SILVER
			|| new_entity_to_spawn->type == entity_type_enum::MOVING_PLATFORM_HORIZONTAL_GOLD
			|| new_entity_to_spawn->type == entity_type_enum::MOVING_PLATFORM_HORIZONTAL_RED
			|| new_entity_to_spawn->type == entity_type_enum::MOVING_PLATFORM_HORIZONTAL_GREEN);

	if (type == NULL)
	{
		type = push_struct(arena, entity_type);

		moving_platform_graphics platform_gfx = level->static_data->platforms_gfx.blue;
		switch (new_entity_to_spawn->type)
		{
			case entity_type_enum::MOVING_PLATFORM_HORIZONTAL_SILVER:
			case entity_type_enum::MOVING_PLATFORM_VERTICAL_SILVER:
			{
				platform_gfx = level->static_data->platforms_gfx.blue;
			}
			break;
			case entity_type_enum::MOVING_PLATFORM_HORIZONTAL_GOLD:
			case entity_type_enum::MOVING_PLATFORM_VERTICAL_GOLD:
			{
				platform_gfx = level->static_data->platforms_gfx.grey;
			}
			break;
			case entity_type_enum::MOVING_PLATFORM_HORIZONTAL_RED:
			case entity_type_enum::MOVING_PLATFORM_VERTICAL_RED:
			{
				platform_gfx = level->static_data->platforms_gfx.red;
			}
			break;
			case entity_type_enum::MOVING_PLATFORM_HORIZONTAL_GREEN:
			case entity_type_enum::MOVING_PLATFORM_VERTICAL_GREEN:
			{
				platform_gfx = level->static_data->platforms_gfx.green;
			}
			break;
			invalid_default_case;
		}

		animation_frame frame = {};
		frame.sprite.parts_count = 3;
		frame.sprite.parts = push_array(arena, frame.sprite.parts_count, sprite_part);
		frame.sprite.parts[0] = platform_gfx.left;
		frame.sprite.parts[0].offset_in_pixels = get_v2(-1, 0) * TILE_SIDE_IN_PIXELS;
		frame.sprite.parts[1] = platform_gfx.middle;
		frame.sprite.parts[2] = platform_gfx.right;
		frame.sprite.parts[2].offset_in_pixels = get_v2(1, 0) * TILE_SIDE_IN_PIXELS;
		type->idle_pose = frame;

		type->collision_rect_dim = get_v2(3, 1);

		type->velocity_multiplier = level->static_data->moving_platform_velocity;

		set_flags(&type->flags, entity_flags::BLOCKS_MOVEMENT);
		set_flags(&type->flags, entity_flags::INDESTRUCTIBLE);

		if (is_horizontal)
		{
			set_flags(&type->flags, entity_flags::MOVING_PLATFORM_HORIZONTAL);
		}
		else
		{
			set_flags(&type->flags, entity_flags::MOVING_PLATFORM_VERTICAL);
		}
	}

	// jeśli znajdujemy się przy ścianie, odsuwamy się
	tile_position entity_position = new_entity_to_spawn->position;
	tile_position tile_to_left = get_tile_position(entity_position.x - 1, entity_position.y);
	tile_position tile_to_right = get_tile_position(entity_position.x + 1, entity_position.y);
	b32 left_tile_collides = is_tile_colliding(&level->current_map, tile_to_left);
	b32 right_tile_collides = is_tile_colliding(&level->current_map, tile_to_right);
	if (left_tile_collides && right_tile_collides)
	{
		// nie mamy gdzie przesunąć
	}
	else
	{
		if (left_tile_collides && false == right_tile_collides)
		{
			entity_position.x++;
		}
		else if (right_tile_collides && false == left_tile_collides)
		{
			entity_position.x--;
		}
	}

	add_entity(level, entity_position, type);
}

void add_next_level_transition_entity(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn)
{
	entity_type* transition_type = push_struct(arena, entity_type);
	transition_type->type_enum = entity_type_enum::NEXT_LEVEL_TRANSITION;

	tile_range occupied_tiles = find_vertical_range_of_free_tiles(&level->current_map,
		new_entity_to_spawn->position, 20);
	transition_type->collision_rect_dim = get_collision_dim_from_tile_range(occupied_tiles);

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

	tile_range occupied_tiles = find_vertical_range_of_free_tiles(&level->current_map,
		new_entity_to_spawn->position, max_size);
	v2 collision_rect_dim = get_collision_dim_from_tile_range(occupied_tiles);

	new_type->collision_rect_dim = collision_rect_dim;
	new_type->message = new_entity_to_spawn->message;

	world_position new_position = add_to_position(
		get_world_position(occupied_tiles.start),
		get_position_difference(occupied_tiles.end, occupied_tiles.start) / 2);

	set_flags(&new_type->flags, entity_flags::INDESTRUCTIBLE);
	set_flags(&new_type->flags, entity_flags::MESSAGE_DISPLAY);

	add_entity(level, new_position, new_type);
}
