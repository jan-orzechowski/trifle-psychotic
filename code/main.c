#include "main.h"
#include "game_data.h"
#include "text_rendering.h"
#include "map.h"
#include "animation.h"
#include "special_entities.h"
#include "collision.h"
#include "entities.h"
#include "player.h"
#include "level_parsing.h"
#include "rendering.h"
#include "ui.h"
#include "input.h"
#include "debug.h"
#include "backdrops.h"
#include "progress.h"
#include "level_initialization.h"

scene_change game_update_and_render(game_state* game, r32 delta_time)
{	
    level_state* level = game->level_state;
    entity* player = get_player(level);
    game_input* input = get_last_frame_input(&game->input_buffer);

#if TRIFLE_DEBUG
    if (input->down.number_of_presses > 0)
    {
        level->enemies_to_kill_counter = -1;
    }
#endif

    if (level->show_message)
    {
        if (level->min_message_timer < 0.0f)
        {			
            if (level->show_exit_warning_message)
            {
                if (input->escape.number_of_presses > 0)
                {
                    if (false == level->active_scene_change.change_scene)
                    {
                        if (game->exit_level_closes_game)
                        {
                            level->active_scene_change.change_scene = true;
                            level->active_scene_change.new_scene = SCENE_EXIT;
                        }
                        else
                        {
                            level->active_scene_change.change_scene = true;
                            level->active_scene_change.new_scene = SCENE_MAIN_MENU;
                            game->platform.stop_playing_music(2000);
                        }						
                    }
                }
            }
            
            if (was_any_key_pressed_in_last_frames(&game->input_buffer, 1))
            {
                level->min_message_timer = 0.0f;
                level->show_message = false;
                level->show_exit_warning_message = false;
            }			
        }
        else
        {
            level->min_message_timer -= delta_time;
        }
    }
    
    if (input->escape.number_of_presses > 0)
    {
        if (false == level->show_message)
        {
            level->show_message = true;
            level->message_to_show = level->static_data->exit_warning_message;
            level->min_message_timer = 0.5f;
            level->messagebox_dimensions = get_v2(120, 60);
            level->show_exit_warning_message = true;
        }
    }

    // update player
    if (false == level->show_message)
    {
        if (player->health <= 0.0f)
        {
            if (false == level->active_scene_change.change_scene)
            {
                level->active_scene_change.change_scene = true;
                level->active_scene_change.new_scene = SCENE_DEATH;
                level->stop_player_movement = true;

                start_screen_shake(level, 0.6f, 30.0f);
                start_entity_death_animation(level, player);
                start_visual_effect_by_type(level, player, SPRITE_EFFECT_TYPE_DEATH);
                game->platform.stop_playing_music(2000);
            }
        }

        if (get_tile_pos_from_world_pos(player->position).y > level->current_map.height)
        {
            // the player has fallen out of the map
            level->active_scene_change.change_scene = true;
            level->active_scene_change.new_scene = SCENE_DEATH;
            level->stop_player_movement = true;
            game->platform.stop_playing_music(2000);
        }

        if (level->current_map.complete_when_all_messengers_killed
            && level->enemies_to_kill_counter <= 0)
        {
            mark_level_as_completed(level->static_data, level->current_map_name);
            save_completed_levels(&game->platform, level->static_data, game->transient_arena);

            level->active_scene_change.change_scene = true;
            level->stop_player_movement = true;
            level->show_victory_message = true;

            if (level->current_map.next_map.string_size == 0)
            {
                level->active_scene_change.new_scene = SCENE_LEVEL_CHOICE;
            }
            else
            {
                level->active_scene_change.new_scene = SCENE_GAME;
                level->active_scene_change.map_to_load = level->current_map.next_map;
            }

            game->platform.stop_playing_music(2000);
        }

        if (level->player_invincibility_cooldown > 0.0f)
        {
            level->player_invincibility_cooldown -= delta_time;
        }

        if (level->player_ignore_enemy_collision_cooldown > 0.0f)
        {
            level->player_ignore_enemy_collision_cooldown -= delta_time;
        }

        update_power_up_timers(level, delta_time);
        animate_entity(&level->player_movement, player, delta_time, 1.0f);

        world_position target_pos = process_input(level, &game->input_buffer, player, delta_time);

        collision_result collision = move(level, player, target_pos);
        if (collision.collided_power_up)
        {
            apply_power_up(level, player, collision.collided_power_up);
        }

        if (collision.collided_enemy)
        {
            handle_player_and_enemy_collision(level, player, collision.collided_enemy);
        }
    
        if (collision.collided_switch)
        {
            handle_player_and_switch_collision(level, collision);
        }

        if (collision.collided_message_display)
        {
            string_ref message = collision.collided_message_display->type->message;
            if (message.string_size)
            {
                level->show_message = true;
                level->message_to_show = collision.collided_message_display->type->message;
                level->min_message_timer = 1.0f;
                level->messagebox_dimensions = get_v2(150, 130);
                collision.collided_message_display->type->type_enum = ENTITY_TYPE_UNKNOWN; // we show the message only once
            }
        }

        if (collision.collided_transition 
            && false == level->active_scene_change.change_scene)
        {
            mark_level_as_completed(level->static_data, level->current_map_name);
            save_completed_levels(&game->platform, level->static_data, game->transient_arena);

            level->active_scene_change.change_scene = true;
            level->active_scene_change.new_scene = SCENE_GAME;
            level->active_scene_change.map_to_load = level->current_map.next_map;
        }

        if (collision.collided_checkpoint)
        {
            collision.collided_checkpoint->type->type_enum = ENTITY_TYPE_UNKNOWN; // checkpoint works only once
            save_checkpoint(level, &game->checkpoint);
            level->show_checkpoint_message_timer = 4.0f;
        }

        if (false == is_zero_v2(player->velocity))
        {
            player->direction = player->velocity.x < 0 ? DIRECTION_W : DIRECTION_E;
        }

        if (is_power_up_active(level->power_ups.invincibility))
        {
            start_visual_effect_by_type(level, player, SPRITE_EFFECT_TYPE_INVINCIBILITY);
        }
        else 
        {
            stop_visual_effect_by_type(level, player, SPRITE_EFFECT_TYPE_INVINCIBILITY);
        }

        if (is_power_up_active(level->power_ups.speed))
        {
            start_visual_effect_by_type(level, player, SPRITE_EFFECT_TYPE_SPEED);
        }
        else
        {
            stop_visual_effect_by_type(level, player, SPRITE_EFFECT_TYPE_SPEED);
        }
    }
    
    update_backdrops_movement(&level->current_map.first_backdrop, &level->current_map.first_backdrop_offset, player, delta_time);
    update_backdrops_movement(&level->current_map.second_backdrop, &level->current_map.second_backdrop_offset, player, delta_time);

    if (false == level->show_message)
    {
        // update entities
        for (i32 entity_index = 1; entity_index < level->entities_count; entity_index++)
        {
            entity* entity = level->entities + entity_index;

            if (false == entity->used)
            {
                continue;
            }
        
            if (false == is_in_neighbouring_chunk(player->position.chunk_pos, entity->position))
            {
                continue;
            }

            if (entity->health < 0.0f)
            {
                if (entity->type->type_enum == ENTITY_TYPE_ENEMY_MESSENGER
                    || entity->type->type_enum == ENTITY_TYPE_ENEMY_YELLOW_FATHER)
                {
                    level->enemies_to_kill_counter--;
                }

                start_entity_death_animation(level, entity);
                remove_entity(entity);
                continue;
            }
            
            if (is_entity_moving_type_entity(entity))
            {				
                process_entity_movement(level, entity, player, delta_time);				
            }
            else if (has_entity_flags_set(entity, ENTITY_FLAG_ENEMY))
            {
                enemy_attack(level, entity, player, delta_time);

                if (entity->player_detected)
                {
                    set_entity_rotated_graphics(entity, &player->position);
                
                    v2 distance_to_player = get_world_pos_diff(player->position, entity->position);
                    r32 distance_to_player_length = length_v2(distance_to_player);
                    if (distance_to_player_length > entity->type->forget_detection_distance)
                    {
                        entity->player_detected = false;
                    }
                }
                else
                {
                    // setting the initial graphics, before the player is detected for the first time
                    if (entity->shooting_sprite.parts_count == 0)
                    {
                        set_entity_rotated_graphics(entity, NULL);
                    }
                }
            }

            r32 frame_duration_modifier = 0.75f + (1.0f / length_v2(entity->velocity));
            animate_entity(NULL, entity, delta_time, frame_duration_modifier);
        }

        // update bullets
        for (i32 bullet_index = 0; bullet_index < level->bullets_count; bullet_index++)
        {
            bullet* bullet = level->bullets + bullet_index;

            if (is_in_neighbouring_chunk(player->position.chunk_pos, bullet->position))
            {
                if (bullet->type)
                {
                    world_position bullet_target_pos = add_to_world_pos(bullet->position, 
                        multiply_v2(bullet->velocity, delta_time));
                    b32 hit = move_bullet(level, bullet, bullet_target_pos);
                    if (hit)
                    {
                        remove_bullet(level, &bullet_index);
                    }
                }
            }
            else
            {
                remove_bullet(level, &bullet_index);
            }
        }

        // update explosions
        for (i32 explosion_index = 0; explosion_index < level->explosions_count; explosion_index++)
        {
            entity* explosion = level->explosions + explosion_index;
            if ((explosion->animation_duration + delta_time) > explosion->current_animation->total_duration)
            {
                remove_explosion(level, &explosion_index);
            }
            else
            {
                animate_entity(NULL, explosion, delta_time, 1.0f);
            }
        }
    }

    // rendering
    {		
        world_position camera_position = player->position;

        if (level->screen_shake_duration > 0.0f)
        {
            level->screen_shake_duration -= delta_time;
            r32 vertical_shake = sin(level->screen_shake_duration * level->screen_shake_multiplier) / 2;
            camera_position = add_to_world_pos(camera_position, get_v2(vertical_shake, 0.0f));
        }

        chunk_position reference_chunk = camera_position.chunk_pos;
        tile_position camera_tile_pos = get_tile_pos_from_world_pos(camera_position);
        v2 camera_tile_offset_in_chunk = get_tile_offset_in_chunk(reference_chunk, camera_tile_pos);
        v2 camera_offset_in_chunk = get_world_pos_and_chunk_pos_diff(camera_position, reference_chunk);
        v2 camera_offset_in_tile = subtract_v2(camera_offset_in_chunk, camera_tile_offset_in_chunk);
    
        render_backdrops(&game->render, level, camera_position);

        render_map_layer(&game->render, level->current_map.background, camera_tile_pos, camera_offset_in_tile);
        render_map_layer(&game->render, level->current_map.map, camera_tile_pos, camera_offset_in_tile);
        render_map_layer(&game->render, level->current_map.foreground, camera_tile_pos, camera_offset_in_tile);

#if TRIFLE_DEBUG
#if TRIFLE_DEBUG_COLLISION
        debug_render_tile_collision_boxes(&game->render, level, camera_position);
#endif
#endif

        // draw gates and switches
        for (i32 entity_index = 0; entity_index < level->entities_count; entity_index++)
        {
            entity* entity = level->entities + entity_index;
            if (false == entity->used)
            {
                continue;
            }

            if (has_entity_flags_set(entity, ENTITY_FLAG_GATE)
                || has_entity_flags_set(entity, ENTITY_FLAG_SWITCH))
            {
                render_entity_animation_frame(&game->render, camera_position, entity);
            }
        }

        // draw other entities
        for (i32 entity_index = 0; entity_index < level->entities_count; entity_index++)
        {
            entity* entity = level->entities + entity_index;
            if (false == entity->used)
            {
                continue;
            }

            if (has_entity_flags_set(entity, ENTITY_FLAG_GATE)
                || has_entity_flags_set(entity, ENTITY_FLAG_SWITCH))
            {
                continue;
            }

            if (is_in_neighbouring_chunk(camera_position.chunk_pos, entity->position))
            {
                render_entity_animation_frame(&game->render, camera_position, entity);

                if (entity->shooting_sprite.parts_count)
                {
                    render_entity_sprite(&game->render, camera_position, entity->position, entity->direction,
                        entity->visual_effect, entity->visual_effect_duration, entity->shooting_sprite);
                }
            }
        }

        // draw bullets
        for (i32 bullet_index = 0; bullet_index < level->bullets_count; bullet_index++)
        {
            bullet* bullet = level->bullets + bullet_index;
            render_entity_sprite(&game->render,
                camera_position, bullet->position, DIRECTION_NONE,
                NULL, 0, bullet->type->idle_pose.sprite);            
        }

        // draw explosions
        for (i32 explosion_index = 0; explosion_index < level->explosions_count; explosion_index++)
        {
            entity* explosion = level->explosions + explosion_index;
            render_entity_animation_frame(&game->render, camera_position, explosion);           
        }

#if TRIFLE_DEBUG
#if TRIFLE_DEBUG_COLLISION
        debug_render_entity_collision_boxes(&game->render, level, camera_position);
        debug_render_bullet_collision_boxes(&game->render, level, camera_position);
#endif
        debug_render_player_information(game, level);
#endif

        render_hitpoint_bar(level->static_data, &game->render, player, is_power_up_active(level->power_ups.invincibility));

        if (level->current_map.complete_when_all_messengers_killed)
        {
            render_counter(level->static_data, &game->render, game->transient_arena, level->enemies_to_kill_counter, 99);
        }
    }

    if (false == level->stop_player_movement)
    {
        render_crosshair(level->static_data, &game->render, input);
    }

    update_and_render_message_box(&game->render, level, game->transient_arena, delta_time);

    if (level->show_victory_message)
    {
        render_victory_text(&game->render, game->transient_arena, level->static_data);
    }
    else if (level->show_checkpoint_message_timer > 0.0f)
    {
        render_checkpoint_text(&game->render, game->transient_arena, level->static_data);
        level->show_checkpoint_message_timer -= delta_time;
    }

    if (level->active_scene_change.change_scene)
    {
        r32 fade_out_speed = level->show_victory_message
            ? level->static_data->game_victory_fade_out_speed
            : level->static_data->game_fade_out_speed;

        process_fade(&game->render, &level->fade_out_perc, delta_time, 
            false, fade_out_speed);
    }

    scene_change scene_change = {0}; 
    if (level->fade_out_perc >= 1.0f)
    {
        scene_change = level->active_scene_change;
    }

    if (level->active_scene_change.change_scene
        && game->exit_level_closes_game)
    {
        // we exit without waiting for fade
        scene_change = level->active_scene_change;
    }

    if (level->active_scene_change.change_scene
        && level->active_scene_change.new_scene == SCENE_DEATH
        && game->skip_deaths_prompts)
    {
        scene_change.new_scene = SCENE_GAME;
        scene_change.restore_checkpoint = true;
    }

    process_fade(&game->render, &level->fade_in_perc, delta_time, 
        true, level->static_data->game_fade_in_speed);

    return scene_change;
}

