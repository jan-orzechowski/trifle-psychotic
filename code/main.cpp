#include "main.h"
#include "game_data.h"
#include "text_rendering.h"
#include "map.h"
#include "animation.h"
#include "gates.h"
#include "collision.h"
#include "entities.h"
#include "player.h"
#include "tmx_parsing.h"
#include "rendering.h"
#include "ui.h"
#include "input.h"
#include "sdl_platform.h"

void render_debug_information(game_state* game, level_state* level)
{
	entity* player = get_player(level);

	b32 is_standing = is_standing_on_ground(level, player);

	char buffer[200];
	v4 text_color = get_v4(1, 1, 1, 0);
	int error = snprintf(buffer, 200, "Chunk:(%d,%d),Pos:(%0.2f,%0.2f),Acc: (%0.2f,%0.2f) Standing: %d, Direction: %s",
		player->position.chunk_pos.x, player->position.chunk_pos.y, player->position.pos_in_chunk.x, player->position.pos_in_chunk.y,
		player->acceleration.x, player->acceleration.y, is_standing, (player->direction == direction::W ? "W" : "E"));

	rect area = get_rect_from_corners(
		get_v2(10, 200), get_v2((SCREEN_WIDTH / 2) - 10, 260)
	);

	render_text(&game->render, game->transient_arena,
		game->level_state->static_data->ui_font, area, buffer, 200, true);
}

