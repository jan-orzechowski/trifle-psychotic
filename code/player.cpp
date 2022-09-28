#include "main.h"
#include "animation.h"
#include "entities.h"

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

b32 damage_player(level_state* level, r32 damage_amount)
{
	entity* player = get_player(level);
	b32 damaged = false;
	if (level->player_invincibility_cooldown <= 0.0f
		&& false == is_power_up_active(level->power_ups.invincibility))
	{	
		damaged = true;
		player->health -= damage_amount;
		start_visual_effect(level, player, 1, false);
		//printf("gracz dostaje %.02f obrazen, zostalo %.02f zdrowia\n", damage_amount, player->health);
		if (player->health < 0.0f)
		{
			// przegrywamy
			add_explosion(level, player->position, level->static_data->explosion_animations.size_48x48);
			debug_breakpoint;
		}
		else
		{
			level->player_invincibility_cooldown = level->static_data->default_player_invincibility_cooldown;
		}
	}
	return damaged;
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

void change_player_movement_mode(player_movement* movement, movement_mode mode)
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

void player_fire_bullet(level_state* level, game_input* input, world_position player_position, entity_type* bullet_type)
{
	v2 relative_mouse_pos = (get_v2(input->mouse_x, input->mouse_y) / SCALING_FACTOR) - SCREEN_CENTER_IN_PIXELS;

	r32 angle = atan2(relative_mouse_pos.y, relative_mouse_pos.x) * (180 / pi32);
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

	v2 bullet_offset = level->static_data->player_shooting_right_bullet_offset;
	sprite player_torso_graphics = level->static_data->player_shooting_right;
	b32 flip_graphics_horizontally = false;

	switch (direction)
	{
		case direction::E:
		{
			bullet_offset = level->static_data->player_shooting_right_bullet_offset;
			player_torso_graphics = level->static_data->player_shooting_right;
		}
		break;
		case direction::NE:
		{
			bullet_offset = level->static_data->player_shooting_right_down_bullet_offset;
			player_torso_graphics = level->static_data->player_shooting_right_down;
		}
		break;
		case direction::N:
		{
			bullet_offset = level->static_data->player_shooting_down_bullet_offset;
			player_torso_graphics = level->static_data->player_shooting_down;
		}
		break;
		case direction::NW:
		{
			bullet_offset = reflection_over_y_axis(
				level->static_data->player_shooting_right_down_bullet_offset);
			player_torso_graphics = level->static_data->player_shooting_right_down;
			flip_graphics_horizontally = true;
		}
		break;
		case direction::W:
		{
			bullet_offset = reflection_over_y_axis(
				level->static_data->player_shooting_right_bullet_offset);
			player_torso_graphics = level->static_data->player_shooting_right;
			flip_graphics_horizontally = true;
		}
		break;
		case direction::SW:
		{
			bullet_offset = reflection_over_y_axis(
				level->static_data->player_shooting_right_up_bullet_offset);
			player_torso_graphics = level->static_data->player_shooting_right_up;
			flip_graphics_horizontally = true;
		}
		break;
		case direction::S:
		{
			bullet_offset = level->static_data->player_shooting_up_bullet_offset;
			player_torso_graphics = level->static_data->player_shooting_up;
		}
		break;
		case direction::SE:
		{
			bullet_offset = level->static_data->player_shooting_right_up_bullet_offset;
			player_torso_graphics = level->static_data->player_shooting_right_up;
		}
		break;
		invalid_default_case;
	}

	v2 bullet_direction = get_unit_vector(relative_mouse_pos);

	if (is_power_up_active(level->power_ups.spread))
	{
		fire_bullet(level, bullet_type, player_position, bullet_offset,
			bullet_direction * bullet_type->constant_velocity);
		fire_bullet(level, bullet_type, player_position, bullet_offset,
			rotate_vector(bullet_direction, 15, false) * bullet_type->constant_velocity);
		fire_bullet(level, bullet_type, player_position, bullet_offset,
			rotate_vector(bullet_direction, -15, false) * bullet_type->constant_velocity);
	}
	else
	{
		fire_bullet(level, bullet_type, player_position, bullet_offset, bullet_direction * bullet_type->constant_velocity);
	}

	level->current_player_torso = player_torso_graphics;
	level->flip_player_torso_horizontally = flip_graphics_horizontally;
}

world_position process_input(level_state* level, input_buffer* input_buffer, entity* player, r32 delta_time)
{
	game_input* input = get_last_frame_input(input_buffer);

	b32 is_standing_at_frame_beginning = is_standing_on_ground(level, player);

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
					if (was_up_key_pressed_in_last_frames(input_buffer, 3))
					{
						player->acceleration += jump_acceleration;
						change_player_movement_mode(&level->player_movement, movement_mode::JUMP);
						printf("ulatwienie!\n");
						break;
					}
				}
			}

			if (input->is_left_mouse_key_held)
			{
				if (player->attack_cooldown <= 0)
				{
					player_fire_bullet(level, input, player->position, player->type->fired_bullet_type);
					player->attack_cooldown = player->type->default_attack_cooldown;
				}
			}

			if (input->up.number_of_presses > 0)
			{
				if (is_standing_at_frame_beginning)
				{
					player->acceleration += jump_acceleration;
					change_player_movement_mode(&level->player_movement, movement_mode::JUMP);
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
					player_fire_bullet(level, input, player->position, player->type->fired_bullet_type);
					player->attack_cooldown = player->type->default_attack_cooldown;
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
		player->acceleration.x *= 1.7f;
		player->acceleration.y *= 1.3f;
	}

	player->velocity = player->type->slowdown_multiplier *
		(player->velocity + (player->acceleration * delta_time));

	world_position target_pos = add_to_position(player->position,
		player->velocity * player->type->velocity_multiplier * delta_time);

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

		change_player_movement_mode(&level->player_movement, movement_mode::RECOIL);
	}
}

save* save_player_state(memory_arena* arena, level_state* level)
{
	assert(level->entities[0].type);
	save* result = push_struct(arena, save);
	result->map_name = copy_string(arena, level->current_map_name);
	result->player_max_health = level->entities[0].type->max_health;
	return result;
}

void restore_player_state(level_state* level, save* save)
{
	assert(level->current_map_initialized);
	assert(level && save);
	level->entities[0].type->max_health = save->player_max_health;
}