void level_introduction_update_and_render(game_state* game, r32 delta_time)
{
    level_state* level = game->level_state;
    world_position camera_position = get_world_pos_from_tile_pos(get_tile_pos(10, 10));
    render_backdrops(&game->render, level, camera_position);
    render_rectangle(&game->render, get_whole_screen_rect(), get_v4(0.0f, 0.0f, 0.0f, 0.3f), false);

    if (level->introduction.fade_in_perc == 0.0f)
    {
        render_large_text(&game->render, &level->static_data->scrolling_text_options,
            *level->current_map.introduction_lines, level->introduction.text_y_offset);

        level->introduction.text_y_offset -= delta_time * level->static_data->introduction_text_speed;
    }

    if (level->introduction.can_be_skipped_timer > 0.0f)
    {
        level->introduction.can_be_skipped_timer -= delta_time;
        if (level->introduction.can_be_skipped_timer <= 0.0f)
        {
            level->introduction.can_be_skipped = true;
        }
    }

    if (level->introduction.can_be_skipped)
    {
        v2 dots_indicator_position = get_v2(
            SCREEN_WIDTH / SCALING_FACTOR / 2, 
            (SCREEN_HEIGHT / SCALING_FACTOR) - 20.0f);
        update_and_render_skippable_indicator(&game->render, level->static_data,
            &level->introduction.skippable_indicator_timer,
            &level->introduction.skippable_indicator_index,
            delta_time, dots_indicator_position);

        if (was_any_key_pressed_in_last_frames(&game->input_buffer, 1))
        {
            level->introduction.skipped = true;
        }
    }

    if (level->introduction.skipped == true)
    {
        process_fade(&game->render, &level->introduction.fade_out_perc, delta_time, 
            false, level->static_data->introduction_fade_speed);
    }

    if (level->introduction.fade_out_perc >= 1.0f)
    {
        level->show_level_introduction = false;
    }

    process_fade(&game->render, &level->introduction.fade_in_perc, delta_time, 
        true, level->static_data->introduction_fade_speed);
}

