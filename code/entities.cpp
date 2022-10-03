#include "main.h"
#include "entities.h"
#include "map.h"
#include "gates.h"
#include "collision.h"
#include "player.h"

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

entity* add_entity(level_state* level, world_position position, entity_type* type)
{
	assert(type);

	entity* new_entity = NULL;
	if (level->entities_count < level->entities_max_count)
	{
		new_entity = &level->entities[level->entities_count];
		level->entities_count++;
		new_entity->position = renormalize_position(position);
		new_entity->type = type;
		new_entity->health = type->max_health;
		new_entity->direction = direction::W;
		new_entity->used = true;
	}
	
	return new_entity;
}

entity* add_entity(level_state* level, tile_position position, entity_type* type)
{
	entity* result = add_entity(level, get_world_position(position), type);
	return result;
}

void remove_entity(entity* entity_to_remove)
{
	*entity_to_remove = {};
}

entity_type_dictionary create_entity_types_dictionary(memory_arena* arena)
{
	entity_type_dictionary result = {};
	result.type_ptrs_count = (i32)entity_type_enum::_LAST + 1;
	result.type_ptrs = push_array(arena, result.type_ptrs_count, entity_type*);
	return result;
}

void set_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type, entity_type* enity_type_ptr)
{
	assert((i32)type < dictionary.type_ptrs_count);
	assert(dictionary.type_ptrs[(i32)type] == NULL);
	dictionary.type_ptrs[(i32)type] = enity_type_ptr;
}

