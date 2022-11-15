#include "level_initialization.h"
#include "entities.h"
#include "special_entities.h"
#include "level_parsing.h"
#include "progress.h"

void initialize_memory_for_checkpoint(game_state* game, memory_arena* arena)
{
    game->checkpoint.entities = push_array(arena, MAX_ENTITIES_COUNT, entity);
    game->checkpoint.entity_dynamic_types = push_array(arena, ENTITY_DYNAMIC_TYPES_MAX_COUNT, entity_type);
}

void initialize_level_state(level_state* level, static_game_data* static_data, string_ref map_name, memory_arena* arena)
{
    *level = (level_state){0};

    level->current_map_name = map_name;
    level->static_data = static_data;

    level->entities_count = 0;
    level->entities = push_array(arena, MAX_ENTITIES_COUNT, entity);
    level->entities_max_count = MAX_ENTITIES_COUNT;

    level->bullets_count = 0;
    level->bullets_max_count = MAX_BULLETS_COUNT; 
    level->bullets = push_array(arena, level->bullets_max_count, bullet);

    level->explosions_count = 0;
    level->explosions_max_count = MAX_EXPLOSIONS_COUNT;
    level->explosions = push_array(arena, level->explosions_max_count, entity);

    level->player_movement.current_mode = PLAYER_MOVEMENT_MODE_WALK;
    level->player_movement.standing_history.buffer_size = 10;
    level->player_movement.standing_history.buffer = push_array(arena,
        level->player_movement.standing_history.buffer_size, b32);

    level->fade_in_perc = static_data->game_fade_in_speed;

    level->gates_dict.entries = push_array(arena, MAX_GATES_COUNT, gate_dictionary_entry);
    level->gates_dict.entries_count = MAX_GATES_COUNT;

    level->entity_dynamic_types_count = 0;
    level->entity_dynamic_types = push_array(arena, ENTITY_DYNAMIC_TYPES_MAX_COUNT, entity_type);

    level->gate_tints_dict.sprite_effects = push_array(arena, MAX_GATES_COUNT, sprite_effect*);
    level->gate_tints_dict.sprite_effects_count = MAX_GATES_COUNT;
    level->gate_tints_dict.probing_jump = 7; 
}

void initialize_level_introduction(level_state* level, memory_arena* arena)
{
    level->show_level_introduction = false;
    level->introduction = (introduction_scene_state){0};

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
            level->static_data->entity_types_dict, ENTITY_TYPE_PLAYER);

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
        world_position starting_position = get_world_pos_from_tile_pos(level->current_map.starting_tile);
        starting_position = add_to_world_position(starting_position,
            get_v2(0, -(player_type->collision_rect_dim.y / 2) + 0.57f));

        add_entity_at_world_position(level, starting_position, player_type);
    }

    entity_to_spawn* new_entity = level->current_map.first_entity_to_spawn;
    while (new_entity)
    {
        switch (new_entity->type)
        {
            case ENTITY_TYPE_GATE_SILVER:
            case ENTITY_TYPE_GATE_GOLD:
            case ENTITY_TYPE_GATE_RED:
            case ENTITY_TYPE_GATE_GREEN:
            {
                add_gate_entity(level, arena, new_entity, false);
            }
            break;
            case ENTITY_TYPE_SWITCH_SILVER:
            case ENTITY_TYPE_SWITCH_GOLD:
            case ENTITY_TYPE_SWITCH_RED:
            case ENTITY_TYPE_SWITCH_GREEN:
            {
                add_gate_entity(level, arena, new_entity, true);
            }
            break;
            case ENTITY_TYPE_NEXT_LEVEL_TRANSITION:
            {
                add_next_level_transition_entity(level, new_entity);
            }
            break;
            case ENTITY_TYPE_MESSAGE_DISPLAY:
            {
                add_message_display_entity(level, new_entity);
            }
            break;
            case ENTITY_TYPE_CHECKPOINT:
            {
                add_checkpoint_entity(level, new_entity);
            }
            break;
            case ENTITY_TYPE_MOVING_PLATFORM_HORIZONTAL_SILVER:
            case ENTITY_TYPE_MOVING_PLATFORM_HORIZONTAL_GOLD:
            case ENTITY_TYPE_MOVING_PLATFORM_HORIZONTAL_RED:
            case ENTITY_TYPE_MOVING_PLATFORM_HORIZONTAL_GREEN:
            case ENTITY_TYPE_MOVING_PLATFORM_VERTICAL_SILVER:
            case ENTITY_TYPE_MOVING_PLATFORM_VERTICAL_GOLD:
            case ENTITY_TYPE_MOVING_PLATFORM_VERTICAL_RED:
            case ENTITY_TYPE_MOVING_PLATFORM_VERTICAL_GREEN:
            {
                add_moving_platform_entity(level, arena, new_entity);
            }
            break;
            case ENTITY_TYPE_UNKNOWN:
            {
                // ignorujemy
            }
            break;
            default:
            {
                entity_type* type = get_entity_type_ptr(
                    level->static_data->entity_types_dict, new_entity->type);
                world_position position = get_world_pos_from_tile_pos(new_entity->position);

                add_entity_at_world_position(level, position, type);

                if (new_entity->type == ENTITY_TYPE_ENEMY_MESSENGER
                    || new_entity->type == ENTITY_TYPE_ENEMY_YELLOW_FATHER)
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
    temporary_memory auxillary_memory_for_loading = begin_temporary_memory(game->transient_arena);
    {
        string_ref level_to_load_name = {0};
        if (game->cmd_level_to_load.string_size > 0)
        {
            level_to_load_name = game->cmd_level_to_load;
            game->cmd_level_to_load = (string_ref){0}; // ładujemy tylko raz
        }
        else if (scene_change.restore_checkpoint
            && game->checkpoint.used
            && game->checkpoint.map_name.string_size > 0)
        {
            level_to_load_name = game->checkpoint.map_name;
        }
        else if (scene_change.map_to_load.string_size)
        {
            level_to_load_name = scene_change.map_to_load;
        }

        if (level_to_load_name.string_size == 0)
        {
            level_to_load_name = copy_c_string(game->transient_arena, "map_01");
        }

        // nazwę przechowujemy poza pamięcią poziomu - potrzebne przy checkpointach
        copy_string_to_buffer(game->level_name_buffer, MAX_LEVEL_NAME_LENGTH, level_to_load_name);
        level_to_load_name = get_string_from_buffer(game->level_name_buffer, MAX_LEVEL_NAME_LENGTH);

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
            game->map_errors = (string_ref){0};
            game->level_state->current_map = parsing_result.parsed_map;
            initialize_current_map(game->level_state, game->arena);
            game->level_initialized = true;

            if (scene_change.restore_checkpoint && game->checkpoint.used)
            {
                restore_checkpoint(game->level_state, &game->checkpoint);
            }
            else
            {
                if (false == game->skip_introductions)
                {
                    // wstęp pokazujemy tylko za pierwszym razem
                    initialize_level_introduction(game->level_state, game->arena);
                }

                save_checkpoint(game->level_state, &game->checkpoint);
            }

            game->platform.start_playing_music(game->level_state->current_map.music_file_name);
        }

        end_temporary_memory(auxillary_memory_for_loading, true);
    }
}