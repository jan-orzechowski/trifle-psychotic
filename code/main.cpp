﻿#include "main.h"
#include "game_data.h"
#include "text_rendering.h"
#include "map.h"
#include "animation.h"
#include "gates.h"
#include "collision.h"
#include "entities.h"
#include "player.h"

#include "sdl_platform.h"

void* push_render_element(render_group* group, u32 size, render_group_entry_type type)
{
	render_group_entry_header* result = 0;
	u32 header_size = sizeof(render_group_entry_header);

	if ((group->push_buffer_size + size + header_size) < group->max_push_buffer_size)
	{
		result = (render_group_entry_header*)(group->push_buffer_base + group->push_buffer_size);
		result->type = type;

		// dajemy wskaźnik do elementu, nie do headera - ten zostanie odczytany w render_group_to_output
		result = result + 1; // przesuwamy się o rozmiar headera
		group->push_buffer_size += (size + header_size);
	}
	else
	{
		invalid_code_path;
	}

	return result;
}

void render_bitmap(render_group* group, textures texture, rect source_rect, rect destination_rect)
{
	render_group_entry_bitmap* entry = (render_group_entry_bitmap*)push_render_element(
		group, sizeof(render_group_entry_bitmap), render_group_entry_type::BITMAP);

	entry->source_rect = source_rect;
	entry->destination_rect = destination_rect;
	entry->texture = texture;
}

void render_rectangle(render_group* group, rect screen_rect_to_fill, v4 color, b32 render_outline_only)
{
	render_group_entry_debug_rectangle* entry = (render_group_entry_debug_rectangle*)push_render_element(
		group, sizeof(render_group_entry_debug_rectangle), render_group_entry_type::DEBUG_RECTANGLE);

	entry->destination_rect = screen_rect_to_fill;
	entry->color = color;
	entry->render_outline_only = render_outline_only;
}

void render_point(render_group* group, v2 point, v4 color)
{
	rect screen_rect = get_rect_from_center(point, get_v2(2.0f, 2.0f));
	render_rectangle(group, screen_rect, color, false);
}

void render_bitmap_with_effects(render_group* group,
	textures texture, rect source_rect, rect destination_rect, v4 tint_color, b32 render_in_additive_mode, b32 flip_horizontally)
{
	render_group_entry_bitmap_with_effects* entry = (render_group_entry_bitmap_with_effects*)push_render_element(
		group, sizeof(render_group_entry_bitmap_with_effects), render_group_entry_type::BITMAP_WITH_EFFECTS);

	entry->source_rect = source_rect;
	entry->destination_rect = destination_rect;
	entry->texture = texture;
	entry->tint_color = tint_color;
	entry->render_in_additive_mode = render_in_additive_mode;
	entry->flip_horizontally = flip_horizontally;
}

static void render_clear(render_group* group, v4 color)
{
	render_group_entry_clear* entry = (render_group_entry_clear*)push_render_element(
		group, sizeof(render_group_entry_clear), render_group_entry_type::BITMAP_WITH_EFFECTS);

	entry->color = color;
}

void render_fade(render_group* group, v4 color, r32 percentage)
{
	render_group_entry_fade* entry = (render_group_entry_fade*)push_render_element(
		group, sizeof(render_group_entry_fade), render_group_entry_type::FADE);

	entry->color = color;
	entry->percentage = percentage;
}

input_buffer initialize_input_buffer(memory_arena* arena)
{
	input_buffer result = {};
	result.size = 60 * 2; // 2 sekundy
	result.buffer = push_array(arena, result.size, game_input);
	return result;
}