scene_change game_update_and_render(game_state* game, level_state* level, r32 delta_time)
{
	if (false == level->current_map_initialized)
	{
		start_screen_shake(level, 0.6f, 30.0f);

		initialize_current_map(game, level);
	}

	entity* player = get_player(level);

	local_persist b32 update = true;
	local_persist r32 min_message_time = 2.0f;

	local_persist string_ref text_to_show = {};

	if (false == update)
	{
		if (min_message_time < 0.0f)
		{
			game_input* input = get_last_frame_input(&game->input_buffer);
			if (input->fire.number_of_presses > 0)
			{
				update = true;
			}
		}
		else
		{
			min_message_time -= delta_time;
		}
	}

	// update player
	if (update) 
	{
		if (player->health <= 0.0f)
		{
			// przegrywamy
			if (false == level->active_scene_change.change_scene)
			{
				level->active_scene_change.change_scene = true;
				level->active_scene_change.new_scene = scene::DEATH;
				level->scene_fade_perc = 0.0f;
				level->active_scene_change.fade_out_speed = 0.5f;
				update = false;

				if (player->type->death_animation)
				{
					add_explosion(level, add_to_position(player->position, player->type->death_animation_offset),
						player->type->death_animation);
				}
			}
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

#if 0
		if (collision.collided_message_display)
		{
			string_ref message = collision.collided_message_display->type->message;
			if (message.string_size)
			{
				printf(message.ptr); printf("\n");	
				update = false;
				text_to_show = collision.collided_message_display->type->message;
				collision.collided_message_display->type->message = {};
			}
		}
#endif

		if (collision.collided_transition 
			&& false == level->active_scene_change.change_scene)
		{
			level->active_scene_change.change_scene = true;
			level->active_scene_change.new_scene = scene::GAME;
			level->active_scene_change.map_to_load = level->current_map.next_map;
			level->active_scene_change.fade_out_speed = 1.5f;
			level->scene_fade_perc = 0.0f;
		}

		v2 player_direction_v2 = get_unit_vector(player->velocity);
		if (false == is_zero(player->velocity))
		{
			player->direction = player->velocity.x < 0 ? direction::W : direction::E;
		}

		if (is_power_up_active(level->power_ups.invincibility))
		{
			start_visual_effect(player, &level->static_data->visual_effects[2], true);
		}
		else 
		{
			stop_visual_effect(player, &level->static_data->visual_effects[2]);
		}
	}

	// update entities
	if (update)
	{
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
				if (entity->type->death_animation)
				{
					add_explosion(level, add_to_position(entity->position, entity->type->death_animation_offset),
						entity->type->death_animation);
				}

				remove_entity(entity);
				continue;
			}
			
			v2 distance_to_player = get_position_difference(player->position, entity->position);
			r32 distance_to_player_length = length(distance_to_player);

			// poruszanie się
			if (are_entity_flags_set(entity, entity_flags::WALKS_HORIZONTALLY)
				|| are_entity_flags_set(entity, entity_flags::FLIES_HORIZONTALLY)
				|| are_entity_flags_set(entity, entity_flags::FLIES_VERTICALLY))
			{				
				if (false == entity->has_walking_path)
				{
					if (are_entity_flags_set(entity, entity_flags::WALKS_HORIZONTALLY))
					{
						find_walking_path_for_enemy(level, entity);
					}

					if (are_entity_flags_set(entity, entity_flags::FLIES_HORIZONTALLY))
					{
						find_flying_path_for_enemy(level, entity, false);
					}

					if (are_entity_flags_set(entity, entity_flags::FLIES_VERTICALLY))
					{
						find_flying_path_for_enemy(level, entity, true);
					}
				}

				tile_position current_goal;
				tile_position current_start;

				if (entity->goal_path_point == 0)
				{
					current_goal = entity->path.start;
					current_start = entity->path.end;
				}
				else if (entity->goal_path_point == 1)
				{
					current_goal = entity->path.end;
					current_start = entity->path.start;
				}
				else
				{
					// wracamy na początek
					entity->goal_path_point = 0;
					current_goal = entity->path.start;
					current_start = entity->path.end;
				}
				
				v2 player_target_offset = get_v2(0.0f, -0.3f);
				world_position player_as_target = add_to_position(player->position, get_v2(0.0f, -0.3f));
				if (is_point_visible_for_entity(level, entity, player_as_target))
				{
					entity->player_detected = true;

					if (are_entity_flags_set(entity, entity_flags::ENEMY)
						&& entity->type->fired_bullet_type)
					{
						if (entity->attack_cooldown > 0.0f)
						{
							// nie robimy nic
							entity->attack_cooldown -= delta_time;

							if (entity->attack_cooldown <= 0.0f)
							{
								entity->attack_series_duration = entity->type->default_attack_series_duration;
							}
						}
						else
						{
							if (entity->attack_series_duration > 0.0f)
							{
								entity->attack_series_duration -= delta_time;

								if (entity->attack_bullet_interval_duration > 0.0f)
								{
									// przerwa, nic nie robimy
									entity->attack_bullet_interval_duration -= delta_time;
								}
								else
								{
									// wystrzeliwujemy jeden pocisk
									enemy_fire_bullet(level, entity, player, get_v2(0.0f, -0.3f));
									entity->attack_bullet_interval_duration =
										entity->type->default_attack_bullet_interval_duration;
								}
							}
							else
							{
								// przywracamy cooldown
								entity->attack_cooldown = entity->type->default_attack_cooldown;
							}
						}
					}
				}
				else
				{
					entity->player_detected = false;
					entity->attack_cooldown = 0.0f;
					entity->attack_series_duration = 0.0f;
					entity->attack_bullet_interval_duration = 0.0f;
				}				

				// zatrzymywanie się lub podchodzenie w zależności od pozycji gracza				
				if (entity->player_detected)
				{
					set_entity_rotated_graphics(entity, &player->position);

					if (distance_to_player_length > entity->type->forget_detection_distance)
					{
						entity->player_detected = false;
					}
					else
					{
						if (distance_to_player_length < entity->type->stop_movement_distance)
						{
							tile_position entity_position = get_tile_position(entity->position);
							current_goal = entity_position;
							current_start = entity_position;										
						}
						else
						{
							tile_position closest_end = get_closest_end_from_tile_range(entity->path, player->position);
							current_goal = closest_end;
						}
					}
				}
				else
				{
					set_entity_rotated_graphics(entity, NULL);
				}

				move_entity(level, entity, current_start, current_goal, player, delta_time);

				if (length(entity->velocity) <= 0.5f)
				{
					entity->current_animation = NULL;
					entity->animation_duration = 0.0f;
				}
				else
				{
					if (entity->current_animation != entity->type->walk_animation)
					{
						entity->current_animation = entity->type->walk_animation;
						entity->animation_duration = 0.0f;
					}
				}

				r32 frame_duration_modifier = 0.75f + (1.0f / length(entity->velocity));
				animate_entity(NULL, entity, delta_time, frame_duration_modifier);
			}
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

				if (is_zero(bullet->velocity))
				{
					remove_bullet(level, &bullet_index);
				}
			}
			else
			{
				remove_bullet(level, &bullet_index);
			}
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
	
		i32 screen_half_width = ceil(HALF_SCREEN_WIDTH_IN_TILES) + 2;
		i32 screen_half_height = ceil(HALF_SCREEN_HEIGHT_IN_TILES) + 2;

		// draw tiles
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

				u32 tile_value = get_tile_value(level->current_map, x_coord_in_world, y_coord_in_world);
				rect tile_bitmap = get_tile_bitmap_rect(tile_value);

				v2 position = get_v2(x_coord_on_screen, y_coord_on_screen) - camera_offset_in_tile;
				rect screen_rect = get_tile_screen_rect(position);
				render_bitmap(&game->render, textures::TILESET, tile_bitmap, screen_rect);

#if 0
				if (is_tile_colliding(level->static_data->collision_reference, tile_value))
				{
					tile_position tile_pos = get_tile_position(x_coord_in_world, y_coord_in_world);
					entity_collision_data tile_collision = get_tile_collision_data(camera_position.chunk_pos, tile_pos);
					v2 relative_position = get_position_difference(tile_pos, camera_position);
					v2 center = relative_position + tile_collision.collision_rect_offset;
					v2 size = tile_collision.collision_rect_dim;
					rect collision_rect = get_rect_from_center(
						SCREEN_CENTER_IN_PIXELS + (center * TILE_SIDE_IN_PIXELS),
						(size * TILE_SIDE_IN_PIXELS));

					render_rectangle(&game->render, collision_rect, { 0,0,0,0 }, true);
				}
#endif
			}
		}

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

		// draw collision debug info
		{
#if 0
			for (i32 entity_index = 0; entity_index < level->entities_count; entity_index++)
			{
				entity* entity = level->entities + entity_index;
				if (false == entity->used)
				{
					continue;
				}

				if (is_in_neighbouring_chunk(camera_position.chunk_pos, entity->position))
				{
					// istotne - offset sprite'a nie ma tu znaczenia
					v2 relative_position = get_position_difference(entity->position, camera_position);
					v2 center = relative_position + entity->type->collision_rect_offset;
					v2 size = entity->type->collision_rect_dim;
					rect collision_rect = get_rect_from_center(
						SCREEN_CENTER_IN_PIXELS + (center * TILE_SIDE_IN_PIXELS),
						size * TILE_SIDE_IN_PIXELS);

					render_rectangle(&game->render, collision_rect, { 0, 0, 0, 0 }, true);

					v2 entity_position = SCREEN_CENTER_IN_PIXELS + relative_position * TILE_SIDE_IN_PIXELS;
					render_point(&game->render, entity_position, get_v4(1, 0, 0, 0));
				}
			}
#endif
		}

		render_debug_information(game, level);

		render_hitpoint_bar(level->static_data, &game->render, player, is_power_up_active(level->power_ups.invincibility));
	}

	// render crosshair
	{
		game_input* input = get_last_frame_input(&game->input_buffer);
		v2 relative_mouse_pos = get_v2(input->mouse_x, input->mouse_y) / SCALING_FACTOR;
		rect screen_rect = get_rect_from_center(relative_mouse_pos, get_v2(13, 13));
		render_bitmap(&game->render, textures::CHARSET, level->static_data->cursor, screen_rect);
	}

	if (update == false && text_to_show.string_size)
	{
		rect text_area = get_rect_from_center(SCREEN_CENTER_IN_PIXELS, get_v2(100, 100));

		v2 margin = get_v2(4, 4);
		render_textbox(level->static_data, &game->render, add_side_length(text_area, margin));

		render_text(&game->render, game->transient_arena, level->static_data->ui_font, 
			text_area, text_to_show, true);
	}

	if (level->active_scene_change.change_scene)
	{
		assert(level->active_scene_change.fade_out_speed > 0.0f);

		level->scene_fade_perc += (delta_time * level->active_scene_change.fade_out_speed);
		if (level->scene_fade_perc > 1.0f)
		{
			level->scene_fade_perc = 1.0f;
		}

		v4 fade_color = get_zero_v4();
		render_fade(&game->render, fade_color, level->scene_fade_perc);
	}

	scene_change scene_change = {}; 
	if (level->scene_fade_perc >= 1.0f)
	{
		scene_change = level->active_scene_change;
	}

	if (level->fade_in_perc > 0.0f)
	{
		level->fade_in_perc -= (delta_time * 1.5f);
		if (level->fade_in_perc < 0.0f)
		{
			level->fade_in_perc = 0.0f;
		}

		v4 fade_color = get_zero_v4();
		render_fade(&game->render, fade_color, level->fade_in_perc);
	}

	return scene_change;
}

