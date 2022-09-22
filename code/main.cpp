#include "main.h"
#include "game_data.h"
#include "text_rendering.h"
#include "map.h"
#include "animation.h"
#include "gates.h"

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

void render_bitmap(render_group* group, temp_texture_enum texture, rect source_rect, rect destination_rect)
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
	temp_texture_enum texture, rect source_rect, rect destination_rect, v4 tint_color, b32 render_in_additive_mode, b32 flip_horizontally)
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

b32 are_flags_set(entity_flags* flags, entity_flags flag_values_to_check)
{
	b32 result = are_flags_set((u32*)flags, (u32)flag_values_to_check);
	return result;
}

void set_flags(entity_flags* flags, entity_flags flag_values_to_check)
{
	set_flags((u32*)flags, (u32)flag_values_to_check);
}

void unset_flags(entity_flags* flags, entity_flags flag_values_to_check)
{
	unset_flags((u32*)flags, (u32)flag_values_to_check);
}

b32 are_entity_flags_set(entity* entity, entity_flags flag_values)
{
	b32 result = are_flags_set(&entity->type->flags, flag_values);
	return result;
}

void render_hitpoint_bar(render_group* render, entity* player, b32 draw_white_bars)
{
	// zabezpieczenie na uint wrapping
	if (player->health < 0.0f)
	{
		player->health = 0.0f;
	}

	u32 filled_health_bars = (u32)(player->health / 10);
	u32 max_health_bars = (u32)(player->type->max_health / 10);

	v2 icon_dim = get_v2(8, 8);
	v2 bar_dim = get_v2(4, 8);

	rect icon_texture_rect = get_rect_from_dimensions(get_v2(0, 0), icon_dim);
	rect icon_screen_rect = get_rect_from_dimensions(get_v2(10, 10), icon_dim);
	
	render_bitmap(render, temp_texture_enum::UI_TEXTURE, icon_texture_rect, icon_screen_rect);
	
	rect bar_texture_rect = get_rect_from_dimensions(get_v2(4, 16), bar_dim);

	if (draw_white_bars)
	{
		bar_texture_rect = get_rect_from_dimensions(get_v2(0, 16), bar_dim);
	}

	rect bar_screen_rect = get_rect_from_dimensions(get_v2(18, 10), bar_dim);
	for (u32 health_bar_index = 0;
		health_bar_index < filled_health_bars;
		health_bar_index++)
	{
		bar_screen_rect = move_rect(bar_screen_rect, get_v2(4, 0));
		render_bitmap(render, temp_texture_enum::UI_TEXTURE, bar_texture_rect, bar_screen_rect);
	}

	bar_texture_rect = get_rect_from_dimensions(get_v2(0, 8), bar_dim);

	for (u32 health_bar_index = filled_health_bars;
		health_bar_index < max_health_bars;
		health_bar_index++)
	{
		bar_screen_rect = move_rect(bar_screen_rect, get_v2(4, 0));
		render_bitmap(render, temp_texture_enum::UI_TEXTURE, bar_texture_rect, bar_screen_rect);
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

		result = get_rect_from_dimensions(get_v2(x, y), get_v2(TILE_SIDE_IN_PIXELS, TILE_SIDE_IN_PIXELS));
	}
	
	return result;
}

b32 is_good_for_walk_path(map level, map collision_ref, u32 tile_x, u32 tile_y)
{
	b32 result = (false == is_tile_colliding(level , collision_ref, tile_x, tile_y)
		&& is_tile_colliding(level, collision_ref, tile_x, tile_y + 1));
	return result;
}

tile_range find_walking_path_for_enemy(map level, map collision_ref, tile_position start_tile)
{
	b32 found_good_start_pos = false;
	tile_position good_start_tile = {};

	tile_position test_tile = start_tile;
	i32 distance_checking_limit = 10;
	for (i32 distance = 0; distance < distance_checking_limit; distance++)
	{
		test_tile.y = start_tile.y + distance;
		if (is_good_for_walk_path(level, collision_ref, test_tile.x, test_tile.y))
		{
			good_start_tile = test_tile;
			found_good_start_pos = true;
			break;
		}
	}

	if (good_start_tile.x == 0 && good_start_tile.y == 0)
	{
		return {};
	}

	tile_position left_end = good_start_tile;
	tile_position right_end = good_start_tile;

	test_tile = good_start_tile;
	for (i32 distance = 0; distance <= distance_checking_limit; distance++)
	{
		test_tile.x = good_start_tile.x - distance;
		if (is_good_for_walk_path(level, collision_ref, test_tile.x, test_tile.y))
		{
			left_end = test_tile;
		}
		else
		{
			break;
		}
	}

	test_tile = good_start_tile;
	for (i32 distance = 0; distance <= distance_checking_limit; distance++)
	{
		test_tile.x = good_start_tile.x + distance;
		if (is_good_for_walk_path(level, collision_ref, test_tile.x, test_tile.y))
		{
			right_end = test_tile;
		}
		else
		{
			break;
		}
	}

	tile_range result = {};
	result.start = left_end;
	result.end = right_end;
	return result;
}