void render_hitpoint_bar(static_game_data* static_data, render_group* render, entity* player, b32 draw_white_bars)
{
	// zabezpieczenie na uint wrapping
	if (player->health < 0.0f)
	{
		player->health = 0.0f;
	}

	u32 filled_health_bars = (u32)(player->health / 10);
	u32 max_health_bars = (u32)(player->type->max_health / 10);
	
	rect icon_screen_rect = get_rect_from_min_corner(get_v2(10, 10), get_v2(10, 10));
	rect bar_screen_rect = get_rect_from_min_corner(get_v2(18, 10), get_v2(4, 8));
	
	rect bar_texture_rect = static_data->ui_gfx.healthbar_red_bar;
	if (draw_white_bars)
	{
		bar_texture_rect = static_data->ui_gfx.healthbar_white_bar;
	}

	render_bitmap(render, textures::CHARSET, static_data->ui_gfx.healthbar_icon, icon_screen_rect);

	for (u32 health_bar_index = 0;
		health_bar_index < filled_health_bars;
		health_bar_index++)
	{
		bar_screen_rect = move_rect(bar_screen_rect, get_v2(4, 0));
		render_bitmap(render, textures::CHARSET, bar_texture_rect, bar_screen_rect);
	}

	bar_texture_rect = static_data->ui_gfx.healthbar_empty_bar;
	for (u32 health_bar_index = filled_health_bars;
		health_bar_index < max_health_bars;
		health_bar_index++)
	{
		bar_screen_rect = move_rect(bar_screen_rect, get_v2(4, 0));
		render_bitmap(render, textures::CHARSET, bar_texture_rect, bar_screen_rect);
	}
}

rect get_tile_rect(u32 tile_id)
{
	rect result = {};
	if (tile_id == 0)
	{
		
	}
	else
	{
		// id liczą się od 1, nie od zera
		u32 column = (tile_id - 1) % TILESET_WIDTH;
		u32 row = (tile_id - 1) / TILESET_WIDTH;

		u32 x = column * TILE_SIDE_IN_PIXELS;
		u32 y = row * TILE_SIDE_IN_PIXELS;

		result = get_rect_from_min_corner(get_v2(x, y), get_v2(TILE_SIDE_IN_PIXELS, TILE_SIDE_IN_PIXELS));
	}
	
	return result;
}

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

void write_to_input_buffer(input_buffer* buffer, game_input* new_input)
{
	buffer->buffer[buffer->current_index] = *new_input;
	buffer->current_index++;
	if (buffer->current_index == buffer->size)
	{
		buffer->current_index = 0;
	}
}

game_input* get_past_input(input_buffer* buffer, u32 how_many_frames_backwards)
{
	i32 input_index = buffer->current_index - 1 - how_many_frames_backwards;
	while (input_index < 0)
	{
		input_index = buffer->size + input_index;
	}
	game_input* result = &buffer->buffer[input_index];
	return result;
}

game_input* get_last_frame_input(input_buffer* buffer)
{
	game_input* result = get_past_input(buffer, 0);
	return result;
}

// żeby działało na dowolnych przyciskach, trzeba dodać nowy enum
b32 was_up_key_pressed_in_last_frames(input_buffer* buffer, u32 number_of_frames)
{
	b32 result = false;
	for (u32 frame = 1; frame <= number_of_frames; frame++)
	{
		game_input* input = get_past_input(buffer, frame);
		if (input->up.number_of_presses > 0)
		{
			result = true;
			break;
		}
	}
	return result;
}

void circular_buffer_test(memory_arena* arena)
{
	temporary_memory test_memory = begin_temporary_memory(arena);

	u32 test_input_count = 200;

	input_buffer* input_buf = push_struct(test_memory.arena, input_buffer);
	input_buf->size = 100;
	input_buf->buffer = push_array(test_memory.arena, input_buf->size, game_input);

	for (u32 input_index = 0; input_index < test_input_count; input_index++)
	{
		game_input* new_test_input = push_struct(test_memory.arena, game_input);
		new_test_input->up.number_of_presses = input_index;
		write_to_input_buffer(input_buf, new_test_input);
	}

	game_input* test_input = 0;
	test_input = get_past_input(input_buf, 0);
	assert(test_input->up.number_of_presses == test_input_count - 1);

	test_input = get_past_input(input_buf, input_buf->size);
	assert(test_input->up.number_of_presses == test_input_count - 1);

	test_input = get_past_input(input_buf, 1);
	assert(test_input->up.number_of_presses == test_input_count - 2);

	end_temporary_memory(test_memory);
}

