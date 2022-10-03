#include "jorutils.h"
#include "jormath.h"
#include "main.h"
#include "map.h"
#include "rendering.h"

b32 are_flags_set(sprite_effect_flags* flags, sprite_effect_flags flag_values_to_check)
{
	b32 result = are_flags_set((u32*)flags, (u32)flag_values_to_check);
	return result;
}

void set_flags(sprite_effect_flags* flags, sprite_effect_flags flag_values_to_check)
{
	set_flags((u32*)flags, (u32)flag_values_to_check);
}

void unset_flags(sprite_effect_flags* flags, sprite_effect_flags flag_values_to_check)
{
	unset_flags((u32*)flags, (u32)flag_values_to_check);
}

r32 get_stage_tint(sprite_effect_stage* stage, r32 total_time)
{
	r32 result;
	if (stage->period == 0)
	{
		result = stage->amplitude; // stała wartość
	}
	else
	{
		// używamy pi zamiast 2pi ze względu na to, że niżej mamy odbicie ujemnych wartości
		result = stage->amplitude * sinf((total_time * pi32 / stage->period) + stage->phase_shift);
	}

	if (result < 0)
	{
		result = -result;
	}

	result += stage->vertical_shift;

	if (result < 0.0f)
	{
		result = 0.0f;
	}

	if (result > 1.0f)
	{
		result = 1.0f;
	}

	return result;
}

v4 get_tint(sprite_effect* effect, r32 time)
{
	v4 result = effect->color / 255;
	if (effect->total_duration == 0 || (time >= 0.0f && time <= effect->total_duration))
	{
		b32 found = false;
		r32 tint_value = 1.0f;
		for (u32 stage_index = 0; stage_index < effect->stages_count; stage_index++)
		{
			sprite_effect_stage* stage = effect->stages + stage_index;
			if (stage->stage_duration == 0 || (time < stage->stage_duration))
			{
				tint_value = get_stage_tint(stage, time);
				found = true;
				//printf("time: %.02f tint: %.02f\n", time, tint_value);
				break;
			}
			else
			{
				time -= stage->stage_duration;
			}
		}

		if (found)
		{
			result = (effect->color / 255) * tint_value;

			if (are_flags_set(&effect->flags, sprite_effect_flags::REVERSE_VALUES))
			{
				if (result.r != 0) { result.r = (1.0f - result.r); }
				if (result.g != 0) { result.g = (1.0f - result.g); }
				if (result.b != 0) { result.b = (1.0f - result.b); }
			}
		}
	}

	return result;
}

void start_visual_effect(entity* entity, sprite_effect* effect, b32 override_current)
{
	if (entity->visual_effect == NULL || override_current)
	{
		if (entity->visual_effect != effect)
		{
			entity->visual_effect = effect;
			entity->visual_effect_duration = 0;
		}
	}
}

void stop_visual_effect(entity* entity, sprite_effect* effect_to_stop)
{
	if (entity->visual_effect == effect_to_stop)
	{
		entity->visual_effect = NULL;
		entity->visual_effect_duration = 0;
	}	
}

void start_visual_effect(level_state* level, entity* entity, u32 sprite_effect_index, b32 override_current)
{
	assert(sprite_effect_index < level->static_data->visual_effects_count);
	sprite_effect* effect = &level->static_data->visual_effects[sprite_effect_index];
	start_visual_effect(entity, effect, override_current);
}

u32 get_current_animation_frame_index(animation* animation, r32 elapsed_time, r32 frame_duration_modifier)
{
	b32 result = 0;
	r32 modified_total_duration = (animation->total_duration * frame_duration_modifier);
	if (modified_total_duration > 0.0f)
	{
		while (elapsed_time > modified_total_duration)
		{
			elapsed_time -= modified_total_duration;
		}
	}
	
	r32 time_within_frame = elapsed_time;
	for (u32 frame_index = 0; frame_index < animation->frames_count; frame_index++)
	{
		animation_frame* frame = animation->frames + frame_index;
		r32 modified_frame_duration = frame->duration * frame_duration_modifier;
		if (time_within_frame > modified_frame_duration)
		{
			time_within_frame -= modified_frame_duration;
			continue;
		}
		else
		{
			result = frame_index;
			break;
		}
	}

	return result;
}