scene_change menu_update_and_render(game_state* game, static_game_data* static_data, r32 delta_time)
{
	scene_change change_to_other_scene = {};

	game_input* input = get_last_frame_input(&game->input_buffer);

	local_persist i32 menu_index = 0;
	local_persist i32 menu_max_index = 3;

	r32 menu_change_default_timer = 0.15f;
	local_persist r32 menu_change_timer = 0.0f;

	if (input->up.number_of_presses > 0)
	{
		if (menu_change_timer < 0)
		{
			menu_index--;
			menu_change_timer = menu_change_default_timer;
		}
	} 
	if (input->down.number_of_presses > 0)
	{
		if (menu_change_timer < 0)
		{
			menu_index++;
			menu_change_timer = menu_change_default_timer;
		}
	}

	menu_change_timer -= delta_time;

	if (menu_index < 0)
	{
		menu_index = menu_max_index;
	}
	if (menu_index > menu_max_index)
	{
		menu_index = 0;
	}

	if (input->fire.number_of_presses)
	{
		switch (menu_index)
		{
			case 0: 
			{
				printf("Nowa gra\n");
				change_to_other_scene.change_scene = true;
				change_to_other_scene.new_scene = scene::GAME;
			} 
			break;
			case 1:
			{
				printf("Kontynuacja\n");
				change_to_other_scene.change_scene = true;
				change_to_other_scene.new_scene = scene::GAME;
				//result.save = ?
			}
			break;
			case 2:
			{
				printf("Credits\n");
				change_to_other_scene.change_scene = true;
				change_to_other_scene.new_scene = scene::CREDITS;
			}
			break;
			case 3:
			{
				printf("Wyjscie\n");
				change_to_other_scene.change_scene = true;
				change_to_other_scene.new_scene = scene::EXIT;
			}
			break;
			invalid_default_case;
		}
	}
	
	i32 option_spacing = 20;
	u32 options_x = 140;
	u32 first_option_y = 140;

	render_menu_option(static_data->menu_font, game, 
		options_x, first_option_y, static_data->menu_new_game_str);
	render_menu_option(static_data->menu_font, game, 
		options_x, first_option_y + option_spacing, static_data->menu_continue_str);
	render_menu_option(static_data->menu_font, game, 
		options_x, first_option_y + 2 * option_spacing, static_data->menu_credits_str);
	render_menu_option(static_data->menu_font, game, 
		options_x, first_option_y + 3 * option_spacing, static_data->menu_exit_str);

	u32 indicator_y = first_option_y + (menu_index * option_spacing) - 4;
	u32 indicator_x = options_x - 20;

	rect indicator_screen_rect = get_rect_from_min_corner(indicator_x, indicator_y, 16, 16);
	rect bitmap_rect = get_rect_from_min_corner(16, 0, 16, 16);

	render_bitmap(&game->render, textures::CHARSET, bitmap_rect, indicator_screen_rect);

	local_persist r32 fade_in_perc = 1.0f;

	if (fade_in_perc > 0.0f)
	{
		fade_in_perc -= (delta_time * 1.5f);
		if (fade_in_perc < 0.0f)
		{
			fade_in_perc = 0.0f;
		}

		v4 fade_color = get_zero_v4();
		render_fade(&game->render, fade_color, fade_in_perc);
	}

	return change_to_other_scene;
};

