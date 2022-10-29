#include "level_initialization.h"
#include "entities.h"
#include "special_entities.h"
#include "level_parsing.h"
#include "progress.h"

void initialize_level_state(level_state* level, static_game_data* static_data, string_ref map_name, memory_arena* arena)
{
	*level = {};

	level->current_map_name = copy_string(arena, map_name);

	level->static_data = static_data;

	level->entities_count = 0;
	level->entities_max_count = 1000;
	level->entities = push_array(arena, level->entities_max_count, entity);

	level->bullets_count = 0;
	level->bullets_max_count = 5000;
	level->bullets = push_array(arena, level->bullets_max_count, bullet);

	level->explosions_count = 0;
	level->explosions_max_count = 500;
	level->explosions = push_array(arena, level->explosions_max_count, entity);

	level->player_movement.current_mode = player_movement_mode::WALK;
	level->player_movement.standing_history.buffer_size = 10;
	level->player_movement.standing_history.buffer = push_array(arena,
		level->player_movement.standing_history.buffer_size, b32);

	level->fade_in_perc = static_data->game_fade_in_speed;
}

void initialize_level_introduction(level_state* level, memory_arena* arena)
{
	level->show_level_introduction = false;
	level->introduction = {};

	if (level->current_map.introduction.string_size > 0)
	{
		level->introduction.can_be_skipped_timer = level->static_data->default_introduction_can_be_skipped_timer;
		level->introduction.fade_in_perc = level->static_data->game_fade_in_speed;
		level->introduction.text_y_offset = (SCREEN_HEIGHT / SCALING_FACTOR);
		level->show_level_introduction = true;

		if (level->current_map.introduction_lines == NULL)
		{
			level->current_map.introduction_lines = get_division_of_text_into_lines(
				arena, &level->static_data->scrolling_text_options, 
				level->current_map.introduction);
		}
	}
}

void initialize_current_map(level_state* level, memory_arena* arena)
{
	assert(false == level->current_map_initialized);

	level->current_map.collision_reference = level->static_data->collision_reference;

	// add player
	{
		entity_type* player_type = get_entity_type_ptr(
			level->static_data->entity_types_dict, entity_type_enum::PLAYER);

		if (level->current_map.initial_max_player_health > 0.0f)
		{
			player_type->max_health = level->current_map.initial_max_player_health;
		}
		else
		{
			player_type->max_health = level->static_data->default_player_max_health;
		}

		// gracz ma środek w innym miejscu niż pozostałe entities 
		// - z tego powodu musimy skorygować początkowe położenie
		world_position starting_position = get_world_position(level->current_map.starting_tile);
		starting_position = add_to_position(starting_position,
			get_v2(0, -(player_type->collision_rect_dim.y / 2) + 0.57f));

		add_entity(level, starting_position, player_type);
	}

	level->gates_dict.entries_count = 100;
	level->gates_dict.entries = push_array(arena, level->gates_dict.entries_count, gate_dictionary_entry);

	level->gate_tints_dict.sprite_effects_count = 100;
	level->gate_tints_dict.sprite_effects = push_array(arena, level->gate_tints_dict.sprite_effects_count, sprite_effect*);
	level->gate_tints_dict.probing_jump = 7;

	entity_to_spawn* new_entity = level->current_map.first_entity_to_spawn;
	while (new_entity)
	{
		switch (new_entity->type)
		{
			case entity_type_enum::GATE_SILVER:
			case entity_type_enum::GATE_GOLD:
			case entity_type_enum::GATE_RED:
			case entity_type_enum::GATE_GREEN:
			{
				add_gate_entity(level, arena, new_entity, false);
			}
			break;
			case entity_type_enum::SWITCH_SILVER:
			case entity_type_enum::SWITCH_GOLD:
			case entity_type_enum::SWITCH_RED:
			case entity_type_enum::SWITCH_GREEN:
			{
				add_gate_entity(level, arena, new_entity, true);
			}
			break;
			case entity_type_enum::NEXT_LEVEL_TRANSITION:
			{
				add_next_level_transition_entity(level, arena, new_entity);
			}
			break;
			case entity_type_enum::MESSAGE_DISPLAY:
			{
				add_message_display_entity(level, arena, new_entity);
			}
			break;
			case entity_type_enum::MOVING_PLATFORM_HORIZONTAL_SILVER:
			case entity_type_enum::MOVING_PLATFORM_HORIZONTAL_GOLD:
			case entity_type_enum::MOVING_PLATFORM_HORIZONTAL_RED:
			case entity_type_enum::MOVING_PLATFORM_HORIZONTAL_GREEN:
			case entity_type_enum::MOVING_PLATFORM_VERTICAL_SILVER:
			case entity_type_enum::MOVING_PLATFORM_VERTICAL_GOLD:
			case entity_type_enum::MOVING_PLATFORM_VERTICAL_RED:
			case entity_type_enum::MOVING_PLATFORM_VERTICAL_GREEN:
			{
				add_moving_platform_entity(level, arena, new_entity);
			}
			break;
			case entity_type_enum::UNKNOWN:
			{
				// ignorujemy
			}
			break;
			default:
			{
				entity_type* type = get_entity_type_ptr(
					level->static_data->entity_types_dict, new_entity->type);
				world_position position = get_world_position(new_entity->position);

				add_entity(level, position, type);

				if (new_entity->type == entity_type_enum::ENEMY_MESSENGER)
				{
					level->enemies_to_kill_counter++;
				}
			}
			break;
		}

		new_entity = new_entity->next;
	}

	level->current_map_initialized = true;
}