scene_change level_choice_update_and_render(game_state* game, r32 delta_time)
{
    static_game_data* static_data = game->static_data;
    game_input* input = get_last_frame_input(&game->input_buffer);

    if (game->level_choice_menu.time_to_first_interaction > 0.0f)
    {
        game->level_choice_menu.time_to_first_interaction -= delta_time;
    }

    i32 options_x = 80;
    i32 option_y = 100;
    i32 option_y_spacing = 20;

    rect message_area = get_rect_from_corners(get_v2(30, 50), get_v2(260, 100));
    render_text_basic(&game->render, game->transient_arena, static_data->ui_font,
        message_area, static_data->choose_level_message, false);
    
    rect option_rect = get_rect_from_corners(
        get_v2(options_x, option_y),
        get_v2(options_x + 165, option_y + 10));

    for (u32 level_index = 0; level_index < static_data->levels_count; level_index++)
    {
        level_choice level = static_data->levels[level_index];

        set_rect_length_to_fit_text(&option_rect, static_data->ui_font, level.name);
        render_menu_option(static_data->ui_font, game, option_rect, level.name, level.completed);
        if (game->level_choice_menu.time_to_first_interaction <= 0.0f
            && was_rect_clicked(input, option_rect))
        {
            game->level_choice_menu.active_scene_change.change_scene = true;
            game->level_choice_menu.active_scene_change.new_scene = SCENE_GAME;
            game->level_choice_menu.active_scene_change.restore_checkpoint = false;
            game->level_choice_menu.active_scene_change.map_to_load = level.map_name;
        }

        move_rect_in_place(&option_rect, get_v2(0.0f, option_y_spacing));
    }

    render_crosshair(static_data, &game->render, input);

    if (game->level_choice_menu.active_scene_change.change_scene)
    {
        process_fade(&game->render, &game->level_choice_menu.fade_out_perc, delta_time, 
            false, static_data->menu_fade_speed);
    }
    else
    {
        process_fade(&game->render, &game->level_choice_menu.fade_in_perc, delta_time, 
            true, static_data->menu_fade_speed);
    }

    if (game->level_choice_menu.time_to_first_interaction <= 0.0f
        && input->escape.number_of_presses > 0)
    {
        game->level_choice_menu.active_scene_change.change_scene = true;
        game->level_choice_menu.active_scene_change.new_scene = SCENE_MAIN_MENU;
    }

    scene_change scene_change = {0};
    if (game->level_choice_menu.fade_out_perc >= 1.0f)
    {
        scene_change = game->level_choice_menu.active_scene_change;
    }

    return scene_change;
}

