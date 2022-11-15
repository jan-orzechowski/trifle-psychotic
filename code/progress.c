#include "progress.h"
#include "player.h"
#include <string.h>

void save_checkpoint(level_state* state, checkpoint* check)
{   
    assert(state->current_map_initialized);

    memcpy(check->entities, state->entities, sizeof(entity) * MAX_ENTITIES_COUNT);
    check->entities_count = state->entities_count;

    check->enemies_to_kill_counter = state->enemies_to_kill_counter;
    check->power_ups = state->power_ups;

    memcpy(check->gates_dict.entries, state->gates_dict.entries, sizeof(gate_dictionary_entry) * MAX_GATES_COUNT);
    check->gates_dict.entries_count = state->gates_dict.entries_count;

    // polegamy na tym, że string ref wskazuje na bufor w game_state, poza pamięcią poziomu
    check->map_name = state->current_map_name; 

    check->used = true;
}

void restore_checkpoint(level_state* state, checkpoint* check)
{
    assert(state->current_map_initialized);

    memcpy(state->entities, check->entities, sizeof(entity) * MAX_ENTITIES_COUNT);
    state->entities_count = check->entities_count;

    state->enemies_to_kill_counter = check->enemies_to_kill_counter;
    state->power_ups = check->power_ups;

    memcpy(state->gates_dict.entries, check->gates_dict.entries, sizeof(gate_dictionary_entry) * MAX_GATES_COUNT);
    state->gates_dict.entries_count = check->gates_dict.entries_count;

    // resetowanie wartości

    state->player_ignore_enemy_collision_cooldown = 0.0f;
    state->player_invincibility_cooldown = 0.0f;
    state->player_movement.recoil_acceleration = get_zero_v2();
    state->player_movement.recoil_acceleration_timer = 0.0f;
    state->player_movement.recoil_timer = 0.0f;    
    state->stop_player_movement = false;

    entity* player = get_player(state);
    player->health = player->type->max_health;
    player->acceleration = get_zero_v2();
    player->velocity = get_zero_v2();

    memset(state->bullets, 0, sizeof(bullet) * state->bullets_max_count);
    memset(state->explosions, 0, sizeof(explosion) * state->explosions_max_count);
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
            push_string_to_builder(&builder, level->map_name);
            push_char_to_builder(&builder, ',');
        }
    }

    string_ref text_to_save = get_string_from_string_builder(&builder);

    write_to_file save = {0};
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
            if (equals_string_ref(level->map_name, name))
            {
                level->completed = true;
                break;
            }
        }
    }
}

void mark_level_as_completed_from_buffer(static_game_data* data, char* name_buffer, i32 string_size)
{
    string_ref name = {0};
    name.ptr = name_buffer;
    name.string_size = string_size;
    mark_level_as_completed(data, name);
}

void load_completed_levels(platform_api* platform, static_game_data* data)
{
    // forma pliku: oddzielona przecinkami lista map bez rozszerzenia, np. map_01,map02
    read_file_result prefs = platform->load_prefs();
    if (prefs.contents != NULL)
    {
        char* str = (char*)prefs.contents;
        char buffer[MAX_LEVEL_NAME_LENGTH] = {0};
        u32 current_char_index = 0;
        while (*str)
        {
            if (current_char_index == MAX_LEVEL_NAME_LENGTH)
            {
                mark_level_as_completed_from_buffer(data, buffer, current_char_index);
                current_char_index = 0;
            }

            char c = *str;
            if (is_whitespace(c) || c == ',')
            {
                if (current_char_index > 0)
                {
                    mark_level_as_completed_from_buffer(data, buffer, current_char_index);
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

        free(prefs.contents);
    }
}