tile_range find_horizontal_range_of_free_tiles(map level, map collision_ref, tile_position starting_tile, u32 length_limit)
{
	tile_position left_end = starting_tile;
	tile_position right_end = starting_tile;
	tile_position test_tile = starting_tile;
	for (i32 distance = 0; distance <= length_limit; distance++)
	{
		test_tile.x = starting_tile.x - distance;
		if (false == is_tile_colliding(level, collision_ref, test_tile.x, test_tile.y))
		{
			left_end = test_tile;
		}
		else
		{
			break;
		}
	}

	test_tile = starting_tile;
	for (i32 distance = 0; distance <= length_limit; distance++)
	{
		test_tile.x = starting_tile.x + distance;
		if (false == is_tile_colliding(level, collision_ref, test_tile.x, test_tile.y))
		{
			right_end = test_tile;
		}
		else
		{
			break;
		}
	}

	tile_range result = {};
	result.start = left_end;
	result.end = right_end;
	return result;
}

tile_range find_vertical_range_of_free_tiles(map level, map collision_ref, tile_position starting_tile, u32 length_limit)
{
	tile_position upper_end = starting_tile;
	tile_position lower_end = starting_tile;
	tile_position test_tile = starting_tile;
	for (i32 distance = 0; distance <= length_limit; distance++)
	{
		test_tile.y = starting_tile.y - distance;
		if (false == is_tile_colliding(level, collision_ref, test_tile.x, test_tile.y))
		{
			upper_end = test_tile;	
		}
		else
		{
			break;
		}
	}

	test_tile = starting_tile;
	for (i32 distance = 0; distance <= length_limit; distance++)
	{
		test_tile.y = starting_tile.y + distance;
		if (false == is_tile_colliding(level, collision_ref, test_tile.x, test_tile.y))
		{
			lower_end = test_tile;
		}
		else
		{
			break;
		}
	}

	tile_range result = {};
	result.start = upper_end;
	result.end = lower_end;
	return result;
}

void fire_bullet(level_state* level, entity_type* bullet_type, world_position bullet_starting_position, 
	v2 bullet_offset, v2 velocity)
{
	if (level->bullets_count < level->bullets_max_count)
	{
		bullet* bul = &level->bullets[level->bullets_count];
		bul->type = bullet_type;
		bullet_starting_position.pos_in_chunk += bullet_offset;
		bul->position = renormalize_position(bullet_starting_position);
		bul->velocity = velocity;
		level->bullets_count++;
	}
}

void fire_bullet(level_state* level, entity* entity, b32 cooldown)
{
	assert(entity->type->fired_bullet_type);

	v2 direction = (entity->direction == direction::E
		? get_v2(1.0f, 0.0f) 
		: get_v2(-1.0f, 0.0f));
	v2 bullet_offset = (entity->direction == direction::E
		? entity->type->fired_bullet_offset 
		: get_v2(-entity->type->fired_bullet_offset.x, entity->type->fired_bullet_offset.y));

	fire_bullet(level, entity->type->fired_bullet_type, entity->position, bullet_offset,
		direction * entity->type->fired_bullet_type->constant_velocity);

	if (cooldown)
	{
		entity->attack_cooldown = entity->type->default_attack_cooldown;
	}
}

void remove_bullet(level_state* level, u32 bullet_index)
{
	assert(level->bullets_count > 0);
	assert(bullet_index < level->bullets_max_count);

	// compact array - działa też w przypadku bullet_index == bullets_count - 1
	bullet last_bullet = level->bullets[level->bullets_count - 1];
	level->bullets[bullet_index] = last_bullet;
	level->bullets_count--;
}

void update_power_up_timers(level_state* level, r32 delta_time)
{
	for (u32 index = 0; index < array_count(level->power_ups.states); index++)
	{
		power_up_state* state = &level->power_ups.states[index];
		state->time_remaining -= delta_time;
		if (state->time_remaining < 0.0f)
		{
			state->time_remaining = 0.0f;
		}
	}
}

b32 is_power_up_active(power_up_state power_up)
{
	b32 result = (power_up.time_remaining > 0.0f);
	return result;
}

void apply_power_up(level_state* level, entity* player, entity* power_up)
{
	assert(are_entity_flags_set(power_up, entity_flags::POWER_UP));
	switch (power_up->type->type_enum)
	{
		case entity_type_enum::POWER_UP_INVINCIBILITY:
		{
			level->power_ups.invincibility.time_remaining += 20.0f;
		} 
		break;
		case entity_type_enum::POWER_UP_HEALTH:
		{
			player->type->max_health += 20.0f;
			player->health = player->type->max_health;
		} 
		break;
		case entity_type_enum::POWER_UP_SPEED:
		{
			level->power_ups.speed.time_remaining += 20.0f;
		} 
		break;
		case entity_type_enum::POWER_UP_DAMAGE:
		{
			level->power_ups.damage.time_remaining += 20.0f;
		} 
		break;
		case entity_type_enum::POWER_UP_GRANADES:
		{
			printf("granaty\n");
		} 
		break;
	}

	power_up->health = -10.0f; // usuwamy obiekt
}

// nie ma znaczenia, czy sprawdzamy na osi x, czy y
b32 check_line_intersection(r32 start_coord, r32 movement_delta, r32 line_coord, r32* movement_perc)
{
	b32 result = false;

	r32 distance_to_line = line_coord - start_coord;
	if (movement_delta == 0)
	{
		result = false; // jesteśmy równolegli do ściany        
	}
	else if (start_coord == line_coord)
	{
		result = false; // stoimy dokładnie na ścianie
	}
	else
	{
		*movement_perc = distance_to_line / movement_delta;
		if (*movement_perc < 0.0f)
		{
			result = false; // ściana jest w drugą stronę
		}
		else if (*movement_perc > 1.0f)
		{
			result = false; // nie trafimy w tej klatce
		}
		else
		{
			result = true;
		}
	}
	return result;
}