scene_change menu_update_and_render(game_state* game, r32 delta_time)
{
    static_game_data* static_data = game->static_data;
    game_input* input = get_last_frame_input(&game->input_buffer);

    if (game->main_menu.time_to_first_interaction > 0.0f)
    {
        game->main_menu.time_to_first_interaction -= delta_time;
    }

    render_bitmap(&game->render, TEXTURE_BACKGROUND_TITLE_SCREEN,
        get_rect_from_corners(get_v2(0, 0), get_v2(384, 320)),
        get_whole_screen_rect());

    rect title_area = get_rect_from_corners(
        get_v2(30, 20),
        get_v2(300, 100));

    render_text_basic(&game->render, game->transient_arena, static_data->title_font,
        title_area, static_data->title_str, true);
    
    i32 options_x = 140;
    i32 option_y = 140;
    i32 option_y_spacing = 20;

    rect option_interactive_rect = render_menu_option_at_coords(static_data->ui_font, game,
        options_x, option_y, static_data->menu_new_game_str, false);
    if (game->main_menu.time_to_first_interaction <= 0.0f
        && was_rect_clicked(input, option_interactive_rect))
    {
        game->main_menu.active_scene_change.change_scene = true;
        game->main_menu.active_scene_change.new_scene = SCENE_LEVEL_CHOICE;
    }

    if (game->checkpoint.used)
    {
        option_y += option_y_spacing;	
        option_interactive_rect = render_menu_option_at_coords(static_data->ui_font, game,
            options_x, option_y, static_data->menu_continue_str, false);
        if (game->main_menu.time_to_first_interaction <= 0.0f
            && was_rect_clicked(input, option_interactive_rect))
        {
            game->main_menu.active_scene_change.change_scene = true;
            game->main_menu.active_scene_change.new_scene = SCENE_GAME;
            game->main_menu.active_scene_change.restore_checkpoint = true;
        }
    }

    option_y += option_y_spacing;
    option_interactive_rect = render_menu_option_at_coords(static_data->ui_font, game,
        options_x, option_y, static_data->menu_credits_str, false);
    if (game->main_menu.time_to_first_interaction <= 0.0f
        && was_rect_clicked(input, option_interactive_rect))
    {
        game->main_menu.active_scene_change.change_scene = true;
        game->main_menu.active_scene_change.new_scene = SCENE_CREDITS;
    }

    if (game->show_exit_game_option)
    {
        option_y += option_y_spacing;
        option_interactive_rect = render_menu_option_at_coords(static_data->ui_font, game,
            options_x, option_y, static_data->menu_exit_str, false);
        if (game->main_menu.time_to_first_interaction <= 0.0f
            && was_rect_clicked(input, option_interactive_rect))
        {
            game->main_menu.active_scene_change.change_scene = true;
            game->main_menu.active_scene_change.new_scene = SCENE_EXIT;
        }
    }

    render_crosshair(static_data, &game->render, input);

    if (game->main_menu.active_scene_change.change_scene)
    {
        process_fade(&game->render, &game->main_menu.fade_out_perc, delta_time, 
            false, static_data->menu_fade_speed);
    }
    else
    {
        process_fade(&game->render, &game->main_menu.fade_in_perc, delta_time, 
            true, static_data->menu_fade_speed);
    }
    
    scene_change scene_change = {0};
    if (game->main_menu.fade_out_perc >= 1.0f)
    {
        scene_change = game->main_menu.active_scene_change;
    }
    return scene_change;
};

