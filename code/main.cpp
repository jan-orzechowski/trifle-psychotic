#include "main.h"
#include "game_data.h"
#include "text_rendering.h"
#include "map.h"
#include "animation.h"
#include "gates.h"
#include "collision.h"
#include "entities.h"
#include "player.h"
#include "level_parsing.h"
#include "rendering.h"
#include "ui.h"
#include "input.h"
#include "sdl_platform.h"
#include "debug.h"
#include "backdrops.h"

void save_checkpoint(game_state* game)
{
	assert(game->level_initialized);	
	assert(game->level_state->current_map_initialized);	

	game->checkpoint = {};
	game->checkpoint.used = true;
	game->checkpoint.map_name = copy_string_to_buffer(game->level_name_buffer, MAX_LEVEL_NAME_LENGTH,
		game->level_state->current_map_name);

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

scene_change game_update_and_render(game_state* game, level_state* level, r32 delta_time)
{	
	entity* player = get_player(level);
	game_input* input = get_last_frame_input(&game->input_buffer);

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
						level->active_scene_change.change_scene = true;
						level->active_scene_change.new_scene = scene::MAIN_MENU;
						level->active_scene_change.fade_out_speed = 0.5f;
						stop_playing_music(2000);
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
			// przegrywamy
			if (false == level->active_scene_change.change_scene)
			{
				level->active_scene_change.change_scene = true;
				level->active_scene_change.new_scene = scene::DEATH;
				level->active_scene_change.fade_out_speed = 0.5f;
				level->stop_player_movement = true;

				start_screen_shake(level, 0.6f, 30.0f);
				start_death_animation(level, player);
				start_visual_effect(level, player, sprite_effects_types::DEATH);
				stop_playing_music(2000);
			}
		}

		if (get_tile_position(player->position).y > level->current_map.height)
		{
			// przegrywamy - spadliśmy z mapy
			level->active_scene_change.change_scene = true;
			level->active_scene_change.new_scene = scene::DEATH;
			level->active_scene_change.fade_out_speed = 0.5f;
			level->stop_player_movement = true;
			stop_playing_music(2000);
		}

		if (level->current_map.complete_when_all_messengers_killed
			&& level->enemies_to_kill_counter <= 0)
		{
			mark_level_as_completed(level->static_data, level->current_map_name);
			save_completed_levels(level->static_data, game->transient_arena);

			level->active_scene_change.change_scene = true;
			level->active_scene_change.fade_out_speed = 1.5f;
			level->stop_player_movement = true;
			
			if (level->current_map.next_map.string_size == 0)
			{
				level->active_scene_change.new_scene = scene::LEVEL_CHOICE;
			}
			else
			{
				level->active_scene_change.new_scene = scene::GAME;
				level->active_scene_change.map_to_load = level->current_map.next_map;
			}

			stop_playing_music(2000);
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
		animate_entity(&level->player_movement, player, delta_time);

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
				level->messagebox_dimensions = get_v2(150, 120);
				collision.collided_message_display->type->message = {};
			}
		}

		if (collision.collided_transition 
			&& false == level->active_scene_change.change_scene)
		{
			mark_level_as_completed(level->static_data, level->current_map_name);
			save_completed_levels(level->static_data, game->transient_arena);

			level->active_scene_change.change_scene = true;
			level->active_scene_change.new_scene = scene::GAME;
			level->active_scene_change.map_to_load = level->current_map.next_map;
			level->active_scene_change.fade_out_speed = 1.5f;
		}

		v2 player_direction_v2 = get_unit_vector(player->velocity);
		if (false == is_zero(player->velocity))
		{
			player->direction = player->velocity.x < 0 ? direction::W : direction::E;
		}

		if (is_power_up_active(level->power_ups.invincibility))
		{
			start_visual_effect(level, player, sprite_effects_types::INVINCIBILITY);
		}
		else 
		{
			stop_visual_effect(level, player, sprite_effects_types::INVINCIBILITY);
		}

		if (is_power_up_active(level->power_ups.speed))
		{
			start_visual_effect(level, player, sprite_effects_types::SPEED);
		}
		else
		{
			stop_visual_effect(level, player, sprite_effects_types::SPEED);
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
				if (entity->type->type_enum == entity_type_enum::ENEMY_CULTIST)
				{
					level->enemies_to_kill_counter--;
				}

				start_death_animation(level, entity);
				remove_entity(entity);
				continue;
			}
			
			if (is_entity_moving_type_entity(entity))
			{				
				process_entity_movement(level, entity, player, delta_time);				
			}
			else if (are_entity_flags_set(entity, entity_flags::ENEMY))
			{
				enemy_attack(level, entity, player, delta_time);

				if (entity->player_detected)
				{
					set_entity_rotated_graphics(entity, &player->position);
				
					v2 distance_to_player = get_position_difference(player->position, entity->position);
					r32 distance_to_player_length = length(distance_to_player);
					if (distance_to_player_length > entity->type->forget_detection_distance)
					{
						entity->player_detected = false;
					}
				}
				else
				{
					// dla ustawienia początkowej grafiki, zanim gracz zostanie wykryty
					if (entity->shooting_sprite.parts_count == 0)
					{
						set_entity_rotated_graphics(entity, NULL);
					}
				}
			}

			r32 frame_duration_modifier = 0.75f + (1.0f / length(entity->velocity));
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
					world_position bullet_target_pos = add_to_position(bullet->position, bullet->velocity * delta_time);
					b32 hit = move_bullet(level, bullet, bullet_index, bullet_target_pos);
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
				animate_entity(NULL, explosion, delta_time);
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
			camera_position = add_to_position(camera_position, get_v2(vertical_shake, 0.0f));
		}

		chunk_position reference_chunk = camera_position.chunk_pos;
		tile_position camera_tile_pos = get_tile_position(camera_position);
		v2 camera_tile_offset_in_chunk = get_tile_offset_in_chunk(reference_chunk, camera_tile_pos);
		v2 camera_offset_in_chunk = get_position_difference(camera_position, reference_chunk);
		v2 camera_offset_in_tile = camera_offset_in_chunk - camera_tile_offset_in_chunk;
	
		render_backdrops(&game->render, level, camera_position);

		render_map_layer(&game->render, level, level->current_map.background, camera_tile_pos, camera_offset_in_tile);
		render_map_layer(&game->render, level, level->current_map.map, camera_tile_pos, camera_offset_in_tile);
		