scene_change death_screen_update_and_render(game_state* game, static_game_data* static_data, r32 delta_time)
{
	scene_change change_to_other_scene = {};

	if (false == game->death_screen.initialized)
	{
		game->death_screen.timer = 5.0f;
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
		game_input* input = get_last_frame_input(&game->input_buffer);
		if (input->fire.number_of_presses > 0)
		{
			game->death_screen.transition_to_main_menu = true;		
		}
	}

	if (game->death_screen.transition_to_main_menu)
	{		
		game->death_screen.fade_out_perc += (delta_time * 0.5f);
		if (game->death_screen.fade_out_perc > 1.0f)
		{
			game->death_screen.fade_out_perc = 1.0f;
		}

		v4 fade_color = get_zero_v4();
		render_fade(&game->render, fade_color, game->death_screen.fade_out_perc);
	}

	if (game->death_screen.transition_to_main_menu 
		&& game->death_screen.fade_out_perc >= 1.0f)
	{
		change_to_other_scene.change_scene = true;
		change_to_other_scene.fade_out_speed = 1.5f;
		change_to_other_scene.new_scene = scene::MAIN_MENU;
	}

	if (game->death_screen.fade_in_perc > 0.0f)
	{
		game->death_screen.fade_in_perc -= (delta_time * 0.5f);
		if (game->death_screen.fade_in_perc < 0.0f)
		{
			game->death_screen.fade_in_perc = 0.0f;
		}

		v4 fade_color = get_zero_v4();
		render_fade(&game->render, fade_color, game->death_screen.fade_in_perc);
	}

	return change_to_other_scene;
}