// działa także, gdy zamienimy x z y
b32 check_segment_intersection(r32 movement_start_x, r32 movement_start_y,
	r32 movement_delta_x, r32 movement_delta_y,
	r32 line_x, r32 min_segment_y, r32 max_segment_y, r32* min_movement_perc)
{
	b32 result = false;
	r32 movement_perc = 0;
	if (check_line_intersection(movement_start_x, movement_delta_x, line_x, &movement_perc))
	{
		v2 movement_start = get_v2(movement_start_x, movement_start_y);
		v2 movement_delta = get_v2(movement_delta_x, movement_delta_y);
		v2 intersection_pos = movement_start + (movement_perc * movement_delta);
		// wiemy, że trafiliśmy w linię - sprawdzamy, czy mieścimy się w zakresie, który nas interesuje
		if (intersection_pos.y > min_segment_y && intersection_pos.y < max_segment_y)
		{
			result = true;
			if (*min_movement_perc > movement_perc)
			{
				*min_movement_perc = movement_perc;
			}
		}
	}
	return result;
}

entity* add_entity(level_state* level, world_position position, entity_type* type)
{
	assert(level->entities_count + 1 < level->entities_max_count);

	entity* new_entity = &level->entities[level->entities_count];
	level->entities_count++;
	new_entity->position = renormalize_position(position);
	new_entity->type = type;
	new_entity->health = type->max_health;
	return new_entity;
}

entity* add_entity(level_state* level, tile_position position, entity_type* type)
{
	entity* result = add_entity(level, get_world_position(position), type);
	return result;
}

void remove_entity(level_state* level, u32 entity_index)
{
	assert(level->entities_count > 1);
	assert(entity_index != 0);
	assert(entity_index < level->entities_max_count);

	// compact array - działa też w przypadku entity_index == entities_count - 1
	entity last_entity = level->entities[level->entities_count - 1];
	level->entities[entity_index] = last_entity;
	level->entities_count--;
}

entity* get_player(level_state* level)
{
	entity* result = &level->entities[0];
	return result;
}

b32 damage_player(level_state* level, r32 damage_amount)
{
	b32 damaged = false;
	if (level->player_invincibility_cooldown <= 0.0f
		&& false == is_power_up_active(level->power_ups.invincibility))
	{
		damaged = true;
		level->entities[0].health -= damage_amount;
		start_visual_effect(level, &level->entities[0], 1, false);
		printf("gracz dostaje %.02f obrazen, zostalo %.02f zdrowia\n", damage_amount, level->entities[0].health);
		if (level->entities[0].health < 0.0f)
		{
			// przegrywamy
			debug_breakpoint;
		}
		else
		{
			level->player_invincibility_cooldown = level->static_data->default_player_invincibility_cooldown;
		}
	}
	return damaged;
}

entity_collision_data get_entity_collision_data(chunk_position reference_chunk, entity* entity)
{
	entity_collision_data result = {};
	result.position = get_position_difference(entity->position, reference_chunk);
	result.collision_rect_dim = entity->type->collision_rect_dim;
	result.collision_rect_offset = entity->type->collision_rect_offset;
	return result;
}

entity_collision_data get_bullet_collision_data(chunk_position reference_chunk, bullet* bullet)
{
	entity_collision_data result = {};
	result.position = get_position_difference(bullet->position, reference_chunk);
	result.collision_rect_dim = bullet->type->collision_rect_dim;
	result.collision_rect_offset = bullet->type->collision_rect_offset;
	return result;
}

entity_collision_data get_tile_collision_data(chunk_position reference_chunk, tile_position tile_pos)
{
	entity_collision_data result = {};
	result.position = get_position_difference(tile_pos, reference_chunk);
	result.collision_rect_dim = get_v2(1.0f, 1.0f);
	return result;
}

collision check_minkowski_collision(
	entity_collision_data a,
	entity_collision_data b,
	v2 movement_delta, r32 min_movement_perc)
{
	collision result = {};
	result.possible_movement_perc = min_movement_perc;

	v2 relative_pos = (a.position + a.collision_rect_offset) - (b.position + b.collision_rect_offset);

	// środkiem zsumowanej figury jest (0,0,0)
	// pozycję playera traktujemy jako odległość od 0
	// 0 jest pozycją entity, z którym sprawdzamy kolizję

	v2 minkowski_dimensions = a.collision_rect_dim + b.collision_rect_dim;
	v2 min_corner = minkowski_dimensions * -0.5f;
	v2 max_corner = minkowski_dimensions * 0.5f;

	b32 west_wall = check_segment_intersection(
		relative_pos.x, relative_pos.y, movement_delta.x, movement_delta.y,
		min_corner.x, min_corner.y, max_corner.y, &result.possible_movement_perc);

	b32 east_wall = check_segment_intersection(
		relative_pos.x, relative_pos.y, movement_delta.x, movement_delta.y,
		max_corner.x, min_corner.y, max_corner.y, &result.possible_movement_perc);

	b32 north_wall = check_segment_intersection(
		relative_pos.y, relative_pos.x, movement_delta.y, movement_delta.x,
		max_corner.y, min_corner.x, max_corner.x, &result.possible_movement_perc);

	b32 south_wall = check_segment_intersection(
		relative_pos.y, relative_pos.x, movement_delta.y, movement_delta.x,
		min_corner.y, min_corner.x, max_corner.x, &result.possible_movement_perc);

	if (west_wall)
	{
		result.collided_wall = direction::W;
		result.collided_wall_normal = get_v2(-1, 0);
	}
	else if (east_wall)
	{
		result.collided_wall = direction::E;
		result.collided_wall_normal = get_v2(1, 0);
	}
	else if (north_wall)
	{
		result.collided_wall = direction::N;
		result.collided_wall_normal = get_v2(0, 1);
	}
	else if (south_wall)
	{
		result.collided_wall = direction::S;
		result.collided_wall_normal = get_v2(0, -1);
	}

	return result;
}