#if TRIFLE_DEBUG
		debug_render_tile_collision_boxes(&game->render, level, camera_position);
#endif

		// draw entities
		for (i32 entity_index = 0; entity_index < level->entities_count; entity_index++)
		{
			entity* entity = level->entities + entity_index;
			if (false == entity->used)
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
			if (is_in_neighbouring_chunk(camera_position.chunk_pos, bullet->position))
			{
				render_entity_sprite(&game->render,
					camera_position, bullet->position, direction::NONE,
					NULL, 0, bullet->type->idle_pose.sprite);
			}
		}

		// draw explosions
		for (i32 explosion_index = 0; explosion_index < level->explosions_count; explosion_index++)
		{
			entity* explosion = level->explosions + explosion_index;
			if (is_in_neighbouring_chunk(camera_position.chunk_pos, explosion->position))
			{
				render_entity_animation_frame(&game->render, camera_position, explosion);
			}
		}

		render_map_layer(&game->render, level, level->current_map.foreground, camera_tile_pos, camera_offset_in_tile);

#if TRIFLE_DEBUG

		debug_render_entity_collision_boxes(&game->render, level, camera_position);
		debug_render_bullet_collision_boxes(&game->render, level, camera_position);
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

	if (level->show_message && level->message_to_show.string_size)
	{
		if (is_zero(level->messagebox_dimensions))
		{
			level->messagebox_dimensions = get_v2(150, 120);
		}

		rect text_area = get_rect_from_center(SCREEN_CENTER_IN_PIXELS, level->messagebox_dimensions);

		v2 margin = get_v2(8, 8);
		render_ui_box(level->static_data, &game->render, add_side_length(text_area, margin));

		render_text(&game->render, game->transient_arena, level->static_data->ui_font, 
			text_area, level->message_to_show, true);

		if (level->min_message_timer <= 0.0f)
		{
			game->message_dots_timer += delta_time;
			if (game->message_dots_timer > 0.4f)
			{
				game->message_dots_timer = 0.0f;
				game->message_dots_index++;
				if (game->message_dots_index > 2)
				{
					game->message_dots_index = 0;
				}
			}
			
			rect dots_indicator_rect = get_rect_from_center(
				get_v2(text_area.min_corner.x + (level->messagebox_dimensions.x / 2), text_area.max_corner.y - 4.5f),
				get_v2(15, 5));

			switch (game->message_dots_index)
			{
				case 0: render_bitmap(&game->render, textures::CHARSET, 
					level->static_data->ui_gfx.msgbox_dots_1, dots_indicator_rect); 
					break;
				case 1: render_bitmap(&game->render, textures::CHARSET, 
					level->static_data->ui_gfx.msgbox_dots_2, dots_indicator_rect); 
					break;
				case 2: render_bitmap(&game->render, textures::CHARSET, 
					level->static_data->ui_gfx.msgbox_dots_3, dots_indicator_rect); 
					break;
			}	
		}
	}

	text_lines* test_scrolling_text = level->current_map.description_lines;
	local_persist r32 y_offset = (SCREEN_HEIGHT / SCALING_FACTOR);
	render_large_text(&game->render, game->transient_arena,
		&level->static_data->scrolling_text_options, *test_scrolling_text, y_offset);

	y_offset -= delta_time * 20;

	
	if (level->active_scene_change.change_scene)
	{
		assert(level->active_scene_change.fade_out_speed > 0.0f);
		process_fade(&game->render, &level->fade_out_perc, delta_time, false, level->active_scene_change.fade_out_speed);
	}

	scene_change scene_change = {}; 
	if (level->fade_out_perc >= 1.0f)
	{
		scene_change = level->active_scene_change;
	}

	process_fade(&game->render, &level->fade_in_perc, delta_time, true);

	return scene_change;
}

