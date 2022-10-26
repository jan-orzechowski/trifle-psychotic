#include "progress.h"
#include "player.h"

void save_checkpoint(game_state* game)
{
	assert(game->level_initialized);
	assert(game->level_state->current_map_initialized);

	game->checkpoint = {};
	game->checkpoint.used = true;
	game->checkpoint.map_name = copy_string_to_buffer(game->level_name_buffer, 
		MAX_LEVEL_NAME_LENGTH, game->level_state->current_map_name);

	entity* player = get_player(game->level_state);
	if (player->type)
	{
		game->checkpoint.player_max_health = player->type->max_health;
	}
}

void restore_checkpoint(game_state* game)
{
	assert(game->level_initialized);
	assert(game->level_state->current_map_initialized);

	entity* player = get_player(game->level_state);
	if (player->type && game->checkpoint.used)
	{
		player->type->max_health = game->checkpoint.player_max_health;
		player->health = player->type->max_health;

#if TRIFLE_DEBUG
		printf("wczytane max health: %d\n", game->checkpoint.player_max_health);
#endif
	}
}

void save_completed_levels(platform_api* platform, static_game_data* data, memory_arena* transient_arena)
{
	temporary_memory memory_for_string_builder = begin_temporary_memory(transient_arena);

	string_builder builder = get_string_builder(transient_arena, 1000);

	for (u32 level_index = 0; level_index < data->levels_count; level_index++)
	{
		level_choice* level = data->levels + level_index;
		if (level->completed)
		{
			push_string(&builder, level->map_name);
			push_string(&builder, ",");
		}
	}

	string_ref text_to_save = get_string_from_string_builder(&builder);

	write_to_file save = {};
	save.buffer = text_to_save.ptr;
	save.length = text_to_save.string_size;

	platform->save_prefs(save);

	end_temporary_memory(memory_for_string_builder, true);
}

void mark_level_as_completed(static_game_data* data, string_ref name)
{
	if (compare_to_c_string(name, "custom") == false)
	{
		for (u32 level_index = 0; level_index < data->levels_count; level_index++)
		{
			level_choice* level = data->levels + level_index;
			if (level->map_name == name)
			{
				level->completed = true;
				break;
			}
		}
	}
}

void mark_level_as_completed(static_game_data* data, char* name_buffer, i32 string_size)
{
	string_ref name = {};
	name.ptr = name_buffer;
	name.string_size = string_size;
	mark_level_as_completed(data, name);
}

void load_completed_levels(platform_api* platform, static_game_data* data)
{
	read_file_result prefs = platform->load_prefs();
	char* str = (char*)prefs.contents;

	// forma pliku: oddzielona przecinkami lista map bez rozszerzenia, np. map_01,map02

	char buffer[MAX_LEVEL_NAME_LENGTH] = {};
	u32 current_char_index = 0;
	while (*str)
	{
		if (current_char_index == MAX_LEVEL_NAME_LENGTH)
		{
			mark_level_as_completed(data, buffer, current_char_index);
			current_char_index = 0;
		}

		char c = *str;
		if (is_whitespace(c) || c == ',')
		{
			if (current_char_index > 0)
			{
				mark_level_as_completed(data, buffer, current_char_index);
				current_char_index = 0;
			}
		}
		else
		{
			buffer[current_char_index] = c;
			current_char_index++;
		}

		str++;
	}

	delete prefs.contents;
}