rect get_render_rect(v2 position_relative_to_camera, v2 rect_size)
{
	r32 w = rect_size.x;
	r32 h = rect_size.y;
	r32 x = SCREEN_CENTER_IN_PIXELS.x + (position_relative_to_camera.x * TILE_SIDE_IN_PIXELS)
		- (rect_size.x / 2);
	r32 y = SCREEN_CENTER_IN_PIXELS.y + (position_relative_to_camera.y * TILE_SIDE_IN_PIXELS)
		- (rect_size.y / 2);
	
	rect result = get_rect_from_min_corner(x, y, w, h);
	return result;
}

rect get_tile_render_rect(v2 position_relative_to_camera)
{
	r32 w = TILE_SIDE_IN_PIXELS;
	r32 h = TILE_SIDE_IN_PIXELS;
	r32 x = SCREEN_CENTER_IN_PIXELS.x + (position_relative_to_camera.x * TILE_SIDE_IN_PIXELS)
		- (TILE_SIDE_IN_PIXELS / 2);
	r32 y = SCREEN_CENTER_IN_PIXELS.y + (position_relative_to_camera.y * TILE_SIDE_IN_PIXELS)
		- (TILE_SIDE_IN_PIXELS / 2);

	rect result = get_rect_from_min_corner(x, y, w, h);
	return result;
}

void debug_render_tile(render_group* render, tile_position tile_pos, v4 color, world_position camera_pos)
{
	v2 position = get_position_difference(tile_pos, camera_pos);
	rect screen_rect = get_tile_render_rect(position);
	render_rectangle(render, screen_rect, color, false);
}

void render_debug_path_ends(render_group* render, entity* entity, world_position camera_pos)
{
	if (entity->has_walking_path)
	{
		debug_render_tile(render, entity->path.start, { 1, 1, 0, 1 }, camera_pos);
		debug_render_tile(render, entity->path.end, { 0, 0, 1, 1 }, camera_pos);
	}
}

void render_entity_sprite(render_group* render, world_position camera_position, world_position entity_position, direction entity_direction,
	sprite_effect* visual_effect, r32 visual_effect_duration, sprite sprite)
{	
	b32 tint_modified = false;
	v4 tint = get_zero_v4();

	if (visual_effect)
	{
		tint_modified = true;	
		tint = get_tint(visual_effect, visual_effect_duration);
	}	

	for (u32 part_index = 0; part_index < sprite.parts_count; part_index++)
	{
		sprite_part* part = &sprite.parts[part_index];
		v2 offset = part->offset_in_pixels;

		b32 flip = false;
		if (part->default_direction != direction::NONE && entity_direction != part->default_direction)
		{
			flip = !flip;
			offset = reflection_over_y_axis(part->offset_in_pixels);
		}

		if (sprite.flip_horizontally)
		{
			flip = !flip;
			offset = reflection_over_y_axis(part->offset_in_pixels);
		}

		v2 position = get_position_difference(entity_position, camera_position);
		v2 render_rect_dim = get_rect_dimensions(part->texture_rect);
		rect screen_rect = get_render_rect(position, render_rect_dim);
		screen_rect = move_rect(screen_rect, offset);

		if (tint_modified)
		{
			assert(tint.r >= 0 && tint.r <= 1 && tint.g >= 0 && tint.g <= 1 && tint.b >= 0 && tint.b <= 1);
			if (are_flags_set(&visual_effect->flags, sprite_effect_flags::ADDITIVE_MODE))
			{
				render_bitmap_with_effects(render, part->texture, part->texture_rect, screen_rect, tint, false, flip);
				render_bitmap_with_effects(render, part->texture, part->texture_rect, screen_rect, tint, true, flip);
			}
			else
			{
				render_bitmap_with_effects(render, part->texture, part->texture_rect, screen_rect, tint, false, flip);
			}
		}
		else
		{
			render_bitmap_with_effects(render, part->texture, part->texture_rect, screen_rect, get_zero_v4(), false, flip);
		}
	}
}