scene_change level_choice_update_and_render(game_state* game, static_game_data* static_data, r32 delta_time)
{
	game_input* input = get_last_frame_input(&game->input_buffer);

	if (game->level_choice_menu.time_to_first_interaction > 0.0f)
	{
		game->level_choice_menu.time_to_first_interaction -= delta_time;
	}

	i32 options_x = 80;
	i32 option_y = 100;
	i32 first_option_y = option_y;
	i32 option_y_spacing = 20;

	rect message_area = get_rect_from_corners(get_v2(30, 50), get_v2(260, 100));
	render_text(&game->render, game->transient_arena, static_data->ui_font,
		message_area, static_data->choose_level_message, false);
	
	rect option_rect = get_rect_from_corners(
		get_v2(options_x, option_y),
		get_v2(options_x + 165, option_y + 10));

	for (u32 level_index = 0; level_index < static_data->levels_count; level_index++)
	{
		level_choice level = static_data->levels[level_index];

		r32 new_width = get_text_area_for_single_line(static_data->ui_font, level.name).x;
		option_rect.max_corner.x = option_rect.min_corner.x + new_width;

		render_menu_option(static_data->ui_font, game, option_rect, level.name, level.completed);
		if (game->level_choice_menu.time_to_first_interaction <= 0.0f
			&& was_rect_clicked(input, option_rect))
		{
			game->level_choice_menu.active_scene_change.change_scene = true;
			game->level_choice_menu.active_scene_change.new_scene = scene::GAME;
			game->level_choice_menu.active_scene_change.restore_checkpoint = false;
			game->level_choice_menu.active_scene_change.map_to_load = level.map_name;
		}

		move_rect(&option_rect, get_v2(0.0f, option_y_spacing));
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
		game->level_choice_menu.active_scene_change.new_scene = scene::MAIN_MENU;
	}

	scene_change scene_change = {};
	if (game->level_choice_menu.fade_out_perc >= 1.0f)
	{
		scene_change = game->level_choice_menu.active_scene_change;
	}

	return scene_change;
}

