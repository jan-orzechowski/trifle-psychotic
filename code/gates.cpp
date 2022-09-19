#include "jorutils.h"
#include "jormath.h"
#include "main.h"
#include "animation.h"
#include "map.h"

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

void open_gates_with_given_color(game_data* game, v4 color)
{
	u32 index = get_hash_from_color(color) % game->gates_dict.entries_count;
	gate_dictionary_entry* entry = &game->gates_dict.entries[index];
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
						sprite->parts[sprite_index].texture = NULL;
					}

					printf("brama na pozycji (%d,%d)\n", pos.x, pos.y);
				}
				else if (are_entity_flags_set(entry->entity, entity_flags::TINTED_DISPLAY))
				{
					start_visual_effect(entry->entity, &game->static_data->visual_effects[0], true);
				}

				// w ten sposób nie będziemy otwierać bram ponownie
				entry->entity = NULL;
			}
		}

		entry = entry->next;
	}
}

void add_gate_entity(game_data* game, memory_arena* arena, entity_to_spawn* new_entity_to_spawn, b32 is_switch)
{
	entity_type* new_type = push_struct(arena, entity_type);

	u32 max_size = 10;

	v2 collision_rect_dim;
	tile_range occupied_tiles;
	if (is_switch)
	{
		occupied_tiles = find_horizontal_range_of_free_tiles(
			game->current_level, game->static_data->collision_reference, new_entity_to_spawn->position, max_size);
		collision_rect_dim = get_length_from_tile_range(occupied_tiles);
	}
	else
	{
		occupied_tiles = find_vertical_range_of_free_tiles(
			game->current_level, game->static_data->collision_reference, new_entity_to_spawn->position, max_size);
		collision_rect_dim = get_length_from_tile_range(occupied_tiles);
	}

	new_type->collision_rect_dim = collision_rect_dim;
	new_type->color = new_entity_to_spawn->color;

	world_position new_position = add_to_position(
		get_world_position(occupied_tiles.start),
		get_position_difference(occupied_tiles.end, occupied_tiles.start) / 2);

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
				*part = game->static_data->switch_frame_left_sprite;
			}
			else if (distance == tiles_count - 1)
			{
				*part = game->static_data->switch_frame_right_sprite;
			}
			else
			{
				*part = game->static_data->switch_frame_middle_sprite;
			}
			part->offset_in_pixels = get_position_difference(
				get_tile_position(occupied_tiles.start.x + distance, occupied_tiles.start.y), new_position)
				* TILE_SIDE_IN_PIXELS;
		}

		new_type->idle_pose = frame;
	}
	else
	{
		u32 tiles_count = occupied_tiles.end.y - occupied_tiles.start.y + 3;

		animation_frame frame = {};
		frame.sprite.parts_count = tiles_count;
		frame.sprite.parts = push_array(arena, frame.sprite.parts_count, sprite_part);

		for (u32 distance = 0; distance < tiles_count; distance++)
		{
			sprite_part* part = &frame.sprite.parts[distance];
			if (distance == 0)
			{
				*part = game->static_data->gate_frame_lower_sprite;
			}
			else if (distance == tiles_count - 1)
			{
				*part = game->static_data->gate_frame_upper_sprite;
			}
			else
			{
				*part = game->static_data->gate_sprite;
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

	entity* new_entity = add_entity(game, new_position, new_type);
	add_gate_to_dictionary(arena, game->gates_dict, new_entity);

	// dodanie wyświetlaczy

	entity_type* new_display_type = push_struct(arena, entity_type);
	set_flags(&new_display_type->flags, entity_flags::TINTED_DISPLAY);
	new_display_type->color = new_entity_to_spawn->color;

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
				*part = game->static_data->switch_display_left_sprite;
			}
			else if (distance == tiles_count - 1)
			{
				*part = game->static_data->switch_display_right_sprite;
			}
			else
			{
				*part = game->static_data->switch_display_middle_sprite;
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

		frame.sprite.parts[0] = game->static_data->gate_display_upper_sprite;
		frame.sprite.parts[0].offset_in_pixels = get_position_difference(
			get_tile_position(occupied_tiles.start.x, occupied_tiles.start.y - 1), new_position)
			* TILE_SIDE_IN_PIXELS;
		frame.sprite.parts[1] = game->static_data->gate_display_lower_sprite;
		frame.sprite.parts[1].offset_in_pixels = get_position_difference(
			get_tile_position(occupied_tiles.start.x, occupied_tiles.end.y + 1), new_position)
			* TILE_SIDE_IN_PIXELS;

		new_display_type->idle_pose = frame;
	}

	entity* display_entity = add_entity(game, new_position, new_display_type);
	add_gate_to_dictionary(arena, game->gates_dict, display_entity);

	// efekt kolorystyczny

	sprite_effect* tint_effect = get_sprite_effect_by_color(game->gate_tints_dict, new_entity_to_spawn->color);
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
		set_sprite_effect_for_color(game->gate_tints_dict, tint_effect);
	}

	start_visual_effect(display_entity, tint_effect, true);
}