void render_textbox(static_game_data* static_data, render_group* group, rect textbox_rect)
{
	v2 dimensions = get_rect_dimensions(textbox_rect);

	v4 background_color = get_v4(255, 255, 255, 255);
	render_rectangle(group, textbox_rect, background_color, false);
	
	v2 tile_dimensions = get_v2(4, 4);
	u32 tile_x_count = ceil(dimensions.x / tile_dimensions.x);
	u32 tile_y_count = ceil(dimensions.y / tile_dimensions.y);

	rect first_destination_rect = get_rect_from_min_corner(
		textbox_rect.min_corner - tile_dimensions, 
		tile_dimensions);
	rect destination_rect = first_destination_rect;

	for (u32 y = 0; y <= tile_y_count + 1; y++)
	{
		destination_rect = move_rect(first_destination_rect, get_v2(0, y * tile_dimensions.y));

		if (y == 0)
		{
			for (u32 x = 0; x <= tile_x_count + 1; x++)
			{
				rect source_bitmap;
				if (x == 0)
				{
					source_bitmap = static_data->ui_gfx.msgbox_frame_upper_left;
				}
				else if (x == tile_x_count + 1)
				{
					source_bitmap = static_data->ui_gfx.msgbox_frame_upper_right;
				}
				else
				{
					source_bitmap = static_data->ui_gfx.msgbox_frame_upper;
				}

				render_bitmap(group, textures::CHARSET, source_bitmap, destination_rect);
				move_rect(&destination_rect, get_v2(tile_dimensions.x, 0));
			}
		}
		else if (y == tile_y_count + 1)
		{
			for (u32 x = 0; x <= tile_x_count + 1; x++)
			{
				rect source_bitmap;
				if (x == 0)
				{
					source_bitmap = static_data->ui_gfx.msgbox_frame_lower_left;
				}
				else if (x == tile_x_count + 1)
				{
					source_bitmap = static_data->ui_gfx.msgbox_frame_lower_right;
				}
				else
				{
					source_bitmap = static_data->ui_gfx.msgbox_frame_lower;
				}

				render_bitmap(group, textures::CHARSET, source_bitmap, destination_rect);
				move_rect(&destination_rect, get_v2(tile_dimensions.x, 0));
			}
		}
		else
		{
			render_bitmap(group, textures::CHARSET, static_data->ui_gfx.msgbox_frame_left, destination_rect);

			destination_rect = move_rect(destination_rect, get_v2((tile_x_count + 1) * tile_dimensions.x, 0));
			render_bitmap(group, textures::CHARSET, static_data->ui_gfx.msgbox_frame_right, destination_rect);
		}
	}
}