scene_change menu_update_and_render(game_state* game, static_game_data* static_data, r32 delta_time)
{
	game_input* input = get_last_frame_input(&game->input_buffer);

	render_bitmap(&game->render, textures::BACKGROUND_TITLE_SCREEN,
		get_rect_from_corners(get_v2(0, 0), get_v2(384, 320)),
		get_rect_from_corners(get_v2(0, 0), get_v2(SCREEN_WIDTH, SCREEN_HEIGHT) / SCALING_FACTOR));

	rect title_area = get_rect_from_corners(
		get_v2(30, 20),
		get_v2(300, 100));

	render_text(&game->render, game->transient_arena, static_data->title_font, 
		title_area, static_data->title_str, true);
	
	i32 options_x = 140;
	i32 option_y = 140;
	i32 first_option_y = option_y;
	i32 option_y_spacing = 20;

	rect option_interactive_rect = render_menu_option(static_data->ui_font, game, 
		options_x, option_y, static_data->menu_new_game_str);
	if (was_rect_clicked(input, option_interactive_rect))
	{
		game->main_menu.active_scene_change.change_scene = true;
		game->main_menu.active_scene_change.new_scene = scene::LEVEL_CHOICE;
	}

	if (game->checkpoint.used)
	{
		option_y += option_y_spacing;	
		option_interactive_rect = render_menu_option(static_data->ui_font, game,
			options_x, option_y, static_data->menu_continue_str);
		if (was_rect_clicked(input, option_interactive_rect))
		{
			game->main_menu.active_scene_change.change_scene = true;
			game->main_menu.active_scene_change.new_scene = scene::GAME;
			game->main_menu.active_scene_change.restore_checkpoint = true;
		}
	}

	option_y += option_y_spacing;
	option_interactive_rect = render_menu_option(static_data->ui_font, game,
		options_x, option_y, static_data->menu_credits_str);
	if (was_rect_clicked(input, option_interactive_rect))
	{
		game->main_menu.active_scene_change.change_scene = true;
		game->main_menu.active_scene_change.new_scene = scene::CREDITS;
	}

	if (game->show_exit_game_option)
	{
		option_y += option_y_spacing;
		option_interactive_rect = render_menu_option(static_data->ui_font, game,
			options_x, option_y, static_data->menu_exit_str);
		if (was_rect_clicked(input, option_interactive_rect))
		{
			game->main_menu.active_scene_change.change_scene = true;
			game->main_menu.active_scene_change.new_scene = scene::EXIT;
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
	
	scene_change scene_change = {};
	if (game->main_menu.fade_out_perc >= 1.0f)
	{
		scene_change = game->main_menu.active_scene_change;
	}
	return scene_change;
};

string_ref get_death_screen_prompt(static_game_data* static_data)
{
	string_ref result = {};
	i32 text_choice = rand() % 2;
	if (text_choice == 0)
	{
		text_choice = rand() % static_data->death_messages_count;
		result = static_data->death_messages[text_choice];
	}
	else
	{
		result = static_data->default_death_message;
	}

	return result;
}

scene_change death_screen_update_and_render(game_state* game, static_game_data* static_data, r32 delta_time)
{
	scene_change change_to_other_scene = {};

	if (false == game->death_screen.initialized)
	{
		game->death_screen.timer = 1.0f;
		game->death_screen.prompt = get_death_screen_prompt(static_data);
		game->death_screen.fade_in_perc = 1.0f;
		game->death_screen.initialized = true;
	}
	
	rect prompt_area = get_whole_screen_text_area(50.0f);
	render_text(&game->render, game->transient_arena, static_data->ui_font,
		prompt_area, game->death_screen.prompt, true);

	if (game->death_screen.timer > 0.0f)
	{
		game->death_screen.timer -= delta_time;
	}
	else
	{
		if (was_any_key_pressed_in_last_frames(&game->input_buffer, 1))
		{
			game->death_screen.transition_to_game = true;
		}
	}

	if (game->death_screen.transition_to_game)
	{		
		process_fade(&game->render, &game->death_screen.fade_out_perc, delta_time, false, 0.5f);
	}

	if (game->death_screen.transition_to_game
		&& game->death_screen.fade_out_perc >= 1.0f)
	{
		change_to_other_scene.change_scene = true;
		change_to_other_scene.fade_out_speed = 1.5f;
		change_to_other_scene.new_scene = scene::GAME;
		change_to_other_scene.restore_checkpoint = true;
	}

	process_fade(&game->render, &game->death_screen.fade_in_perc, delta_time, true, 0.5f);

	return change_to_other_scene;
}

void main_game_loop(game_state* game, r32 delta_time)
{
	scene_change scene_change = {};

	if (game->current_scene == scene::GAME && game->map_errors.string_size > 0)
	{
		game->current_scene = scene::MAP_ERRORS;
	}

	switch (game->current_scene)
	{
		case scene::GAME:
		{
			// kod inicjalizacyjny jest poniżej
			if (false == game->level_initialized)
			{
				scene_change.change_scene = true;
				scene_change.new_scene = scene::GAME;			
			}
			else
			{
				scene_change = game_update_and_render(game, game->level_state, delta_time);
			}			
		};
		break;
		case scene::MAIN_MENU:
		{
			scene_change = menu_update_and_render(game, game->static_data, delta_time);
		};
		break;
		case scene::LEVEL_CHOICE:
		{
			scene_change = level_choice_update_and_render(game, game->static_data, delta_time);
		};
		break;
		case scene::MAP_ERRORS:
		{
			if (game->map_errors.string_size > 0)
			{			
				render_text(&game->render, game->transient_arena, 
					&game->static_data->parsing_errors_text_options, game->map_errors);

				game_input* input = get_last_frame_input(&game->input_buffer);
				if (was_any_key_pressed_in_last_frames(&game->input_buffer, 1))
				{
					scene_change.change_scene = true;
					scene_change.new_scene = scene::MAIN_MENU;
					game->main_menu = {};
				}
			}
			else
			{
				scene_change.change_scene = true;
				scene_change.new_scene = scene::MAIN_MENU;
				game->main_menu = {};
			}
		};
		break;
		case scene::DEATH:
		{
			scene_change = death_screen_update_and_render(game, game->static_data, delta_time);
		};
		break;
		case scene::CREDITS:
		{
			
		};
		break;
	}

	if (scene_change.change_scene)
	{
		game->current_scene = scene_change.new_scene;
		switch (scene_change.new_scene)
		{
			case scene::GAME:
			{				
				/*printf("przed inicjalizacja permanent arena: %d, transient arena: %d\n",
					game->arena->size_used, game->transient_arena->size_used);*/

				temporary_memory auxillary_memory_for_loading = begin_temporary_memory(game->transient_arena);
					
				string_ref level_to_load_name = {};
				if (scene_change.restore_checkpoint 
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
					level_to_load_name = copy_c_string_to_memory_arena(game->transient_arena, "map_01");
				}

				if (game->game_level_memory.size_used_at_creation != 0)
				{
					end_temporary_memory(game->game_level_memory, true);
				}
				game->game_level_memory = begin_temporary_memory(game->arena);
			
				initialize_level_state(game->level_state, game->static_data, level_to_load_name, game->arena);
				tmx_map_parsing_result parsing_result = load_map(level_to_load_name, game->arena, game->transient_arena);
				if (parsing_result.errors->errors_count > 0)
				{					
					game->map_errors = get_parsing_errors_message(game->arena, &game->render, 
						&game->static_data->parsing_errors_text_options, parsing_result.errors);

					game->level_initialized = false;
				} 
				else
				{
					game->map_errors = {};
					game->level_state->current_map = parsing_result.parsed_map;
					initialize_current_map(game, game->level_state);
					game->level_initialized = true;
						
					if (scene_change.restore_checkpoint && game->checkpoint.used)
					{
						restore_checkpoint(game);
					}

					save_checkpoint(game);

					start_playing_music(game->level_state->current_map.music_file_name);

					if (game->level_state->current_map.description_lines == NULL)
					{
						game->level_state->current_map.description_lines = get_division_of_text_into_lines(
							game->arena, &game->static_data->scrolling_text_options, game->level_state->current_map.description);
					}
				}

				end_temporary_memory(auxillary_memory_for_loading, true);
				
			/*	printf("po inicjalizacji permanent arena: %d, transient arena: %d\n",
					game->arena->size_used, game->transient_arena->size_used);*/
			}
			break;
			case scene::MAIN_MENU:
			{
				game->main_menu = {};
				game->main_menu.fade_in_perc = 1.0f;
			}
			break;
			case scene::LEVEL_CHOICE:
			{
				game->level_choice_menu = {};
				game->level_choice_menu.time_to_first_interaction = 1.0f;
				game->level_choice_menu.fade_in_perc = 1.0f;

				load_completed_levels(game->static_data);
			}
			break;
			case scene::MAP_ERRORS:
			{
				
			}
			break;
			case scene::DEATH:
			{
				game->death_screen = {};
			}
			break;
			case scene::CREDITS:
			{

			}
			break;
			case scene::EXIT:
			{
				game->exit_game = true;
			}
			break;
		}
	}

	render_group_to_output(&game->render);
}