rect get_tiles_area_to_check_for_collision(world_position entity_position, v2 collision_rect_offset, v2 collision_rect_dim, world_position target_pos)
{
	entity_position.pos_in_chunk += collision_rect_offset;

	tile_position entity_tile = get_tile_position(entity_position);
	tile_position target_tile = get_tile_position(target_pos);

	// domyślne zachowanie casta to obcięcie części ułamkowej
	i32 x_margin = (i32)ceil(collision_rect_dim.x);
	i32 y_margin = (i32)ceil(collision_rect_dim.y);

	rect result = {};
	result.min_corner.x = (r32)min((i32)entity_tile.x - x_margin, (i32)target_tile.x - x_margin);
	result.min_corner.y = (r32)min((i32)entity_tile.y - y_margin, (i32)target_tile.y - y_margin);
	result.max_corner.x = (r32)max((i32)entity_tile.x + x_margin, (i32)target_tile.x + x_margin);
	result.max_corner.y = (r32)max((i32)entity_tile.y + y_margin, (i32)target_tile.y + y_margin);
	return result;
}

rect get_tiles_area_to_check_for_collision(entity* entity, world_position target_pos)
{
	rect result = get_tiles_area_to_check_for_collision(
		entity->position, entity->type->collision_rect_offset, entity->type->collision_rect_dim, target_pos);
	return result;
}

rect get_tiles_area_to_check_for_collision(bullet* bullet, world_position target_pos)
{
	rect result = get_tiles_area_to_check_for_collision(
		bullet->position, bullet->type->collision_rect_offset, bullet->type->collision_rect_dim, target_pos);
	return result;
}



collision_result move(level_state* level, entity* moving_entity, world_position target_pos)
{
	collision_result result = {};

	v2 movement_delta = get_position_difference(target_pos, moving_entity->position);
	chunk_position reference_chunk = get_tile_chunk_position(get_tile_position(moving_entity->position));

	if (false == is_zero(movement_delta))
	{
		r32 movement_apron = 0.001f;

		for (u32 iteration = 0; iteration < 4; iteration++)
		{
			if (is_zero(movement_delta))
			{
				break;
			}

			collision closest_collision = {};
			closest_collision.collided_wall = direction::NONE;
			closest_collision.possible_movement_perc = 1.0f;

			// collision with tiles
			{
				rect area_to_check = get_tiles_area_to_check_for_collision(moving_entity, target_pos);
				for (i32 tile_y_to_check = area_to_check.min_corner.y;
					tile_y_to_check <= area_to_check.max_corner.y;
					tile_y_to_check++)
				{
					for (i32 tile_x_to_check = area_to_check.min_corner.x;
						tile_x_to_check <= area_to_check.max_corner.x;
						tile_x_to_check++)
					{
						tile_position tile_to_check_pos = get_tile_position(tile_x_to_check, tile_y_to_check);
						u32 tile_value = get_tile_value(level->current_map, tile_x_to_check, tile_y_to_check);
						if (is_tile_colliding(level->static_data->collision_reference, tile_value))
						{
							collision new_collision = check_minkowski_collision(
								get_entity_collision_data(reference_chunk, moving_entity),
								get_tile_collision_data(reference_chunk, tile_to_check_pos),
								movement_delta, closest_collision.possible_movement_perc);

							if (new_collision.collided_wall != direction::NONE)
							{
								// paskudny hack rozwiązujący problem blokowania się na ścianie, jeśli kolidujemy z nią podczas spadania
								/*if (new_collision.collided_wall == direction::S)
								{
									u32 upper_tile_value = get_tile_value(level->current_map, tile_x_to_check, tile_y_to_check - 1);
									if (is_tile_colliding(level->collision_reference, upper_tile_value))
									{
										if ((moving_entity->position - tile_to_check_pos.x > 0)
										{
											new_collision.collided_wall_normal = get_v2(-1, 0);
										}
										else
										{
											new_collision.collided_wall_normal = get_v2(1, 0);
										}
									}
								}*/

								if (new_collision.possible_movement_perc < closest_collision.possible_movement_perc)
								{
									closest_collision = new_collision;
								}
							}
						}
					}
				}
			}

			collision closest_tile_collision = closest_collision;

			// collision with entities
			{
				for (u32 entity_index = 0; entity_index < level->entities_count; entity_index++)
				{
					entity* entity_to_check = level->entities + entity_index;
					if (entity_to_check != moving_entity)
					{
						collision new_collision = check_minkowski_collision(
							get_entity_collision_data(reference_chunk, moving_entity),
							get_entity_collision_data(reference_chunk, entity_to_check),
							movement_delta, closest_collision.possible_movement_perc);

						if (new_collision.collided_wall != direction::NONE)
						{
							// bierzemy tutaj uwagę tylko entities, z którymi kolidowalibyśmy wcześniej niż ze ścianą
							if (new_collision.possible_movement_perc < closest_tile_collision.possible_movement_perc)
							{
								if (are_entity_flags_set(moving_entity, entity_flags::PLAYER))
								{
									if (are_entity_flags_set(entity_to_check, entity_flags::ENEMY))
									{
										result.collided_enemy = entity_to_check;
									}

									if (are_entity_flags_set(entity_to_check, entity_flags::SWITCH))
									{
										result.collided_switch = entity_to_check;
									}

									if (entity_to_check->type->type_enum == entity_type_enum::NEXT_LEVEL_TRANSITION)
									{
										result.collided_transition = entity_to_check;
									}

									if (are_entity_flags_set(entity_to_check, entity_flags::POWER_UP))
									{
										result.collided_power_up = entity_to_check;
									}
								}
							}

							if (are_entity_flags_set(entity_to_check, entity_flags::BLOCKS_MOVEMENT))
							{
								closest_collision = new_collision;
							}
						}
					}
				}
			}

			// przesuwamy się o tyle, o ile możemy
			if ((closest_collision.possible_movement_perc - movement_apron) > 0.0f)
			{
				v2 possible_movement = movement_delta * (closest_collision.possible_movement_perc - movement_apron);
				moving_entity->position = add_to_position(moving_entity->position, possible_movement);
				// pozostałą deltę zmniejszamy o tyle, o ile się poruszyliśmy
				movement_delta -= possible_movement;
				//printf("przesuniecie o (%.04f, %.04f)\n", possible_movement.x, possible_movement.y);
			}
			
			result.collision_data = closest_collision;

			if (false == is_zero(closest_collision.collided_wall_normal))
			{
				v2 wall_normal = closest_collision.collided_wall_normal;
				v2 movement_delta_orig = movement_delta;

				// i sprawdzamy, co zrobić z pozostałą deltą - czy możemy się poruszyć wzdłuż ściany lub odbić
				i32 how_many_times_subtract = 1; // 1 dla ślizgania się, 2 dla odbijania    
				v2 bounced = wall_normal * inner(wall_normal, moving_entity->velocity);
				moving_entity->velocity -= how_many_times_subtract * bounced;

				movement_delta -= how_many_times_subtract * (wall_normal * inner(movement_delta, wall_normal));
			}
			else
			{
				// jeśli nie było kolizji, nie ma potrzeby kolejnych iteracji
				break;
			}
		}
	}

	return result;
}

