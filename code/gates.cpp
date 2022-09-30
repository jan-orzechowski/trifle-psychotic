#include "jorutils.h"
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

void open_gates_with_given_color(level_state* level, v4 color)
{
	u32 index = get_hash_from_color(color) % level->gates_dict.entries_count;
	gate_dictionary_entry* entry = &level->gates_dict.entries[index];
	while (entry)
	{
		if (entry->entity != NULL)
		{
			// na wypadek kolizji
			if (entry->entity->type->color == color)
			{
				tile_position pos = get_tile_position(entry->entity->position);
				if (are_entity_flags_set(entry->entity, entity_flags::SWITCH))
				{
					printf("switch na pozycji (%d,%d)\n", pos.x, pos.y);
				}
				else if (are_entity_flags_set(entry->entity, entity_flags::GATE))
				{
					unset_flags(&entry->entity->type->flags, entity_flags::BLOCKS_MOVEMENT);
					sprite* sprite = &entry->entity->type->idle_pose.sprite;
					for (u32 sprite_index = 1; sprite_index < sprite->parts_count - 1; sprite_index++)
					{
						sprite->parts[sprite_index].texture = textures::NONE;
					}

					printf("brama na pozycji (%d,%d)\n", pos.x, pos.y);
				}
				else if (are_entity_flags_set(entry->entity, entity_flags::TINTED_DISPLAY))
				{
					start_visual_effect(entry->entity, &level->static_data->visual_effects[0], true);
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
	u32 max_size = 15;
	if (is_switch)
	{
		occupied_tiles = find_horizontal_range_of_free_tiles(
			level->current_map, level->static_data->collision_reference, new_entity_to_spawn->position, max_size);
		collision_rect_dim = get_collision_dim_from_tile_range(occupied_tiles);
	}
	else
	{
		occupied_tiles = find_vertical_range_of_free_tiles(
			level->current_map, level->static_data->collision_reference, new_entity_to_spawn->position, max_size);
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
			case entity_type_enum::SWITCH_BLUE:  switch_gfx = level->static_data->switches_gfx.blue; break;
			case entity_type_enum::SWITCH_GREY:  switch_gfx = level->static_data->switches_gfx.grey; break;
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
			case entity_type_enum::GATE_BLUE:  gate_gfx = level->static_data->gates_gfx.blue; break;
			case entity_type_enum::GATE_GREY:  gate_gfx = level->static_data->gates_gfx.grey; break;
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
		tint_effect->stages_count = 1;
		tint_effect->stages = push_array(arena, tint_effect->stages_count, sprite_effect_stage);
		tint_effect->color = new_entity_to_spawn->color;
		tint_effect->stages[0].amplitude = 1.0f;
		tint_effect->stages[0].vertical_shift = 0.5f;
		tint_effect->stages[0].phase_shift = 0;
		tint_effect->stages[0].period = 3.0f;
		tint_effect->stages[0].stage_duration = 0.0f;
		tint_effect->total_duration = tint_effect->stages[0].stage_duration;

		set_flags(&tint_effect->flags, sprite_effect_flags::REPEATS);
		set_sprite_effect_for_color(level->gate_tints_dict, tint_effect);
	}

	start_visual_effect(display_entity, tint_effect, true);
}