void main_game_loop(game_state* game, static_game_data* static_data, r32 delta_time)
{
	scene_change scene_change = {};
	switch (game->current_scene)
	{
		case scene::GAME:
		{
			if (false == game->first_game_run_initialized)
			{
				temporary_memory auxillary_memory_for_loading = begin_temporary_memory(game->transient_arena);
				
				game->level_state = push_struct(game->arena, level_state);
				game->game_level_memory = begin_temporary_memory(game->arena);
				string_ref level_name = copy_c_string_to_memory_arena(game->arena, "map_01");
				initialize_level_state(game->level_state, static_data, level_name, game->arena);

				tmx_map_parsing_result parsing_result = load_map(level_name, game->arena, game->transient_arena);
				game->level_state->current_map = parsing_result.parsed_map;

				if (parsing_result.errors->errors_count > 0)
				{
					scene_change.change_scene = true;
					scene_change.new_scene = scene::MAP_ERRORS;
					game->map_errors = get_parsing_errors_message(game->arena, &game->render, 
						static_data->ui_font, get_whole_screen_text_area(0.0f), parsing_result.errors);

					end_temporary_memory(auxillary_memory_for_loading, true);
					break;
				}

				game->first_game_run_initialized = true;
				end_temporary_memory(auxillary_memory_for_loading, true);
			}
						
			scene_change = game_update_and_render(game, game->level_state, delta_time);
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
				if (input->is_left_mouse_key_held || input->fire.number_of_presses > 0)
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
			// czy potrzebujemy tutaj osobnej "sceny"?
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
				if (scene_change.map_to_load.string_size == 0)
				{
					scene_change.change_scene = true;	
					scene_change.new_scene = scene::MAIN_MENU;
					game->main_menu = {};
					break;
				}

				temporary_memory auxillary_memory_for_loading = begin_temporary_memory(game->transient_arena);
				{
					string_ref level_name = copy_string(game->transient_arena, scene_change.map_to_load);
					save* save = save_player_state(game->transient_arena, game->level_state);
					end_temporary_memory(game->game_level_memory, true);

					game->game_level_memory = begin_temporary_memory(game->arena);
					initialize_level_state(game->level_state, static_data, level_name, game->arena);

					tmx_map_parsing_result parsing_result = load_map(level_name, game->arena, game->transient_arena);
					game->level_state->current_map = parsing_result.parsed_map;

					initialize_current_map(game, game->level_state);
					restore_player_state(game->level_state, save);
				}
				end_temporary_memory(auxillary_memory_for_loading, true);
			}
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
			invalid_default_case;
		}
	}

	render_group_to_output(&game->render);
}