entity_type* get_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type)
{
	assert((i32)type < dictionary.type_ptrs_count);
	entity_type* result = dictionary.type_ptrs[(i32)type];
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

void remove_bullet(level_state* level, i32* bullet_index)
{
	if (level->bullets_count > 0)
	{
		assert(*bullet_index >= 0);
		assert(*bullet_index < level->bullets_max_count);

		// compact array
		bullet* last_bullet = &level->bullets[level->bullets_count - 1];
		level->bullets[*bullet_index] = *last_bullet;
		level->bullets_count--;
		*last_bullet = {}; // czyszczenie

		if (*bullet_index > 0)
		{
			(*bullet_index)--;
		}
	}
}

tile_range find_walking_path(map level, map collision_ref, tile_position start_tile)
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

void find_walking_path_for_enemy(level_state* level, entity* entity)
{
	tile_position starting_position = get_tile_position(entity->position);
	tile_range new_path = find_walking_path(
		level->current_map, level->static_data->collision_reference, starting_position);

	// na razie sprawdziliśmy same pola - powinniśmy sprawdzić jeszcze kolizję z bramami i przełącznikami
	tile_range first_part = get_tile_range(starting_position, new_path.start);
	tile_range second_part = get_tile_range(starting_position, new_path.end);
	first_part = find_path_fragment_not_blocked_by_entities(level, first_part);
	second_part = find_path_fragment_not_blocked_by_entities(level, second_part);

	new_path = get_tile_range(first_part.end, second_part.end);

	entity->path = new_path;
	entity->has_walking_path = true;
}

void find_flying_path_for_enemy(level_state* level, entity* entity, b32 vertical)
{
	u32 length_limit = 15;
	tile_position starting_position = get_tile_position(entity->position);
	tile_range new_path;
	
	if (vertical)
	{
		new_path = find_vertical_range_of_free_tiles(
			level->current_map, level->static_data->collision_reference, starting_position, length_limit);
	}
	else
	{
		new_path = find_horizontal_range_of_free_tiles(
			level->current_map, level->static_data->collision_reference, starting_position, length_limit);
	}
	
	tile_range first_part = get_tile_range(starting_position, new_path.start);
	tile_range second_part = get_tile_range(starting_position, new_path.end);
	first_part = find_path_fragment_not_blocked_by_entities(level, first_part);
	second_part = find_path_fragment_not_blocked_by_entities(level, second_part);

	new_path = get_tile_range(first_part.end, second_part.end);

	entity->path = new_path;
	entity->has_walking_path = true;

	printf("nowe latanie: od (%d, %d) do (%d, %d)\n",
		new_path.start.x, new_path.start.y, new_path.end.x, new_path.end.y);
}

// domyślnie trójkąt jest zwrócony podstawą w prawo
// można podać x i y odwrotnie dla trójkątów zwróconych podstawą w górę lub w dół
b32 is_point_within_right_triangle(r32 triangle_height, r32 relative_x, r32 relative_y, b32 invert_sign)
{
	b32 result = false;
	relative_x = (invert_sign ? -relative_x : relative_x);
	if (relative_x < triangle_height)
	{
		// trzeba pamiętać, że y idą do dołu ekranu
		r32 max_y = relative_x; // bok wyrażony f(x) = x
		r32 min_y = -relative_x; // bok wyrażony f(x) = -x
		if (relative_y <= max_y && relative_y >= min_y)
		{
			// znajdujemy się w trójkącie
			result = true;
		}
	}

	return result;
}

b32 is_point_visible_within_90_degrees(world_position looking_point, direction looking_direction, r32 max_looking_distance, world_position point_to_check)
{
	assert(looking_direction != direction::NONE);

	// sprawdzamy trójkąt z wierzchołkiem umiejscowionym w patrzącym entity
	// kąt przy tym wierzchołku jest prosty
	// a więc boki trójkąta można określić funkcjami f(x)=x, f(x)=-1
	b32 result = false;
	r32 triangle_height = max_looking_distance;

	v2 relative_pos = get_position_difference(point_to_check, looking_point);
	if (looking_direction == direction::E && relative_pos.x > 0.0f)
	{
		result = is_point_within_right_triangle(triangle_height, relative_pos.x, relative_pos.y, false);
	}
	else
	if (looking_direction == direction::W && relative_pos.x < 0.0f)
	{
		result = is_point_within_right_triangle(triangle_height, relative_pos.x, relative_pos.y, true);
	}
	else // odwracamy x i y
	if (looking_direction == direction::N && relative_pos.y > 0.0f)
	{
		result = is_point_within_right_triangle(triangle_height, relative_pos.y, relative_pos.x, false);
	}
	else
	if (looking_direction == direction::S && relative_pos.y < 0.0f)
	{
		result = is_point_within_right_triangle(triangle_height, relative_pos.y, relative_pos.x, true);
	}

	return result;
}

b32 is_point_visible_for_entity(level_state* level, entity* looking_entity, world_position point)
{
	b32 result = false;
	world_position looking_point = add_to_position(looking_entity->position, looking_entity->type->looking_position_offset);
	if (looking_entity->type->detection_type != detection_type::DETECT_NOTHING)
	{
		v2 distance_to_point = get_position_difference(point, looking_point);
		if (length(distance_to_point) <= looking_entity->type->detection_distance)
		{
			switch (looking_entity->type->detection_type)
			{
				case detection_type::DETECT_180_DEGREES_BELOW:
				{
					if (distance_to_point.y < 0)
					{
						result = true;
					}
				}
				break;
				case detection_type::DETECT_180_DEGREES_IN_FRONT:
				{
					if (looking_entity->direction == direction::W)
					{
						if (distance_to_point.x <= 0)
						{
							result = true;
						}
					}
					else
					{
						if (distance_to_point.x >= 0)
						{
							result = true;
						}
					}					
				}
				break;
				case detection_type::DETECT_360_DEGREES:
				{
					result = true;
				}
				break;
				case detection_type::DETECT_90_DEGREES_IN_FRONT:
				{
					result = is_point_visible_within_90_degrees(looking_point, looking_entity->direction,
						looking_entity->type->detection_distance, point);
				}
				break;
			}
		}

		if (result)
		{
			if (check_if_sight_line_is_obstructed(level, looking_point, point))
			{
				result = false;
			}
		}
	}

	return result;
};

void add_next_level_transition(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn)
{
	entity_type* transition_type = push_struct(arena, entity_type);
	transition_type->type_enum = entity_type_enum::NEXT_LEVEL_TRANSITION;

	tile_range occupied_tiles = find_vertical_range_of_free_tiles(
		level->current_map, level->static_data->collision_reference, new_entity_to_spawn->position, 20);
	transition_type->collision_rect_dim = get_collision_dim_from_tile_range(occupied_tiles);

	world_position new_position = add_to_position(
		get_world_position(occupied_tiles.start),
		get_position_difference(occupied_tiles.end, occupied_tiles.start) / 2);

	set_flags(&transition_type->flags, entity_flags::INDESTRUCTIBLE);

	add_entity(level, new_position, transition_type);
}

void add_message_display_entity(level_state* level, memory_arena* arena, entity_to_spawn* new_entity_to_spawn)
{
	entity_type* new_type = push_struct(arena, entity_type);

	u32 max_size = 10;

	tile_range occupied_tiles = find_vertical_range_of_free_tiles(
		level->current_map, level->static_data->collision_reference, new_entity_to_spawn->position, max_size);
	v2 collision_rect_dim = get_collision_dim_from_tile_range(occupied_tiles);

	new_type->collision_rect_dim = collision_rect_dim;
	new_type->message = new_entity_to_spawn->message;

	world_position new_position = add_to_position(
		get_world_position(occupied_tiles.start),
		get_position_difference(occupied_tiles.end, occupied_tiles.start) / 2);

	set_flags(&new_type->flags, entity_flags::INDESTRUCTIBLE);
	set_flags(&new_type->flags, entity_flags::MESSAGE_DISPLAY);

	entity* new_entity = add_entity(level, new_position, new_type);
}

void initialize_current_map(game_state* game, level_state* level)
{
	assert(false == level->current_map_initialized);

	//temporary_memory memory_for_initialization = begin_temporary_memory(game->transient_arena);

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
			case entity_type_enum::GATE_BLUE:
			case entity_type_enum::GATE_GREY:
			case entity_type_enum::GATE_RED:
			case entity_type_enum::GATE_GREEN:
			{
				add_gate_entity(level, game->arena, new_entity, false);
			}
			break;
			case entity_type_enum::SWITCH_BLUE:
			case entity_type_enum::SWITCH_GREY:
			case entity_type_enum::SWITCH_RED:
			case entity_type_enum::SWITCH_GREEN:
			{
				add_gate_entity(level, game->arena, new_entity, true);
			}
			break;
			case entity_type_enum::NEXT_LEVEL_TRANSITION:
			{
				add_next_level_transition(level, game->arena, new_entity);
			}
			break;
			case entity_type_enum::MESSAGE_DISPLAY:
			{
				add_message_display_entity(level, game->arena, new_entity);
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

	//end_temporary_memory(memory_for_initialization);

	level->current_map_initialized = true;
}

entity* add_explosion(level_state* level, world_position position, animation* explosion_animation)
{
	assert(explosion_animation);
	
	entity* new_explosion = NULL;
	if (level->explosions_count < level->explosions_max_count)
	{
		new_explosion = &level->explosions[level->explosions_count];
		level->explosions_count++;
		new_explosion->position = renormalize_position(position);
		new_explosion->current_animation = explosion_animation;
		new_explosion->animation_duration = 0.0f;
	}
	return new_explosion;
}

void remove_explosion(level_state* level, i32* explosion_index)
{
	if (level->explosions_count > 0)
	{
		assert(*explosion_index >= 0);
		assert(*explosion_index < level->explosions_max_count);

		// compact array - działa też w przypadku explosion_index == explosions_count - 1
		entity* last_explosion = &level->explosions[level->explosions_count - 1];
		level->explosions[*explosion_index] = *last_explosion;
		level->explosions_count--;
		*last_explosion = {}; // czyszczenie

		if (*explosion_index > 0)
		{
			(*explosion_index)--;
		}
	}
}

shooting_rotation get_entity_shooting_rotation(shooting_rotation_sprites* rotation_sprites, v2 shooting_direction)
{
	assert(rotation_sprites);
	shooting_rotation result = {};

	r32 angle = atan2(shooting_direction.y, shooting_direction.x) * (180 / pi32);
	angle += 45.0f / 2.0f;
	
	if (angle > 180)
	{
		angle = -360.0f + angle;
	}

	if (angle < -180)
	{
		angle = 360.0f + angle;
	}

	// przypomnienie: N jest w dół ekranu!

	direction direction = direction::NONE;
	if      (0.0f    < angle && angle <= 45.0f)   direction = direction::E;
	else if (45.0f   < angle && angle <= 90.0f)   direction = direction::NE;
	else if (90.0f   < angle && angle <= 135.0f)  direction = direction::N;
	else if (135.0f  < angle && angle <= 180.0f)  direction = direction::NW;
	else if (-45.0f  < angle && angle <= 0.0f)	  direction = direction::SE;
	else if (-90.0f  < angle && angle <= -45.0f)  direction = direction::S;
	else if (-135.0f < angle && angle <= -90.0f)  direction = direction::SW;
	else if (-180.0f < angle && angle <= -135.0f) direction = direction::W;

	result.bullet_offset = rotation_sprites->right_bullet_offset;
	result.rotated_sprite = rotation_sprites->right;
	result.flip_horizontally = false;

	switch (direction)
	{
		case direction::E:
		{
			result.bullet_offset = rotation_sprites->right_bullet_offset;
			result.rotated_sprite = rotation_sprites->right;
		}
		break;
		case direction::NE:
		{
			result.bullet_offset = rotation_sprites->right_down_bullet_offset;
			result.rotated_sprite = rotation_sprites->right_down;
		}
		break;
		case direction::N:
		{
			result.bullet_offset = rotation_sprites->down_bullet_offset;
			result.rotated_sprite = rotation_sprites->down;
		}
		break;
		case direction::NW:
		{
			result.bullet_offset = reflection_over_y_axis(
				rotation_sprites->right_down_bullet_offset);
			result.rotated_sprite = rotation_sprites->right_down;
			result.flip_horizontally = true;
		}
		break;
		case direction::W:
		{
			result.bullet_offset = reflection_over_y_axis(
				rotation_sprites->right_bullet_offset);
			result.rotated_sprite = rotation_sprites->right;
			result.flip_horizontally = true;
		}
		break;
		case direction::SW:
		{
			result.bullet_offset = reflection_over_y_axis(
				rotation_sprites->right_up_bullet_offset);
			result.rotated_sprite = rotation_sprites->right_up;
			result.flip_horizontally = true;
		}
		break;
		case direction::S:
		{
			result.bullet_offset = rotation_sprites->up_bullet_offset;
			result.rotated_sprite = rotation_sprites->up;
		}
		break;
		case direction::SE:
		{
			result.bullet_offset = rotation_sprites->right_up_bullet_offset;
			result.rotated_sprite = rotation_sprites->right_up;
		}
		break;
		invalid_default_case;
	}

	return result;
}

void set_entity_rotated_graphics(entity* entity, world_position* target)
{
	if (entity->type->rotation_sprites)
	{
		if (target)
		{
			v2 shooting_direction = get_unit_vector(get_position_difference(*target, entity->position));
			shooting_rotation rotation = get_entity_shooting_rotation(entity->type->rotation_sprites, shooting_direction);
			entity->shooting_sprite = rotation.rotated_sprite;
			entity->shooting_sprite.flip_horizontally = rotation.flip_horizontally;
		}
		else
		{
			entity->shooting_sprite = entity->type->rotation_sprites->right;
			if (entity->direction == direction::W)
			{
				entity->shooting_sprite.flip_horizontally = true;
			}
		}
	}
}

void move_entity_on_path(level_state* level, entity* entity_to_move, tile_position current_start, tile_position current_goal, 
	entity* player, r32 delta_time)
{
	if (current_goal != current_start)
	{
		v2 distance = get_position_difference(current_goal, entity_to_move->position);
		r32 distance_length = length(distance);
		v2 distance_to_start = get_position_difference(current_start, entity_to_move->position);
		r32 distance_to_start_length = length(distance_to_start);

		v2 direction = get_zero_v2();
		if (distance_length != 0)
		{
			direction = get_unit_vector(distance);
		}

		assert(entity_to_move->type->velocity_multiplier != 0.0f);
		r32 velocity = entity_to_move->type->velocity_multiplier;

		r32 slowdown_threshold = 2.0f;
		r32 margin = 0.1f;
		if (distance_length < slowdown_threshold)
		{
			velocity *= ((distance_length + margin) / slowdown_threshold);
		}
		else if (distance_to_start_length < slowdown_threshold)
		{
			velocity *= ((distance_to_start_length + margin) / slowdown_threshold);
		}

		entity_to_move->velocity = direction * velocity;
		if (entity_to_move->velocity.x < 0.0f)
		{
			entity_to_move->direction = direction::W;
		}
		else
		{
			entity_to_move->direction = direction::E;
		}

		world_position new_position = add_to_position(entity_to_move->position, (direction * velocity * delta_time));
		v2 movement_delta = get_position_difference(new_position, entity_to_move->position);
		entity_to_move->position = new_position;

		// sprawdzenie kolizji z graczem
		chunk_position reference_chunk = get_tile_chunk_position(get_tile_position(entity_to_move->position));
		collision new_collision = check_minkowski_collision(
			get_entity_collision_data(reference_chunk, entity_to_move),
			get_entity_collision_data(reference_chunk, player),
			movement_delta, 1.0f);

		if (new_collision.collided_wall != direction::NONE)
		{
			handle_player_and_enemy_collision(level, player, entity_to_move);
		}

		if (length(get_position_difference(current_goal, entity_to_move->position)) < 0.01f)
		{
			if (entity_to_move->goal_path_point == 0)
			{
				entity_to_move->goal_path_point = 1;
			}
			else if (entity_to_move->goal_path_point == 1)
			{
				entity_to_move->goal_path_point = 0;
			}
		}
	}
	else
	{
		entity_to_move->velocity = get_zero_v2();
	}
}

void move_entity_towards_player(level_state* level, entity* entity_to_move, entity* player, r32 delta_time)
{
	assert(entity_to_move->type->velocity_multiplier != 0.0f);
	r32 velocity = entity_to_move->type->velocity_multiplier;

	v2 direction = get_unit_vector(get_position_difference(player->position, entity_to_move->position));

	entity_to_move->velocity = direction * velocity;

	world_position new_position = add_to_position(entity_to_move->position, (direction * velocity * delta_time));
	v2 movement_delta = get_position_difference(new_position, entity_to_move->position);
	entity_to_move->position = new_position;

	// sprawdzenie kolizji z graczem
	chunk_position reference_chunk = get_tile_chunk_position(get_tile_position(entity_to_move->position));
	collision new_collision = check_minkowski_collision(
		get_entity_collision_data(reference_chunk, entity_to_move),
		get_entity_collision_data(reference_chunk, player),
		movement_delta, 1.0f);

	if (new_collision.collided_wall != direction::NONE)
	{
		handle_player_and_enemy_collision(level, player, entity_to_move);
	}
}

void handle_entity_and_bullet_collision(level_state* level, bullet* moving_bullet, entity* hit_entity)
{
	if (are_entity_flags_set(hit_entity, entity_flags::PLAYER))
	{
		damage_player(level, moving_bullet->type->damage_on_contact);
	}
	else
	{
		if (false == are_entity_flags_set(hit_entity, entity_flags::INDESTRUCTIBLE))
		{
			start_visual_effect(level, hit_entity, 1, false);
			hit_entity->health -= moving_bullet->type->damage_on_contact;
			printf("pocisk trafil w entity, %.2f obrazen, zostalo %.2f\n",
				moving_bullet->type->damage_on_contact, hit_entity->health);

			v2 bullet_direction = get_position_difference(moving_bullet->position, hit_entity->position);
			
			if (false == hit_entity->player_detected)
			{
				direction previous_direction = hit_entity->direction;
				if (bullet_direction.x < 0)
				{
					hit_entity->direction = direction::W;
				}
				else
				{
					hit_entity->direction = direction::E;
				}

				entity* player = get_player(level);
				if (is_point_visible_for_entity(level, hit_entity, player->position))
				{
					hit_entity->player_detected = true;					
				}
				else
				{
					hit_entity->direction = previous_direction;
				}
			}	
		}
	}
}

void enemy_fire_bullet(level_state* level, entity* enemy, entity* target, v2 target_offset)
{
	world_position target_position = add_to_position(target->position, target_offset);
	v2 target_relative_pos = get_position_difference(target_position, enemy->position);
	v2 direction_to_target = get_unit_vector(target_relative_pos);

	v2 bullet_offset = get_zero_v2();
	if (enemy->type->rotation_sprites)
	{
		shooting_rotation rotation = get_entity_shooting_rotation(enemy->type->rotation_sprites, direction_to_target);
		bullet_offset = rotation.bullet_offset;				
		direction_to_target = get_unit_vector(get_position_difference(
			target_position, add_to_position(enemy->position, rotation.bullet_offset)));
	}

	fire_bullet(level, enemy->type->fired_bullet_type, enemy->position, bullet_offset,
		direction_to_target * enemy->type->fired_bullet_type->constant_velocity);				
}

void process_entity_movement(level_state* level, entity* entity_to_move, entity* player, r32 delta_time)
{
	v2 distance_to_player = get_position_difference(player->position, entity_to_move->position);
	r32 distance_to_player_length = length(distance_to_player);

	if (false == entity_to_move->has_walking_path)
	{
		if (are_entity_flags_set(entity_to_move, entity_flags::WALKS_HORIZONTALLY))
		{
			find_walking_path_for_enemy(level, entity_to_move);
		}

		if (are_entity_flags_set(entity_to_move, entity_flags::FLIES_HORIZONTALLY))
		{
			find_flying_path_for_enemy(level, entity_to_move, false);
		}

		if (are_entity_flags_set(entity_to_move, entity_flags::FLIES_VERTICALLY))
		{
			find_flying_path_for_enemy(level, entity_to_move, true);
		}
	}

	if (entity_to_move->has_walking_path)
	{
		tile_position current_goal;
		tile_position current_start;

		if (entity_to_move->goal_path_point == 0)
		{
			current_goal = entity_to_move->path.start;
			current_start = entity_to_move->path.end;
		}
		else if (entity_to_move->goal_path_point == 1)
		{
			current_goal = entity_to_move->path.end;
			current_start = entity_to_move->path.start;
		}
		else
		{
			// wracamy na początek
			entity_to_move->goal_path_point = 0;
			current_goal = entity_to_move->path.start;
			current_start = entity_to_move->path.end;
		}

		v2 player_target_offset = get_v2(0.0f, -0.3f);
		world_position player_as_target = add_to_position(player->position, get_v2(0.0f, -0.3f));
		if (is_point_visible_for_entity(level, entity_to_move, player_as_target))
		{
			entity_to_move->player_detected = true;

			if (are_entity_flags_set(entity_to_move, entity_flags::ENEMY)
				&& entity_to_move->type->fired_bullet_type)
			{
				if (entity_to_move->attack_cooldown > 0.0f)
				{
					// nie robimy nic
					entity_to_move->attack_cooldown -= delta_time;

					if (entity_to_move->attack_cooldown <= 0.0f)
					{
						entity_to_move->attack_series_duration = entity_to_move->type->default_attack_series_duration;
					}
				}
				else
				{
					if (entity_to_move->attack_series_duration > 0.0f)
					{
						entity_to_move->attack_series_duration -= delta_time;

						if (entity_to_move->attack_bullet_interval_duration > 0.0f)
						{
							// przerwa, nic nie robimy
							entity_to_move->attack_bullet_interval_duration -= delta_time;
						}
						else
						{
							// wystrzeliwujemy jeden pocisk
							enemy_fire_bullet(level, entity_to_move, player, get_v2(0.0f, -0.3f));
							entity_to_move->attack_bullet_interval_duration =
								entity_to_move->type->default_attack_bullet_interval_duration;
						}
					}
					else
					{
						// przywracamy cooldown
						entity_to_move->attack_cooldown = entity_to_move->type->default_attack_cooldown;
					}
				}
			}
		}
		else
		{
			entity_to_move->player_detected = false;
			entity_to_move->attack_cooldown = 0.0f;
			entity_to_move->attack_series_duration = 0.0f;
			entity_to_move->attack_bullet_interval_duration = 0.0f;
		}

		// zatrzymywanie się lub podchodzenie w zależności od pozycji gracza				
		if (entity_to_move->player_detected)
		{
			set_entity_rotated_graphics(entity_to_move, &player->position);

			if (distance_to_player_length > entity_to_move->type->forget_detection_distance)
			{
				entity_to_move->player_detected = false;
			}
			else
			{
				if (distance_to_player_length < entity_to_move->type->stop_movement_distance)
				{
					tile_position entity_position = get_tile_position(entity_to_move->position);
					current_goal = entity_position;
					current_start = entity_position;
				}
				else
				{
					tile_position closest_end = get_closest_end_from_tile_range(entity_to_move->path, player->position);
					current_goal = closest_end;
				}
			}
		}
		else
		{
			set_entity_rotated_graphics(entity_to_move, NULL);
		}

		move_entity_on_path(level, entity_to_move, current_start, current_goal, player, delta_time);

		if (length(entity_to_move->velocity) <= 0.5f)
		{
			entity_to_move->current_animation = NULL;
			entity_to_move->animation_duration = 0.0f;
		}
		else
		{
			if (entity_to_move->current_animation != entity_to_move->type->walk_animation)
			{
				entity_to_move->current_animation = entity_to_move->type->walk_animation;
				entity_to_move->animation_duration = 0.0f;
			}
		}	
	}
	else
	{
		if (are_entity_flags_set(entity_to_move, entity_flags::FLIES_TOWARDS_PLAYER))
		{
			v2 player_target_offset = get_v2(0.0f, -0.3f);
			world_position player_as_target = add_to_position(player->position, get_v2(0.0f, -0.3f));
			if (is_point_visible_for_entity(level, entity_to_move, player_as_target))
			{
				entity_to_move->player_detected = true;
			}
			else
			{
				entity_to_move->player_detected = false;
			}

			if (entity_to_move->player_detected)
			{
				if (distance_to_player_length > entity_to_move->type->forget_detection_distance)
				{
					entity_to_move->player_detected = false;
				}
				else
				{
					move_entity_towards_player(level, entity_to_move, player, delta_time);
				}
			}
		}
	}
}