void change_and_initialize_level(game_state* game, scene_change scene_change)
{
	/*printf("przed inicjalizacja permanent arena: %d, transient arena: %d\n",
						game->arena->size_used, game->transient_arena->size_used);*/

	temporary_memory auxillary_memory_for_loading = begin_temporary_memory(game->transient_arena);
	{
		string_ref level_to_load_name = {};
		if (game->cmd_level_to_load.string_size > 0)
		{
			level_to_load_name = game->cmd_level_to_load;
			game->cmd_level_to_load = {}; // ładujemy tylko raz
		}
		else if (scene_change.restore_checkpoint
			&& game->checkpoint.used
			&& game->checkpoint.map_name.string_size > 0)
		{
			level_to_load_name = copy_string(game->transient_arena, game->checkpoint.map_name);
		}
		else if (scene_change.map_to_load.string_size)
		{
			level_to_load_name = copy_string(game->transient_arena, scene_change.map_to_load);
		}

		if (level_to_load_name.string_size == 0)
		{
			level_to_load_name = copy_c_string(game->transient_arena, "map_01");
		}

		if (game->game_level_memory.size_used_at_creation != 0)
		{
			end_temporary_memory(game->game_level_memory, true);
		}
		game->game_level_memory = begin_temporary_memory(game->arena);

		initialize_level_state(game->level_state, game->static_data, level_to_load_name, game->arena);
		tmx_map_parsing_result parsing_result = load_map(&game->platform, level_to_load_name, game->arena, game->transient_arena);
		if (parsing_result.errors->errors_count > 0)
		{
			game->map_errors = get_parsing_errors_message(game->arena,
				&game->static_data->parsing_errors_text_options, parsing_result.errors);

			game->level_initialized = false;
		}
		else
		{
			game->map_errors = {};
			game->level_state->current_map = parsing_result.parsed_map;
			initialize_current_map(game->level_state, game->arena);
			game->level_initialized = true;

			if (scene_change.restore_checkpoint && game->checkpoint.used)
			{
				restore_checkpoint(game);
			}
			else
			{
				if (false == game->skip_introduction)
				{
					// wstęp pokazujemy tylko za pierwszym razem
					initialize_level_introduction(game->level_state, game->arena);
				}
			}

			save_checkpoint(game);

			game->platform.start_playing_music(game->level_state->current_map.music_file_name);
		}

		end_temporary_memory(auxillary_memory_for_loading, true);
	}

	/*	printf("po inicjalizacji permanent arena: %d, transient arena: %d\n",
			game->arena->size_used, game->transient_arena->size_used);*/
}