animation_frame* get_current_animation_frame(entity* entity)
{
	animation_frame* result = NULL;
	if (entity->current_animation
		&& entity->current_frame_index < entity->current_animation->frames_count)
	{
		result = &entity->current_animation->frames[entity->current_frame_index];
	}
	return result;
}

void animate_entity(player_movement* movement, entity* entity, r32 delta_time, r32 frame_duration_modifier)
{
	if (entity->visual_effect)
	{
		if (entity->visual_effect->total_duration == 0.0f)
		{
			// efekt bez końca
			entity->visual_effect_duration += delta_time;
			if (entity->visual_effect_duration > R32_MAX_VALUE - 10.0f)
			{
				// zapętlenie
				entity->visual_effect_duration = 0;
			}
		}
		else
		{
			if (entity->visual_effect_duration < entity->visual_effect->total_duration)
			{
				entity->visual_effect_duration += delta_time;
			}
			else	
			{	
				// zapętlenie lub skończenie efektu 
				if (are_flags_set((u32*)&entity->visual_effect->flags, (u32)sprite_effect_flags::REPEATS))
				{
					entity->visual_effect_duration = 0;
				}
				else
				{
					entity->visual_effect = NULL;
				}
			}
		}
	}

	if (entity->current_animation)
	{
		entity->animation_duration += delta_time;
		r32 modified_total_duration = entity->current_animation->total_duration * frame_duration_modifier;
		if (modified_total_duration > 0.0f)
		{
			while (entity->animation_duration > modified_total_duration)
			{
				entity->animation_duration -= modified_total_duration;
			}
		}
	
		entity->current_frame_index =
			get_current_animation_frame_index(entity->current_animation, entity->animation_duration, frame_duration_modifier);
	}

	if (movement)
	{
		switch (movement->current_mode)
		{
			case movement_mode::WALK:
			{
				if (length(entity->velocity) > 0.05f)
				{
					if (entity->current_animation != entity->type->walk_animation)
					{
						entity->current_animation = entity->type->walk_animation;
						entity->animation_duration = 0.0f;
					}
				}
				else
				{
					entity->current_animation = NULL;
					entity->animation_duration = 0.0f;
				}
			}
			break;
			case movement_mode::JUMP:
			{
				entity->current_animation = NULL;
				entity->animation_duration = 0.0f;
			}
			break;
			case movement_mode::RECOIL:
			{
				entity->current_animation = NULL;
				entity->animation_duration = 0.0f;
			}
			break;
		}
	}
}

void render_entity_animation_frame(render_group* render, world_position camera_position, entity* entity)
{
	sprite* sprite_to_render = NULL;
	if (entity->current_animation && entity->current_animation->frames)
	{
		sprite_to_render = &entity->current_animation->frames[entity->current_frame_index].sprite;
	}

	if (sprite_to_render == NULL || sprite_to_render->parts == NULL)
	{
		sprite_to_render = &entity->type->idle_pose.sprite;
	}

	assert(sprite_to_render != NULL);

	render_entity_sprite(render, camera_position, entity->position, entity->direction,
		entity->visual_effect, entity->visual_effect_duration, *sprite_to_render);
}

// strength 20 jest ok, 30 jest mocne, 40 - chyba zbyt
void start_screen_shake(level_state* level, r32 duration_in_seconds, r32 strength)
{
	level->screen_shake_duration = duration_in_seconds;
	level->screen_shake_multiplier = strength;
}

void process_fade(render_group* render, r32* percentage, r32 delta_time, b32 fade_in, r32 delta_time_multiplier)
{
	v4 fade_color = get_zero_v4();
	if (fade_in)
	{
		if (*percentage > 0.0f)
		{
			*percentage -= (delta_time * delta_time_multiplier);
			if (*percentage < 0.0f)
			{
				*percentage = 0.0f;
			}

			render_fade(render, fade_color, *percentage);
		}
	}
	else
	{
		if (*percentage < 1.0f)
		{
			*percentage += (delta_time * delta_time_multiplier);
			if (*percentage > 1.0f)
			{
				*percentage = 1.0f;
			}

			render_fade(render, fade_color, *percentage);
		}
	}
}