scene_change credits_screen_update_and_render(game_state* game, r32 delta_time)
{
    static_game_data* static_data = game->static_data;
    scene_change change_to_other_scene = {0};

    text_lines* text_to_show;
    if (game->credits_screen.ending_text_mode)
    {
        text_to_show = static_data->ending_text_lines;
    }
    else
    {
        text_to_show = static_data->credits_text_lines;
    }

    render_bitmap(&game->render, TEXTURE_BACKGROUND_TITLE_SCREEN,
        get_rect_from_corners(get_v2(0, 0), get_v2(384, 320)),
        get_whole_screen_rect());
    render_rectangle(&game->render, get_whole_screen_rect(), get_v4(0.0f, 0.0f, 0.0f, 0.3f), false);

    if (game->credits_screen.fade_in_perc == 0.0f)
    {
        render_large_text(&game->render, &static_data->scrolling_text_options,
            *text_to_show, game->credits_screen.text_y_offset);

        game->credits_screen.text_y_offset -= delta_time * static_data->credits_text_speed;
    }

    b32 can_exit = false;
    if (game->credits_screen.ending_text_mode)
    {
        if (game->credits_screen.text_y_offset < (-SCREEN_HEIGHT) / 2)
        {
            can_exit = true;
        }
    }
    else
    {
        if (game->credits_screen.time_to_first_interaction > 0.0f)
        {
            game->credits_screen.time_to_first_interaction -= delta_time;
        }
        else
        {
            can_exit = true;
        }
    }
        
    if (can_exit)
    {
        v2 dots_indicator_position = get_v2(
            SCREEN_WIDTH / SCALING_FACTOR / 2,
            (SCREEN_HEIGHT / SCALING_FACTOR) - 20.0f);
        update_and_render_skippable_indicator(&game->render, static_data,
            &game->credits_screen.skippable_indicator_timer,
            &game->credits_screen.skippable_indicator_index,
            delta_time, dots_indicator_position);

        if (was_any_key_pressed_in_last_frames(&game->input_buffer, 1))
        {
            game->credits_screen.transition_to_main_menu = true;
        }
    }

    if (game->credits_screen.transition_to_main_menu)
    {
        process_fade(&game->render, &game->credits_screen.fade_out_perc, delta_time, 
            false, static_data->credits_screen_fade_speed);

        if (game->credits_screen.fade_out_perc >= 1.0f)
        {
            change_to_other_scene.change_scene = true;
            change_to_other_scene.new_scene = SCENE_MAIN_MENU;
        }
    }

    process_fade(&game->render, &game->credits_screen.fade_in_perc, delta_time,
        true, static_data->credits_screen_fade_speed);

    return change_to_other_scene;
}