b32 move_bullet(level_state* level, bullet* moving_bullet, u32 bullet_index, world_position target_pos)
{
	b32 was_collision = false;
	
	v2 movement_delta = get_position_difference(target_pos, moving_bullet->position);
	chunk_position reference_chunk = get_tile_chunk_position(get_tile_position(moving_bullet->position));

	if (false == is_zero(movement_delta))
	{
		r32 movement_apron = 0.001f;

		collision closest_collision = {};
		closest_collision.collided_wall = direction::NONE;
		closest_collision.possible_movement_perc = 1.0f;
		entity* collided_entity = NULL;

		// collision with tiles
		{
			rect area_to_check = get_tiles_area_to_check_for_collision(moving_bullet, target_pos);
			for (i32 tile_y_to_check = area_to_check.min_corner.y;
				tile_y_to_check <= area_to_check.max_corner.y;
				tile_y_to_check++)
			{
				for (i32 tile_x_to_check = area_to_check.min_corner.x;
					tile_x_to_check <= area_to_check.max_corner.x;
					tile_x_to_check++)
				{
					tile_position tile_to_check_pos = get_tile_position(tile_x_to_check, tile_y_to_check);
					u32 tile_value = get_tile_value(level->current_map, tile_x_to_check, tile_y_to_check);
					if (is_tile_colliding(level->static_data->collision_reference, tile_value))
					{
						collision new_collision = check_minkowski_collision(
							get_bullet_collision_data(reference_chunk, moving_bullet),
							get_tile_collision_data(reference_chunk, tile_to_check_pos),
							movement_delta, closest_collision.possible_movement_perc);

						if (new_collision.collided_wall != direction::NONE)
						{
							if (new_collision.possible_movement_perc < closest_collision.possible_movement_perc)
							{
								closest_collision = new_collision;
							}
						}
					}
				}
			}
		}

		// collision with entities
		{
			for (u32 entity_index = 0; entity_index < level->entities_count; entity_index++)
			{
				entity* entity_to_check = level->entities + entity_index;
				if (are_entity_flags_set(entity_to_check, entity_flags::BLOCKS_MOVEMENT))
				{
					collision new_collision = check_minkowski_collision(
						get_bullet_collision_data(reference_chunk, moving_bullet),
						get_entity_collision_data(reference_chunk, entity_to_check),
						movement_delta, closest_collision.possible_movement_perc);

					if (new_collision.collided_wall != direction::NONE)
					{
						if (are_flags_set(&moving_bullet->type->flags, entity_flags::DAMAGES_PLAYER))
						{
							if (entity_index == 0)
							{
								// mamy gracza							
								if (new_collision.possible_movement_perc < closest_collision.possible_movement_perc)
								{
									closest_collision = new_collision;
									collided_entity = entity_to_check;
								}
							}
						}
						else
						{
							if (entity_index != 0)
							{
								// mamy przeciwnika
								if (new_collision.possible_movement_perc < closest_collision.possible_movement_perc)
								{
									closest_collision = new_collision;
									collided_entity = entity_to_check;
								}
							}
						}
					}
				}
			}
		}

		was_collision = (closest_collision.collided_wall != direction::NONE);

		if ((closest_collision.possible_movement_perc - movement_apron) > 0.0f)
		{
			v2 possible_movement = movement_delta * (closest_collision.possible_movement_perc - movement_apron);
			moving_bullet->position = add_to_position(moving_bullet->position, possible_movement);
			movement_delta -= possible_movement;
		}

		if (collided_entity)
		{
			if (are_entity_flags_set(collided_entity, entity_flags::PLAYER))
			{
				damage_player(level, moving_bullet->type->damage_on_contact);
			}
			else
			{
				if (false == are_entity_flags_set(collided_entity, entity_flags::INDESTRUCTIBLE))
				{
					start_visual_effect(level, collided_entity, 1, false);
					collided_entity->health -= moving_bullet->type->damage_on_contact;
					printf("pocisk trafil w entity, %.2f obrazen, zostalo %.2f\n",
						moving_bullet->type->damage_on_contact, collided_entity->health);
				}
			}
		}

		if (was_collision)
		{
			remove_bullet(level, bullet_index);
		}
	}

	return was_collision;
}

