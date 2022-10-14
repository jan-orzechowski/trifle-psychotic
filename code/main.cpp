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

void save_checkpoint(game_state* game)
{
	assert(game->level_initialized);	
	assert(game->level_state->current_map_initialized);	

	game->checkpoint = {};
	game->checkpoint.used = true;
	entity* player = get_player(game->level_state);
	if (player->type)
	{
		game->checkpoint.map_name = copy_string_to_buffer(game->level_name_buffer, MAX_LEVEL_NAME_LENGTH, 
			game->level_state->current_map_name);
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

void render_map_layer(render_group* render, level_state* level, map_layer layer, tile_position camera_tile_pos, v2 camera_offset_in_tile)
{	
	if (layer.tiles_count > 0)
	{
		i32 screen_half_width = ceil(HALF_SCREEN_WIDTH_IN_TILES) + 2;
		i32 screen_half_height = ceil(HALF_SCREEN_HEIGHT_IN_TILES) + 2;
		
		for (i32 y_coord_relative = -screen_half_height;
			y_coord_relative < screen_half_height;
			y_coord_relative++)
		{
			i32 y_coord_on_screen = y_coord_relative;
			i32 y_coord_in_world = camera_tile_pos.y + y_coord_relative;

			for (i32 x_coord_relative = -screen_half_width;
				x_coord_relative < screen_half_width;
				x_coord_relative++)
			{
				i32 x_coord_on_screen = x_coord_relative;
				i32 x_coord_in_world = camera_tile_pos.x + x_coord_relative;

				u32 tile_value = get_tile_value(level->current_map, layer, x_coord_in_world, y_coord_in_world);
				if (tile_value != 0 && tile_value != 1) // przezroczyste pola
				{
					rect tile_bitmap = get_tile_bitmap_rect(tile_value);

					v2 position = get_v2(x_coord_on_screen, y_coord_on_screen) - camera_offset_in_tile;
					rect screen_rect = get_tile_screen_rect(position);
					render_bitmap(render, textures::TILESET, tile_bitmap, screen_rect);
				}
			}
		}
	}
}

void render_static_repeated_backdrop(render_group* render, level_state* level,
	world_position camera_position, backdrop_properties backdrop)
{	
	assert(backdrop.texture != textures::NONE);
	assert(false == is_zero(backdrop.size));

	i32 size_y = (i32)backdrop.size.y;
	i32 size_x = (i32)backdrop.size.x;
	for (i32 y = -size_y; y < SCREEN_HEIGHT + size_y; y += size_y)
	{		
		for (i32 x = -size_x; x < SCREEN_WIDTH + size_x; x += size_x)
		{ 
			render_bitmap(render, backdrop.texture, 
				get_rect_from_corners(get_zero_v2(), backdrop.size),
				get_rect_from_min_corner(get_v2(x, y), backdrop.size));
		}
	}
}

void render_scrolling_repeated_backdrop(render_group* render, level_state* level, 
	world_position camera_position, backdrop_properties backdrop, v2 offset_in_tiles, b32 repeat_only_horizontally)
{
	assert(backdrop.texture != textures::NONE);
	assert(false == is_zero(backdrop.size));

	v2 backdrop_size_in_tiles = (backdrop.size / TILE_SIDE_IN_PIXELS);
	if (backdrop.x_slowdown != 0)
	{
		backdrop_size_in_tiles.x *= backdrop.x_slowdown;
	}
	if (backdrop.y_slowdown != 0)
	{
		backdrop_size_in_tiles.y *= backdrop.y_slowdown;
	}

	tile_position camera_tile_pos = get_tile_position(camera_position);
	tile_position backdrop_origin_tile_pos = get_tile_position(
		camera_tile_pos.x - (camera_tile_pos.x % (i32)backdrop_size_in_tiles.x),
		camera_tile_pos.y - (camera_tile_pos.y % (i32)backdrop_size_in_tiles.y));

	if (repeat_only_horizontally)
	{
		backdrop_origin_tile_pos.y = 0;
	}

	v2 backdrop_offset = 
		get_position_difference(camera_position, backdrop_origin_tile_pos) + offset_in_tiles;
		
	if (backdrop.x_slowdown != 0)
	{
		backdrop_offset.x /= backdrop.x_slowdown;
	}
	if (backdrop.y_slowdown != 0)
	{
		backdrop_offset.y /= backdrop.y_slowdown;
	}

	if (repeat_only_horizontally)
	{
		r32 max_y_offset = (backdrop.size.y - (SCREEN_HEIGHT / SCALING_FACTOR)) / TILE_SIDE_IN_PIXELS;
		if (backdrop_offset.y > max_y_offset)
		{
			backdrop_offset.y = max_y_offset;
		}
		if (backdrop_offset.y < 0)
		{
			backdrop_offset.y = 0;
		}
	}	

	i32 size_y = (i32)backdrop.size.y;
	i32 size_x = (i32)backdrop.size.x;

	i32 starting_y = -(backdrop_offset.y * TILE_SIDE_IN_PIXELS);
	i32 starting_x = -(backdrop_offset.x * TILE_SIDE_IN_PIXELS);
	for (i32 y = starting_y; y < SCREEN_HEIGHT; y += backdrop.size.y)
	{			
		for (i32 x = starting_x; x < SCREEN_WIDTH; x += backdrop.size.x)
		{					
			render_bitmap(render, backdrop.texture,
				get_rect_from_corners(get_zero_v2(), backdrop.size), 
				get_rect_from_min_corner(get_v2(x, y), backdrop.size));
		}	
	}
}

void update_backdrops_movement(backdrop_properties* backdrop, v2* backdrop_offset, r32 delta_time, v2 player_velocity)
{
	if (backdrop->texture != textures::NONE)
	{
		backdrop_offset->x += (backdrop->x_speed * delta_time);
		backdrop_offset->y += (backdrop->y_speed * delta_time);
		
		r32 max_x_offset = backdrop->size.x / TILE_SIDE_IN_PIXELS;
		if (backdrop_offset->x > max_x_offset)
		{
			backdrop_offset->x = 0.0f;
		}

		r32 max_y_offset = backdrop->size.y / TILE_SIDE_IN_PIXELS;
		if (backdrop_offset->y > max_y_offset)
		{
			backdrop_offset->y = 0.0f;
		}
	}	
}

void render_backdrops(render_group* render, level_state* level, world_position camera_position)
{	
	if (level->current_map.second_backdrop.texture != textures::NONE)
	{
		if (level->current_map.second_backdrop.x_slowdown == 0
			&& level->current_map.second_backdrop.y_slowdown == 0)
		{
			render_static_repeated_backdrop(render, level, camera_position, level->current_map.second_backdrop);
		}
		else
		{
			render_scrolling_repeated_backdrop(render, level, camera_position, level->current_map.second_backdrop, 
				level->current_map.second_backdrop_offset, true);
		}
	}

	if (level->current_map.first_backdrop.texture != textures::NONE)
	{
		if (level->current_map.first_backdrop.x_slowdown == 0
			&& level->current_map.first_backdrop.y_slowdown == 0)
		{
			render_static_repeated_backdrop(render, level, camera_position, level->current_map.first_backdrop);
		}
		else
		{
			render_scrolling_repeated_backdrop(render, level, camera_position, level->current_map.first_backdrop,
				level->current_map.first_backdrop_offset, true);
		}
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
			}
		}

		if (get_tile_position(player->position).y > level->current_map.height)
		{
			// przegrywamy - spadliśmy z mapy
			level->active_scene_change.change_scene = true;
			level->active_scene_change.new_scene = scene::DEATH;
			level->active_scene_change.fade_out_speed = 0.5f;
			level->stop_player_movement = true;
		}

		if (level->player_invincibility_cooldown > 0.0f)
		{
			level->player_invincibility_cooldown -= delta_time;
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
			v4 color = collision.collided_switch->type->color;
			open_gates_with_given_color(level, color);
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
	
	update_backdrops_movement(&level->current_map.first_backdrop, &level->current_map.first_backdrop_offset, 
		delta_time, player->velocity);
	update_backdrops_movement(&level->current_map.second_backdrop, &level->current_map.second_backdrop_offset,
		delta_time, player->velocity);

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

		render_counter(level->static_data, &game->render, game->transient_arena, level->enemies_to_kill_counter, 99);
	}

	// render crosshair
	if (false == level->stop_player_movement)
	{
		game_input* input = get_last_frame_input(&game->input_buffer);
		v2 relative_mouse_pos = get_v2(input->mouse_x, input->mouse_y) / SCALING_FACTOR;
		rect screen_rect = get_rect_from_center(relative_mouse_pos, get_v2(13, 13));
		render_bitmap(&game->render, textures::CHARSET, level->static_data->ui_gfx.crosshair, screen_rect);
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

scene_change menu_update_and_render(game_state* game, static_game_data* static_data, r32 delta_time)
{
	scene_change change_to_other_scene = {};

	game_input* input = get_last_frame_input(&game->input_buffer);
	if (input->up.number_of_presses > 0)
	{
		if (game->main_menu.option_change_timer < 0.0f)
		{
			game->main_menu.option_index--;
			game->main_menu.option_change_timer = 0.15f;;
		}
	} 
	if (input->down.number_of_presses > 0)
	{
		if (game->main_menu.option_change_timer < 0.0f)
		{
			game->main_menu.option_index++;
			game->main_menu.option_change_timer = 0.15f;;
		}
	}

	game->main_menu.option_change_timer -= delta_time;
	game->main_menu.option_max_index = 1; 
	if (game->show_exit_game_option) game->main_menu.option_max_index++;
	if (game->checkpoint.used) game->main_menu.option_max_index++;

	if (game->main_menu.option_index < 0)
	{
		game->main_menu.option_index = game->main_menu.option_max_index;
	}
	if (game->main_menu.option_index > game->main_menu.option_max_index)
	{
		game->main_menu.option_index = 0;
	}

	if (input->fire.number_of_presses > 0)
	{
		switch (game->main_menu.option_index)
		{
			case 0: 
			{
				// new game
				change_to_other_scene.change_scene = true;
				change_to_other_scene.new_scene = scene::GAME;
				change_to_other_scene.restore_checkpoint = false;
			} 
			break;
			case 1:
			{
				if (game->checkpoint.used)
				{
					// continue
					change_to_other_scene.change_scene = true;
					change_to_other_scene.new_scene = scene::GAME;
					change_to_other_scene.restore_checkpoint = true;
				}
				else
				{
					change_to_other_scene.change_scene = true;
					change_to_other_scene.new_scene = scene::CREDITS;
				}
			}		
			break;
			case 2:
			{
				if (game->checkpoint.used)
				{
					change_to_other_scene.change_scene = true;
					change_to_other_scene.new_scene = scene::CREDITS;
				}
				else
				{
					if (game->show_exit_game_option)
					{
						change_to_other_scene.change_scene = true;
						change_to_other_scene.new_scene = scene::EXIT;
					}
				}
			}
			break;
			case 3:
			{
				if (game->show_exit_game_option)
				{
					change_to_other_scene.change_scene = true;
					change_to_other_scene.new_scene = scene::EXIT;
				}
			}
			break;
			invalid_default_case;
		}
	}
	
	i32 options_x = 140;
	i32 option_y = 140;
	i32 first_option_y = option_y;
	i32 option_y_spacing = 20;

	render_menu_option(static_data->menu_font, game, 
		options_x, option_y, static_data->menu_new_game_str);
	option_y += option_y_spacing;

	if (game->checkpoint.used)
	{
		render_menu_option(static_data->menu_font, game,
			options_x, option_y, static_data->menu_continue_str);
		option_y += option_y_spacing;
	}

	render_menu_option(static_data->menu_font, game, 
		options_x, option_y, static_data->menu_credits_str);
	option_y += option_y_spacing;

	if (game->show_exit_game_option)
	{
		render_menu_option(static_data->menu_font, game,
			options_x, option_y, static_data->menu_exit_str);
	}

	u32 indicator_y = first_option_y + (game->main_menu.option_index * option_y_spacing) - 4;
	u32 indicator_x = options_x - 20;

	rect indicator_screen_rect = get_rect_from_min_corner(indicator_x, indicator_y, 16, 16);
	render_bitmap(&game->render, textures::CHARSET, static_data->ui_gfx.menu_indicator, indicator_screen_rect);

	process_fade(&game->render, &game->main_menu.fade_percentage, delta_time, true, 1.5f);

	return change_to_other_scene;
};

scene_change death_screen_update_and_render(game_state* game, static_game_data* static_data, r32 delta_time)
{
	scene_change change_to_other_scene = {};

	if (false == game->death_screen.initialized)
	{
		game->death_screen.timer = 1.0f;
		// tutaj mamy alokację powodującą zwiększenie transient arena...
		const char* prompt = "Jesli nie teraz, umarlbys za 30 lat";
		game->death_screen.prompt = copy_c_string_to_memory_arena(game->transient_arena, prompt);
		game->death_screen.fade_in_perc = 1.0f;
		game->death_screen.initialized = true;
	}
	
	render_text(&game->render, game->transient_arena, static_data->ui_font,
		get_whole_screen_text_area(50.0f), game->death_screen.prompt);

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

void main_game_loop(game_state* game, static_game_data* static_data, r32 delta_time)
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
			scene_change = menu_update_and_render(game, static_data, delta_time);
		};
		break;
		case scene::MAP_ERRORS:
		{
			if (game->map_errors.string_size > 0)
			{			
				render_text(&game->render, game->transient_arena, static_data->ui_font, 
					get_whole_screen_text_area(0.0f), game->map_errors);

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
			scene_change = death_screen_update_and_render(game, static_data, delta_time);
		};
		break;
		case scene::CREDITS:
		{
			
		};
		break;
		invalid_default_case;
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
			
				initialize_level_state(game->level_state, static_data, level_to_load_name, game->arena);
				tmx_map_parsing_result parsing_result = load_map(level_to_load_name, game->arena, game->transient_arena);
				if (parsing_result.errors->errors_count > 0)
				{
					game->map_errors = get_parsing_errors_message(game->arena, &game->render,
						static_data->ui_font, get_whole_screen_text_area(0.0f), parsing_result.errors);

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
				}

				end_temporary_memory(auxillary_memory_for_loading, true);
				
			/*	printf("po inicjalizacji permanent arena: %d, transient arena: %d\n",
					game->arena->size_used, game->transient_arena->size_used);*/
			}
			break;
			case scene::MAIN_MENU:
			{
				game->main_menu = {};
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
			invalid_default_case;
		}
	}

	render_group_to_output(&game->render);
}