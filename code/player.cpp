#include "main.h"
#include "animation.h"
#include "entities.h"
#include "player.h"

entity* get_player(level_state* level)
{
	entity* result = &level->entities[0];
	return result;
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
		case entity_type_enum::POWER_UP_SPREAD:
		{
			level->power_ups.spread.time_remaining += 20.0f;
		}
		break;
	}

	power_up->health = -10.0f; // usuwamy obiekt
}

void damage_player(level_state* level, r32 damage_amount, b32 ignore_after_damage_invincibility)
{
	entity* player = get_player(level);	
	if (false == is_power_up_active(level->power_ups.invincibility))
	{	
		if (ignore_after_damage_invincibility || level->player_invincibility_cooldown <= 0.0f)
		{
			player->health -= damage_amount;			
			if (player->health >= 0.0f)
			{
				level->player_invincibility_cooldown = level->static_data->default_player_invincibility_cooldown;
			}
		}
	}
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

void write_to_standing_history(standing_history* buffer, b32 was_standing)
{
	buffer->buffer[buffer->current_index] = was_standing;
	buffer->current_index++;
	if (buffer->current_index == buffer->buffer_size)
	{
		buffer->current_index = 0;
	}
}

b32 get_if_was_standing(standing_history* buffer, u32 how_many_frames_backwards)
{
	i32 input_index = buffer->current_index - 1 - how_many_frames_backwards;
	while (input_index < 0)
	{
		input_index = buffer->buffer_size + input_index;
	}
	b32 result = buffer->buffer[input_index];
	return result;
}

void change_player_movement_mode(player_movement* movement, movement_mode mode)
{
	movement->previous_mode = movement->current_mode;
	movement->previous_mode_frame_duration = movement->frame_duration;
	movement->current_mode = mode;
	movement->frame_duration = 0;

#if TRIFLE_DEBUG
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
#endif
}

void player_fire_bullet(level_state* level, game_input* input, entity* player)
{
	if (player->attack_cooldown <= 0)
	{
		v2 relative_mouse_pos = (get_v2(input->mouse_x, input->mouse_y) / SCALING_FACTOR) - SCREEN_CENTER_IN_PIXELS;
		if (false == is_zero(relative_mouse_pos))
		{
			v2 shooting_direction = get_unit_vector(relative_mouse_pos);

			shooting_rotation rotation = get_entity_shooting_rotation(player->type->rotation_sprites, shooting_direction);

			entity_type* bullet_type = level->static_data->player_normal_bullet_type;
			if (is_power_up_active(level->power_ups.damage))
			{
				bullet_type = level->static_data->player_power_up_bullet_type;
			}

			// bez tego mamy rozjazd - kierunek lotu pocisku jest obliczany na podstawie położenia myszy względem środka ekranu
			// ale pocisk nie jest wystrzeliwany ze środka, tylko z rotation.bullet_offset
			v2 adjusted_shooting_direction = get_unit_vector(relative_mouse_pos - (rotation.bullet_offset * TILE_SIDE_IN_PIXELS));

			if (is_power_up_active(level->power_ups.spread))
			{
				fire_bullet(level, bullet_type, player->position, rotation.bullet_offset,
					adjusted_shooting_direction * bullet_type->constant_velocity);
				fire_bullet(level, bullet_type, player->position, rotation.bullet_offset,
					rotate_vector(adjusted_shooting_direction, 15, false) * bullet_type->constant_velocity);
				fire_bullet(level, bullet_type, player->position, rotation.bullet_offset,
					rotate_vector(adjusted_shooting_direction, -15, false) * bullet_type->constant_velocity);
			}
			else
			{
				fire_bullet(level, bullet_type, player->position, rotation.bullet_offset,
					adjusted_shooting_direction * bullet_type->constant_velocity);
			}

			player->attack_cooldown = player->type->default_attack_cooldown;
		}
	}	
}

void set_player_rotated_graphics_based_on_mouse_position(game_input* input, entity* player)
{
	v2 relative_mouse_pos = (get_v2(input->mouse_x, input->mouse_y) / SCALING_FACTOR) - SCREEN_CENTER_IN_PIXELS;
	if (false == is_zero(relative_mouse_pos))
	{
		v2 shooting_direction = get_unit_vector(relative_mouse_pos);

		shooting_rotation rotation = get_entity_shooting_rotation(player->type->rotation_sprites, shooting_direction);

		player->shooting_sprite = rotation.rotated_sprite;
		player->shooting_sprite.flip_horizontally = rotation.flip_horizontally;
	}
}

world_position process_input(level_state* level, input_buffer* input_buffer, entity* player, r32 delta_time)
{
	game_input empty_input = {};
	game_input* input = get_last_frame_input(input_buffer);

	if (level->stop_player_movement)
	{
		// wartości wybrane dla obróconej pozycji
		empty_input.mouse_x = SCREEN_CENTER_IN_PIXELS.x * 2 + 10;
		empty_input.mouse_y = SCREEN_CENTER_IN_PIXELS.y * 2 + 10;
		input = &empty_input;	
	}

	set_player_rotated_graphics_based_on_mouse_position(input, player);

	collision_result standing_on = {};
	b32 is_standing_at_frame_beginning = is_standing_on_ground(level, player, &standing_on);
	write_to_standing_history(&level->player_movement.standing_history, is_standing_at_frame_beginning);

	if (standing_on.collided_switch)
	{
		v4 color = standing_on.collided_switch->type->color;
		open_gates_with_given_color(level, color);
	}

	v2 gravity = get_v2(0, 1.0f);
	v2 jump_acceleration = get_v2(0, -33.0f);

	// zmiana statusu
	switch (level->player_movement.current_mode)
	{
		case movement_mode::WALK:
		{
			if (false == is_standing_at_frame_beginning)
			{
				change_player_movement_mode(&level->player_movement, movement_mode::JUMP);
			}
		}
		break;
		case movement_mode::JUMP:
		{
			if (is_standing_at_frame_beginning)
			{
				change_player_movement_mode(&level->player_movement, movement_mode::WALK);
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
					change_player_movement_mode(&level->player_movement, movement_mode::WALK);
					stop_visual_effect(level, player, sprite_effects_types::RECOIL);
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
			if (input->is_left_mouse_key_held)
			{
				player_fire_bullet(level, input, player);
			}

			if (input->up.number_of_presses > 0)
			{
				if (get_if_was_standing(&level->player_movement.standing_history, 0)
					&& get_if_was_standing(&level->player_movement.standing_history, 1))
				{
					player->acceleration = jump_acceleration;
					change_player_movement_mode(&level->player_movement, movement_mode::JUMP);
					break;
				}
			}

			if (input->left.number_of_presses > 0)
			{
				player->acceleration += get_v2(-1.25f, 0);
			}

			if (input->right.number_of_presses > 0)
			{
				player->acceleration += get_v2(1.25f, 0);
			}
		}
		break;
		case movement_mode::JUMP:
		{
			player->acceleration = gravity;

			if (input->is_left_mouse_key_held)
			{
				player_fire_bullet(level, input, player);
			}

			if (input->left.number_of_presses > 0)
			{
				player->acceleration += get_v2(-1, 0);
			}

			if (input->right.number_of_presses > 0)
			{
				player->acceleration += get_v2(1, 0);
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
		player->acceleration.x *= 1.7f;
		player->acceleration.y *= 1.3f;
	}

	player->velocity = player->type->slowdown_multiplier *
		(player->velocity + (player->acceleration * delta_time));

	world_position target_pos = add_to_position(player->position,
		player->velocity * player->type->velocity_multiplier * delta_time);

	if (standing_on.collided_platform)
	{
		if (level->player_movement.current_mode != movement_mode::JUMP)
		{
			// jeśli platforma porusza się w dół, "przyklejamy" gracza do platformy
			if (standing_on.collided_platform->velocity.y > 0.1f)
			{
				r32 closest_y_distance_from_platform =
					(standing_on.collided_platform->type->collision_rect_dim.y / 2)
					+ (player->type->collision_rect_dim.y / 2)
					+ player->type->collision_rect_offset.y;

				r32 actual_y_distance_from_platform =
					-get_position_difference(target_pos, standing_on.collided_platform->position).y;

				r32 difference = actual_y_distance_from_platform - closest_y_distance_from_platform;
				target_pos = add_to_position(target_pos, get_v2(0.0f, difference - 0.01f));
			}

			// nie jest to zgodne z fizyką, ale bardziej intuicyjne dla gracza
			// platforma przemieszcza gracza tylko wtedy, kiedy nie ma własnej prędkości
			if (length(player->velocity) < 0.05f)
			{
				target_pos = add_to_position(target_pos, standing_on.collided_platform->velocity * delta_time);
			}
		}
	}

	return target_pos;
}

void handle_player_and_enemy_collision(level_state* level, entity* player, entity* enemy)
{
	if (is_power_up_active(level->power_ups.invincibility))
	{
		enemy->health -= 50.0f;
	}
	else
	{
		if (are_entity_flags_set(enemy, entity_flags::DESTRUCTION_ON_PLAYER_CONTACT))
		{
			damage_player(level, enemy->type->damage_on_contact, true);
			enemy->health = -1.0f;
		}
		else if (are_entity_flags_set(enemy, entity_flags::PLAYER_RECOIL_ON_CONTACT))
		{
			start_visual_effect(level, player, sprite_effects_types::RECOIL);
			
			damage_player(level, enemy->type->damage_on_contact, false);

			v2 direction = get_unit_vector(
				get_position_difference(player->position, enemy->position));

			r32 acceleration = enemy->type->player_acceleration_on_collision;

			level->player_movement.recoil_timer = 2.0f;
			level->player_movement.recoil_acceleration_timer = 1.0f;
			level->player_movement.recoil_acceleration = (direction * acceleration);

			/*printf("odrzut! nowe przyspieszenie: (%.02f,%.02f)\n",
				level->player_movement.recoil_acceleration.x,
				level->player_movement.recoil_acceleration.y);*/

			change_player_movement_mode(&level->player_movement, movement_mode::RECOIL);
		}		
	}
}