scene_change death_screen_update_and_render(game_state* game, r32 delta_time)
{
    static_game_data* static_data = game->static_data;
    scene_change change_to_other_scene = {0};

    if (false == game->death_screen.initialized)
    {
        string_ref death_screen_prompt = static_data->default_death_message;
        i32 text_choice = rand() % 2;
        if (text_choice == 0)
        {
            text_choice = rand() % static_data->death_messages_count;
            death_screen_prompt = static_data->death_messages[text_choice];
        }

        game->death_screen.timer = 1.0f;
        game->death_screen.prompt = death_screen_prompt;
        game->death_screen.fade_in_perc = 1.0f;
        game->death_screen.initialized = true;
    }
    
    rect prompt_area = get_whole_screen_text_area(50.0f);
    render_text_basic(&game->render, game->transient_arena, static_data->ui_font,
        prompt_area, game->death_screen.prompt, true);

    if (game->death_screen.timer > 0.0f)
    {
        game->death_screen.timer -= delta_time;
    }
    else
    {
        v2 dots_indicator_position = get_v2(
            SCREEN_WIDTH / SCALING_FACTOR / 2,
            (SCREEN_HEIGHT / SCALING_FACTOR) - 20.0f);
        update_and_render_skippable_indicator(&game->render, static_data,
            &game->death_screen.skippable_indicator_timer,
            &game->death_screen.skippable_indicator_index,
            delta_time, dots_indicator_position);

        if (was_any_key_pressed_in_last_frames(&game->input_buffer, 1))
        {
            game->death_screen.transition_to_game = true;
        }
    }

    if (game->death_screen.transition_to_game)
    {		
        process_fade(&game->render, &game->death_screen.fade_out_perc, delta_time, 
            false, static_data->death_screen_fade_speed);

        if (game->death_screen.fade_out_perc >= 1.0f)
        {
            change_to_other_scene.change_scene = true;
            change_to_other_scene.new_scene = SCENE_GAME;
            change_to_other_scene.restore_checkpoint = true;
        }
    }

    process_fade(&game->render, &game->death_screen.fade_in_perc, delta_time, 
        true, static_data->death_screen_fade_speed);

    return change_to_other_scene;
}