scene_change game_update_and_render(game_state* game, level_state* level, r32 delta_time)
{
	if (false == level->current_map_initialized)
	{
		initialize_current_map(game, level);
	}

	entity* player = get_player(level);

	entity* debug_entity_to_render_path = 0;

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
		if (level->player_invincibility_cooldown > 0.0f)
		{
			level->player_invincibility_cooldown -= delta_time;
			//printf("niezniszczalnosc jeszcze przez %.02f\n", level->player_invincibility_cooldown);
		}

		update_power_up_timers(level, delta_time);
		if (is_power_up_active(level->power_ups.damage))
		{
			player->type->fired_bullet_type = &level->static_data->bullet_types[2];
		}
		else
		{
			player->type->fired_bullet_type = &level->static_data->bullet_types[0];
		}

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
				printf(message.ptr); printf("\n");	
				update = false;
				text_to_show = collision.collided_message_display->type->message;
				collision.collided_message_display->type->message = {};
			}
		}

		if (collision.collided_transition 
			&& false == level->active_scene_change.change_scene)
		{
			level->active_scene_change.change_scene = true;
			level->active_scene_change.new_scene = scene::GAME;
			level->active_scene_change.map_to_load = level->current_map.next_map;
			level->scene_fade_perc = 0.0f;
		}

		v2 player_direction_v2 = get_unit_vector(player->velocity);
		if (false == is_zero(player->velocity))
		{
			player->direction = player->velocity.x < 0 ? direction::W : direction::E;
		}
		else
		{
			// zostawiamy stary
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
		for (u32 entity_index = 1; entity_index < level->entities_count; entity_index++)
		{
			entity* entity = level->entities + entity_index;

			if (false == is_in_neighbouring_chunk(player->position.chunk_pos, entity->position))
			{
				continue;
			}

			if (entity->health < 0)
			{
				remove_entity(level, entity_index);
				entity_index--; // ze względu na działanie compact array
				add_explosion(level, entity->position, level->static_data->explosion_animations.size_48x48);
			}

			if (entity->attack_cooldown > 0)
			{
				entity->attack_cooldown -= delta_time;
			}

			if (are_entity_flags_set(entity, entity_flags::WALKS_HORIZONTALLY))
			{
				if (entity->has_walking_path)
				{
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

					if (current_goal != current_start)
					{
						v2 distance = get_position_difference(current_goal, entity->position);
						r32 distance_length = length(distance);
						v2 distance_to_start = get_position_difference(current_start, entity->position);
						r32 distance_to_start_length = length(distance_to_start);

						v2 direction = get_zero_v2();
						if (distance_length != 0)
						{
							direction = get_unit_vector(distance);
						}

						r32 velocity = entity->type->velocity_multiplier;

						r32 slowdown_threshold = 2.0f;
						r32 fudge = 0.1f;
						if (distance_length < slowdown_threshold)
						{
							velocity *= ((distance_length + fudge) / slowdown_threshold);
						}
						else if (distance_to_start_length < slowdown_threshold)
						{
							velocity *= ((distance_to_start_length + fudge) / slowdown_threshold);
						}

						entity->velocity = direction * velocity;
						if (entity->velocity.x < 0.0f)
						{
							entity->direction = direction::W;
						}
						else
						{
							entity->direction = direction::E;
						}

						world_position new_position = add_to_position(entity->position, (direction * velocity * delta_time));
						v2 movement_delta = get_position_difference(new_position, entity->position);
						entity->position = new_position;

						// sprawdzenie kolizji z graczem
						chunk_position reference_chunk = get_tile_chunk_position(get_tile_position(entity->position));
						collision new_collision = check_minkowski_collision(
							get_entity_collision_data(reference_chunk, entity),
							get_entity_collision_data(reference_chunk, player),
							movement_delta, 1.0f);
						if (new_collision.collided_wall != direction::NONE)
						{
							handle_player_and_enemy_collision(level, player, entity);
						}

						if (length(get_position_difference(current_goal, entity->position)) < 0.01f)
						{
							//entity->position = goal_pos;

							if (entity->goal_path_point == 0)
							{
								//printf("dotarliśmy do 0\n");
								entity->goal_path_point = 1;
							}
							else if (entity->goal_path_point == 1)
							{
								//printf("dotarliśmy do 1\n");
								entity->goal_path_point = 0;
							}
						}
					}
					debug_entity_to_render_path = entity;
				}
				else
				{
					tile_range new_path = find_walking_path_for_enemy(
						level->current_map, level->static_data->collision_reference, get_tile_position(entity->position));
					entity->path = new_path;
					entity->has_walking_path = true;

					// zabezpieczenie na wypadek nierównego ustawienia entity w edytorze 
					tile_position current_position = get_tile_position(entity->position);
					if (current_position.y != entity->path.start.y)
					{
						tile_position new_position = get_tile_position(current_position.x, entity->path.start.y);
						entity->position = get_world_position(new_position);
					}
				}

				tile_position tile_pos = get_tile_position(entity->position);
				//printf("enemy %d,%d x velocity: %.02f\n", tile_pos.x, tile_pos.y, entity->velocity.x);
				r32 frame_duration_modifier = 0.75f + (1.0f / length(entity->velocity));

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

				animate_entity(NULL, entity, delta_time, frame_duration_modifier);
			}

			if (are_entity_flags_set(entity, entity_flags::ENEMY)
				&& entity->type->fired_bullet_type)
			{
				if (entity->attack_cooldown <= 0)
				{
					// trochę wyżej niż środek bohatera, wygląda to naturalniej
					world_position player_target_position = add_to_position(player->position, get_v2(0, -0.5f));

					v2 player_relative_pos = get_position_difference(player_target_position, entity->position);
					v2 direction_to_player = get_unit_vector(player_relative_pos);
					entity->direction = direction_to_player.x < 0 ? direction::W : direction::E;

					if (is_point_visible_for_entity(level, entity, player_target_position, entity->type->player_detecting_distance))
					{
						tile_position tile_pos = get_tile_position(entity->position);
						printf("widoczny przez entity o wspolrzednych (%d,%d)\n", tile_pos.x, tile_pos.y);
						fire_bullet(level, entity->type->fired_bullet_type, entity->position, get_zero_v2(),
							direction_to_player * entity->type->fired_bullet_type->constant_velocity);
						entity->attack_cooldown = entity->type->default_attack_cooldown;
					}
				}
			}
		}

		// update bullets
		for (u32 bullet_index = 0; bullet_index < level->bullets_count; bullet_index++)
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
						bullet_index--; // ze względu na compact array - został usunięty bullet, ale nowy został wstawiony na jego miejsce
					}
				}

				if (is_zero(bullet->velocity))
				{
					remove_bullet(level, bullet_index);
					bullet_index--;
				}
			}
			else
			{
				remove_bullet(level, bullet_index);
				bullet_index--;

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
		chunk_position reference_chunk = player->position.chunk_pos;
		tile_position player_tile_pos = get_tile_position(player->position);
		v2 player_tile_offset_in_chunk = get_tile_offset_in_chunk(reference_chunk, player_tile_pos);
		v2 player_offset_in_chunk = get_position_difference(player->position, reference_chunk);
		v2 player_offset_in_tile = player_offset_in_chunk - player_tile_offset_in_chunk;
	
		i32 screen_half_width = ceil(HALF_SCREEN_WIDTH_IN_TILES) + 2;
		i32 screen_half_height = ceil(HALF_SCREEN_HEIGHT_IN_TILES) + 2;

		// draw tiles
		for (i32 y_coord_relative = -screen_half_height;
			y_coord_relative < screen_half_height;
			y_coord_relative++)
		{
			i32 y_coord_on_screen = y_coord_relative;
			i32 y_coord_in_world = player_tile_pos.y + y_coord_relative;

			for (i32 x_coord_relative = -screen_half_width;
				x_coord_relative < screen_half_width;
				x_coord_relative++)
			{
				i32 x_coord_on_screen = x_coord_relative;
				i32 x_coord_in_world = player_tile_pos.x + x_coord_relative;

				u32 tile_value = get_tile_value(level->current_map, x_coord_in_world, y_coord_in_world);
				rect tile_bitmap = get_tile_rect(tile_value);

				v2 position = get_v2(x_coord_on_screen, y_coord_on_screen) - player_offset_in_tile;
				rect screen_rect = get_tile_render_rect(position);
				render_bitmap(&game->render, textures::TILESET, tile_bitmap, screen_rect);

#if 0
				if (is_tile_colliding(level->static_data->collision_reference, tile_value))
				{
					tile_position tile_pos = get_tile_position(x_coord_in_world, y_coord_in_world);
					entity_collision_data tile_collision = get_tile_collision_data(player->position.chunk_pos, tile_pos);
					v2 relative_position = get_position_difference(tile_pos, player->position);
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

		if (debug_entity_to_render_path)
		{
			render_debug_path_ends(&game->render, debug_entity_to_render_path, player->position);				
		}

		// draw player 
		{
			// nogi
			render_entity_animation_frame(&game->render, player->position, player);
		
			// tułów - zależny od kierunku strzelania
			sprite sprite = level->current_player_torso;
			direction sprite_direction = direction::E;
			if (level->flip_player_torso_horizontally)
			{
				sprite_direction = direction::W;
				sprite.flip_horizontally = true;
			}

			render_entity_sprite(&game->render, player->position, player->position, player->direction,
				player->visual_effect, player->visual_effect_duration, sprite);
		}

		// draw entities
		for (u32 entity_index = 1; entity_index < level->entities_count; entity_index++)
		{
			entity* entity = level->entities + entity_index;
			if (is_in_neighbouring_chunk(player->position.chunk_pos, entity->position))
			{
				render_entity_animation_frame(&game->render, player->position, entity);
			}
		}

		// draw bullets
		for (u32 bullet_index = 0; bullet_index < level->bullets_count; bullet_index++)
		{
			bullet* bullet = level->bullets + bullet_index;
			if (is_in_neighbouring_chunk(player->position.chunk_pos, bullet->position))
			{
				render_entity_sprite(&game->render,
					player->position, bullet->position, direction::NONE,
					NULL, 0, bullet->type->idle_pose.sprite);

		// draw explosions
		for (i32 explosion_index = 0; explosion_index < level->explosions_count; explosion_index++)
		{
			entity* explosion = level->explosions + explosion_index;
			if (is_in_neighbouring_chunk(player->position.chunk_pos, explosion->position))
			{
				render_entity_animation_frame(&game->render, player->position, explosion);
			}
		}

		// draw collision debug info
		{
#if 0
			for (u32 entity_index = 0; entity_index < level->entities_count; entity_index++)
			{
				entity* entity = level->entities + entity_index;
				if (is_in_neighbouring_chunk(player->position.chunk_pos, entity->position))
				{
					// istotne - offset sprite'a nie ma tu znaczenia
					v2 relative_position = get_position_difference(entity->position, player->position);
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
	
	if (update == false && text_to_show.string_size)
	{
		rect text_area = get_rect_from_center(SCREEN_CENTER_IN_PIXELS, get_v2(100, 100));

		v2 margin = get_v2(4, 4);
		render_textbox(level->static_data, &game->render, add_side_length(text_area, margin));

		render_text(&game->render, game->transient_arena, level->static_data->ui_font, text_area, text_to_show, true);
	}

	if (level->active_scene_change.change_scene)
	{
		level->scene_fade_perc += (delta_time * 1.5f);
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

void render_menu_option(font font, game_state* game, u32 x_coord, u32 y_coord, string_ref title)
{
	rect textbox_area = get_rect_from_corners(
		get_v2(x_coord, y_coord),
		get_v2(x_coord + 100, y_coord + 20));
	
	render_text(&game->render, game->transient_arena, font, textbox_area, title);
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
	
	render_clear(&game->render, get_zero_v4());

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

	return change_to_other_scene;
};

void main_game_loop(game_state* game, static_game_data* static_data, r32 delta_time)
{		
	scene_change scene_change = {};
	switch (game->current_scene)
	{
		case scene::GAME:
		{
			if (false == game->first_game_run_initialized)
			{
				game->level_state = push_struct(game->arena, level_state);

				game->game_level_memory = begin_temporary_memory(game->arena);
				string_ref level_name = copy_c_string_to_memory_arena(game->arena, "map_01");
				initialize_level_state(game->level_state, static_data, level_name, game->arena);
				game->level_state->current_map = load_map(level_name, game->arena, game->transient_arena);

				game->first_game_run_initialized = true;
			}
						
			scene_change = game_update_and_render(game, game->level_state, delta_time);
		};
		break;
		case scene::MAIN_MENU:
		{
			scene_change = menu_update_and_render(game, static_data, delta_time);
		};
		break;
		case scene::DEATH:
		{
			// czy potrzebujemy tutaj osobnej "sceny"?
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
		switch (scene_change.new_scene)
		{
			case scene::GAME:
			{
				temporary_memory auxillary_memory_for_loading = begin_temporary_memory(game->transient_arena);
				{
					string_ref level_name = copy_string(game->transient_arena, scene_change.map_to_load);
					save* save = save_player_state(game->transient_arena, game->level_state);
					end_temporary_memory(game->game_level_memory, true);

					game->game_level_memory = begin_temporary_memory(game->arena);
					initialize_level_state(game->level_state, static_data, level_name, game->arena);
					game->level_state->current_map = load_map(level_name, game->arena, game->transient_arena);
					initialize_current_map(game, game->level_state);
					restore_player_state(game->level_state, save);
				}
				end_temporary_memory(auxillary_memory_for_loading, true);
			}
			case scene::MAIN_MENU:
			{
				
			}
			case scene::DEATH:
			{

			}
			case scene::CREDITS:
			{

			}
			break;
			invalid_default_case;
		}
	}

	render_group_to_output(&game->render);
}