b32 is_standing_on_ground(level_state* level, entity* entity_to_check)
{
	b32 result = false;
	r32 corner_distance_apron = 0.0f;
	r32 max_distance_to_tile = 0.05f;

	entity test_entity = *entity_to_check;
	world_position target_pos = add_to_position(test_entity.position, get_v2(0.0f, 0.1f));
	collision_result collision = move(level, &test_entity, target_pos);
	if (collision.collided_enemy 
		|| collision.collided_switch 
		|| collision.collision_data.collided_wall == direction::S)
	{
		result = true;
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

	font font = {};
	font.pixel_height = 8;
	font.pixel_width = 8;

	render_text(&game->render, game->transient_arena, font, area, buffer, 200, true);
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

void change_movement_mode(player_movement* movement, movement_mode mode)
{
	movement->previous_mode = movement->current_mode;
	movement->previous_mode_frame_duration = movement->frame_duration;
	movement->current_mode = mode;
	movement->frame_duration = 0;

	//debug
	switch (mode)
	{
		case movement_mode::WALK:
			printf("switch to WALK after %d frames\n", movement->previous_mode_frame_duration);
			break;
		case movement_mode::JUMP:
			printf("switch to JUMP after %d frames\n", movement->previous_mode_frame_duration);
			break;
		case movement_mode::RECOIL:
			printf("switch to RECOIL after %d frames\n", movement->previous_mode_frame_duration);
			break;
	}
}

world_position process_input(level_state* level, entity* player, r32 delta_time)
{
	game_input* input = get_last_frame_input(&level->input);

	b32 is_standing_at_frame_beginning = is_standing_on_ground(level, player);

	v2 gravity = get_v2(0, 1.0f);

	// zmiana statusu
	switch (level->player_movement.current_mode)
	{
		case movement_mode::WALK:
		{
			if (false == is_standing_at_frame_beginning)
			{
				change_movement_mode(&level->player_movement, movement_mode::JUMP);
			}
		}
		break;
		case movement_mode::JUMP:
		{
			if (is_standing_at_frame_beginning)
			{
				change_movement_mode(&level->player_movement, movement_mode::WALK);
			}
		}
		break;
		case movement_mode::RECOIL:
		{
			// czy odzyskujemy kontrolę?
			if (is_standing_at_frame_beginning)
			{
				if (level->player_movement.recoil_timer > 0.0f)
				{
					// nie
					level->player_movement.recoil_timer -= delta_time;
				}
				else
				{
					// tak
					change_movement_mode(&level->player_movement, movement_mode::WALK);
				}
			}
			else
			{
				// w tym wypadku po prostu lecimy dalej
			}
		}
		break;
	}

	level->player_movement.frame_duration++;

	if (player->attack_cooldown > 0)
	{
		player->attack_cooldown -= delta_time;
	}

	// przetwarzanie inputu
	player->acceleration = get_zero_v2();
	switch (level->player_movement.current_mode)
	{
		case movement_mode::WALK:
		{
			// ułatwienie dla gracza - jeśli gracz nacisnął skok w ostatnich klatkach skoku, wykonujemy skok i tak
			if (level->player_movement.frame_duration == 1)
			{
				if (is_standing_at_frame_beginning
					&& level->player_movement.previous_mode == movement_mode::JUMP)
				{
					if (was_up_key_pressed_in_last_frames(&level->input, 3))
					{
						player->acceleration += get_v2(0, -30);
						change_movement_mode(&level->player_movement, movement_mode::JUMP);
						printf("ulatwienie!\n");
						break;
					}
				}
			}

			if (input->is_left_mouse_key_held)
			{
				if (player->attack_cooldown <= 0)
				{
					fire_bullet(level, player, true);
				}
			}

			if (input->up.number_of_presses > 0)
			{
				if (is_standing_at_frame_beginning)
				{
					player->acceleration += get_v2(0, -30);
					change_movement_mode(&level->player_movement, movement_mode::JUMP);
					break;
				}
			}

			if (input->left.number_of_presses > 0)
			{
				player->acceleration += get_v2(-1, 0);
			}

			if (input->right.number_of_presses > 0)
			{
				player->acceleration += get_v2(1, 0);
			}

#if 0
			if (input->up.number_of_presses > 0)
			{
				player->acceleration += get_v2(0, -1);
			}

			if (input->down.number_of_presses > 0)
			{
				player->acceleration += get_v2(0, 1);
			}
#endif
		}
		break;
		case movement_mode::JUMP:
		{
			player->acceleration = gravity;

			if (input->is_left_mouse_key_held)
			{
				if (player->attack_cooldown <= 0)
				{
					fire_bullet(level, player, true);
				}
			}

			if (input->left.number_of_presses > 0)
			{
				player->acceleration += get_v2(-0.5f, 0);
			}

			if (input->right.number_of_presses > 0)
			{
				player->acceleration += get_v2(0.5f, 0);
			}
		}
		break;
		case movement_mode::RECOIL:
		{
			if (false == is_standing_at_frame_beginning)
			{
				player->acceleration = gravity;
			}

			if (level->player_movement.recoil_acceleration_timer > 0.0f)
			{
				level->player_movement.recoil_acceleration_timer -= delta_time;
				player->acceleration += level->player_movement.recoil_acceleration;
			}
		}
		break;
	}

	if (is_power_up_active(level->power_ups.invincibility))
	{
		player->acceleration.x *= 0.5f;
	}

	if (is_power_up_active(level->power_ups.speed))
	{
		player->acceleration.x *= 2.0f;
	}

	player->velocity = player->type->slowdown_multiplier *
		(player->velocity + (player->acceleration * delta_time));
	
	world_position target_pos = add_to_position(player->position,
		player->velocity * player->type->velocity_multiplier * delta_time);

	return target_pos;
}

rect get_render_rect(v2 position_relative_to_camera, v2 rect_size)
{
	r32 w = rect_size.x;
	r32 h = rect_size.y;
	r32 x = SCREEN_CENTER_IN_PIXELS.x + (position_relative_to_camera.x * TILE_SIDE_IN_PIXELS)
		- (rect_size.x / 2);
	r32 y = SCREEN_CENTER_IN_PIXELS.y + (position_relative_to_camera.y * TILE_SIDE_IN_PIXELS)
		- (rect_size.y / 2);
	
	rect result = get_rect_from_dimensions(x, y, w, h);
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

	rect result = get_rect_from_dimensions(x, y, w, h);
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
		if (entity_direction != part->default_direction)
		{
			flip = true;
			offset = get_v2(-part->offset_in_pixels.x, part->offset_in_pixels.y);
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
			if (part->texture == temp_texture_enum::BULLETS_TEXTURE)
			{
				debug_breakpoint;
			}

			render_bitmap_with_effects(render, part->texture, part->texture_rect, screen_rect, get_zero_v4(), false, flip);
		}
	}
}

void add_next_level_transition(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn)
{
	entity_type* transition_type = push_struct(arena, entity_type);
	transition_type->type_enum = entity_type_enum::NEXT_LEVEL_TRANSITION;

	tile_range occupied_tiles = find_vertical_range_of_free_tiles(
		level->current_map, level->static_data->collision_reference, new_entity_to_spawn->position, 20);
	transition_type->collision_rect_dim = get_length_from_tile_range(occupied_tiles);

	world_position new_position = add_to_position(
		get_world_position(occupied_tiles.start),
		get_position_difference(occupied_tiles.end, occupied_tiles.start) / 2);

	set_flags(&transition_type->flags, entity_flags::INDESTRUCTIBLE);

	add_entity(level, new_position, transition_type);
}

void initialize_current_map(game_state* game, level_state* level)
{
	assert(false == level->current_map_initialized);

	temporary_memory memory_for_initialization = begin_temporary_memory(game->transient_arena);

	add_entity(level, level->current_map.starting_tile,
		get_entity_type_ptr(level->static_data->entity_types_dict, entity_type_enum::PLAYER));

	level->gates_dict.entries_count = 100;
	level->gates_dict.entries = push_array(game->arena, level->gates_dict.entries_count, gate_dictionary_entry);

	level->gate_tints_dict.sprite_effects_count = 100;
	level->gate_tints_dict.sprite_effects = push_array(game->arena, level->gate_tints_dict.sprite_effects_count, sprite_effect*);
	level->gate_tints_dict.probing_jump = 7;

	for (u32 entity_index = 0;
		entity_index < level->current_map.entities_to_spawn_count;
		entity_index++)
	{
		entity_to_spawn* new_entity = level->current_map.entities_to_spawn + entity_index;
		switch (new_entity->type)
		{
			case entity_type_enum::GATE:
			{
				add_gate_entity(level, game->arena, new_entity, false);
			}
			break;
			case entity_type_enum::SWITCH:
			{
				add_gate_entity(level, game->arena, new_entity, true);
			}
			break;
			case entity_type_enum::NEXT_LEVEL_TRANSITION:
			{
				add_next_level_transition(level, game->arena, new_entity);
			}
			break;
			case entity_type_enum::UNKNOWN:
			{
				// ignorujemy
			}
			break;
			default:
			{
				add_entity(level, get_world_position(new_entity->position),
					get_entity_type_ptr(level->static_data->entity_types_dict, new_entity->type));
			}
			break;
		}
	}

	end_temporary_memory(memory_for_initialization);

	level->current_map_initialized = true;
}

void handle_player_and_enemy_collision(level_state* level, entity* player, entity* enemy)
{
	if (is_power_up_active(level->power_ups.invincibility))
	{
		enemy->health -= 50.0f;
	}
	else
	{
		damage_player(level, enemy->type->damage_on_contact);

		v2 direction = get_unit_vector(
			get_position_difference(player->position, enemy->position));

		r32 acceleration = enemy->type->player_acceleration_on_collision;

		level->player_movement.recoil_timer = 2.0f;
		level->player_movement.recoil_acceleration_timer = 1.0f;
		level->player_movement.recoil_acceleration = (direction * acceleration);

		printf("odrzut! nowe przyspieszenie: (%.02f,%.02f)\n",
			level->player_movement.recoil_acceleration.x,
			level->player_movement.recoil_acceleration.y);

		change_movement_mode(&level->player_movement, movement_mode::RECOIL);
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

	// update player
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

		world_position target_pos = process_input(level, player, delta_time);

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
	for (u32 entity_index = 1; entity_index < level->entities_count; entity_index++)
	{
		entity* entity = level->entities + entity_index;

		if (false == is_in_neighbouring_chunk(player->position.chunk_pos, entity->position))
		{
			continue;
		}

		animate_entity(NULL, entity, delta_time);

		if (entity->health < 0)
		{
			remove_entity(level, entity_index);
			entity_index--; // ze względu na działanie compact array
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
		}

		if (are_entity_flags_set(entity, entity_flags::ENEMY)
			&& entity->type->fired_bullet_type)
		{
			if (entity->attack_cooldown <= 0)
			{
				v2 player_relative_pos = get_position_difference(player->position, entity->position);
				r32 distance_to_player = length(player_relative_pos);
				if (distance_to_player < entity->type->player_detecting_distance)
				{
					v2 direction_to_player = get_unit_vector(player_relative_pos);
					entity->direction = direction_to_player.x < 0 ? direction::W : direction::E;
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
				render_bitmap(&game->render, temp_texture_enum::TILESET_TEXTURE, tile_bitmap, screen_rect);

#if 1
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

		// draw entities
		for (u32 entity_index = 0; entity_index < level->entities_count; entity_index++)
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
			}
		}

		// draw collision debug info
		{
#if 1
			for (u32 entity_index = 0; entity_index < level->entities_count; entity_index++)
			{
				entity* entity = level->entities + entity_index;
				if (is_in_neighbouring_chunk(player->position.chunk_pos, entity->position))
				{
					if (false == is_zero(entity->type->color))
					{
						debug_breakpoint;
					}

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

		render_hitpoint_bar(&game->render, player, is_power_up_active(level->power_ups.invincibility));
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

void render_menu_option(game_state* game, u32 x_coord, u32 y_coord, string_ref title)
{
	rect textbox_area = get_rect_from_corners(
		get_v2(x_coord, y_coord),
		get_v2(x_coord + 100, y_coord + 20));

	font font = {};
	font.pixel_height = 8;
	font.pixel_width = 8;
	render_text(&game->render, game->transient_arena, font, textbox_area, title);
}

scene_change menu_update_and_render(game_state* game, static_game_data* static_data, input_buffer* input_buffer, r32 delta_time)
{
	scene_change change_to_other_scene = {};

	game_input* input = get_last_frame_input(input_buffer);

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

	render_menu_option(game, options_x, first_option_y, static_data->menu_new_game_str);
	render_menu_option(game, options_x, first_option_y + option_spacing, static_data->menu_continue_str);
	render_menu_option(game, options_x, first_option_y + 2 * option_spacing, static_data->menu_credits_str);
	render_menu_option(game, options_x, first_option_y + 3 * option_spacing, static_data->menu_exit_str);

	u32 indicator_y = first_option_y + (menu_index * option_spacing) - 4;
	u32 indicator_x = options_x - 20;

	rect indicator_screen_rect = get_rect_from_dimensions(indicator_x, indicator_y, 16, 16);
	rect bitmap_rect = get_rect_from_dimensions(16, 0, 16, 16);

	render_bitmap(&game->render, temp_texture_enum::MISC_TEXTURE, bitmap_rect, indicator_screen_rect);

	return change_to_other_scene;
};

save* save_player_state(memory_arena* arena, level_state* level)
{
	assert(level->entities[0].type);
	save* result = push_struct(arena, save);
	result->map_name = copy_string(arena, level->current_map_name);
	result->granades_count = 0;
	result->player_max_health = level->entities[0].type->max_health;
	return result;
}

void restore_player_state(level_state* level, save* save)
{
	assert(level->current_map_initialized);
	assert(level && save);
	level->entities[0].type->max_health = save->player_max_health;
}

void main_game_loop(game_state* game, static_game_data* static_data, input_buffer* input_buffer, r32 delta_time)
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
				initialize_level_state(game->level_state, static_data, input_buffer, level_name, game->arena);
				game->level_state->current_map = load_map(level_name, game->arena, game->transient_arena);

				game->first_game_run_initialized = true;
			}
						
			game->level_state->input = *(input_buffer);
			scene_change = game_update_and_render(game, game->level_state, delta_time);

		};
		break;
		case scene::MAIN_MENU:
		{
			scene_change = menu_update_and_render(game, static_data, input_buffer, delta_time);
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
					initialize_level_state(game->level_state, static_data, input_buffer, level_name, game->arena);
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