scene_change map_errors_screen_update_and_render(game_state* game)
{
    scene_change change_to_other_scene = {0};

    if (game->map_errors.string_size > 0)
    {
        render_text(&game->render, game->transient_arena,
            &game->static_data->parsing_errors_text_options, game->map_errors);

        if (was_any_key_pressed_in_last_frames(&game->input_buffer, 1))
        {
            change_to_other_scene.change_scene = true;
            change_to_other_scene.new_scene = SCENE_MAIN_MENU;
            game->main_menu = (main_menu_state){0};
        }
    }
    else
    {
        change_to_other_scene.change_scene = true;
        change_to_other_scene.new_scene = SCENE_MAIN_MENU;
        game->main_menu = (main_menu_state){0};
    }

    return change_to_other_scene;
}

void main_game_loop(game_state* game, r32 delta_time)
{
    scene_change scene_change = {0};

    if (game->current_scene == SCENE_GAME && game->map_errors.string_size > 0)
    {
        game->current_scene = SCENE_MAP_ERRORS;
    }

    switch (game->current_scene)
    {
        case SCENE_GAME:
        {
            if (false == game->level_initialized)
            {
                scene_change.change_scene = true;
                scene_change.new_scene = SCENE_GAME;			
            }
            else
            {
                if (game->level_state->show_level_introduction)
                {
                    level_introduction_update_and_render(game, delta_time);
                }
                else
                {
                    scene_change = game_update_and_render(game, delta_time);
                }
            }			
        };
        break;
        case SCENE_MAIN_MENU:
        {
            scene_change = menu_update_and_render(game, delta_time);
        };
        break;
        case SCENE_LEVEL_CHOICE:
        {
            scene_change = level_choice_update_and_render(game, delta_time);
        };
        break;
        case SCENE_MAP_ERRORS:
        {
            scene_change = map_errors_screen_update_and_render(game);
        };
        break;
        case SCENE_DEATH:
        {
            scene_change = death_screen_update_and_render(game, delta_time);
        };
        break;
        case SCENE_CREDITS:
        {
            scene_change = credits_screen_update_and_render(game, delta_time);
        };
        break;
    }

    if (scene_change.change_scene)
    {
        scene previous_scene = game->current_scene;
        game->current_scene = scene_change.new_scene;
        switch (scene_change.new_scene)
        {
            case SCENE_GAME:
            {				
                change_and_initialize_level(game, scene_change);
            }
            break;
            case SCENE_MAIN_MENU:
            {
                game->main_menu = (main_menu_state){0};
                game->main_menu.time_to_first_interaction = game->static_data->default_time_to_first_menu_interaction;
                game->main_menu.fade_in_perc = 1.0f;
            }
            break;
            case SCENE_LEVEL_CHOICE:
            {                
                if (previous_scene == SCENE_GAME 
                    && check_if_all_levels_are_completed(game->static_data))
                {
                    game->current_scene = SCENE_CREDITS;
                    game->credits_screen = (credits_screen_state){ 0 };
                    game->credits_screen.time_to_first_interaction = game->static_data->default_time_to_first_menu_interaction;
                    game->credits_screen.fade_in_perc = 1.0f;
                    game->credits_screen.ending_text_mode = true;
                    game->credits_screen.text_y_offset = (SCREEN_HEIGHT / SCALING_FACTOR);
                }
                else
                {
                    game->level_choice_menu = (level_choice_menu_state){ 0 };
                    game->level_choice_menu.time_to_first_interaction = game->static_data->default_time_to_first_menu_interaction;
                    game->level_choice_menu.fade_in_perc = 1.0f;

                    load_completed_levels(&game->platform, game->static_data);
                }              
            }
            break;
            case SCENE_MAP_ERRORS:
            {
                // no initialization needed
            }
            break;
            case SCENE_DEATH:
            {
                game->death_screen = (death_screen_state){0};
            }
            break;
            case SCENE_CREDITS:
            {
                game->credits_screen = (credits_screen_state){0};
                game->credits_screen.time_to_first_interaction = game->static_data->default_time_to_first_menu_interaction;
                game->credits_screen.fade_in_perc = 1.0f;
                game->credits_screen.text_y_offset = (SCREEN_HEIGHT / SCALING_FACTOR);
            }
            break;
            case SCENE_EXIT:
            {
                game->exit_game = true;
            }
            break;
        }
    }

    game->platform.render_list_to_output(&game->render);
}