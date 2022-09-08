﻿#include "main.h"
#include "tmx_parsing.h"
#include "jorutils.h"

read_file_result read_file(std::string path)
{
	read_file_result result = {};

	SDL_RWops* file = SDL_RWFromFile(path.c_str(), "r");
	if (file)
	{
		int64_t file_size = SDL_RWsize(file);
		if (file_size != -1 // błąd
			&& file_size < 1024 * 1024 * 5) // zabezpieczenie
		{
			result.size = file_size;
			result.contents = new char[file_size + 1]; // dodatkowy bajt na końcu przyda się przy parsowaniu
			for (int byte_index = 0;
				byte_index < file_size;
				++byte_index)
			{
				SDL_RWread(file, (void*)((char*)result.contents + byte_index), sizeof(char), 1);
			}
			*((char*)result.contents + file_size) = 0;
		}
		else
		{
			print_sdl_error();
		}

		SDL_RWclose(file);
	}
	else
	{
		print_sdl_error();
	}

	return result;
}

sprite get_tile_graphics(sdl_game_data* sdl_game, memory_arena* arena, u32 tile_value)
{
	sprite result = {};

	SDL_Rect texture_rect = get_tile_rect(tile_value);

	result.parts_count = 1;
	result.parts = push_array(arena, result.parts_count, sprite_part);
	result.parts[0].texture = sdl_game->tileset_texture;
	result.parts[0].texture_rect = texture_rect;
	result.parts[0].offset = get_zero_v2();

	return result;
}

v2 get_standing_collision_rect_offset(v2 collision_rect_dim)
{
	// zakładamy że wszystkie obiekty mają pozycję na środku pola, czyli 0.5f nad górną krawędzią pola pod nimi
	v2 offset = get_zero_v2();
	offset.y = -((collision_rect_dim.y / 2) - 0.5f);
	return offset;
}

sprite_effect_stage* add_sprite_effect_stage(sprite_effect* effect,
	r32 amplitude, r32 phase_shift, r32 vertical_shift, r32 period, r32 stage_duration)
{
	assert(stage_duration != 0);

	sprite_effect_stage* new_stage = &effect->stages[effect->stages_count++];
	effect->total_duration += stage_duration;

	new_stage->amplitude = amplitude;
	new_stage->phase_shift = phase_shift;
	new_stage->period = period;
	new_stage->stage_duration = stage_duration;

	return new_stage;
}

sprite_effect_stage* add_constant_tint_sprite_effect_stage(sprite_effect* effect,
	r32 tint_perc, r32 stage_duration)
{
	sprite_effect_stage* new_stage = add_sprite_effect_stage(effect, tint_perc, 0.0f, 0.0f, 0.0f, stage_duration);
	return new_stage;
}

void fill_animation_frame(animation* animation, u32 frame_index, u32 part_index, sprite_part part, r32 duration)
{
	animation->frames[frame_index].sprite.parts[part_index] = part;
	animation->frames[frame_index].duration = duration;
	animation->total_duration += duration;
}

sprite_part get_sprite_part(SDL_Texture* texture, SDL_Rect texture_rect, v2 offset = get_zero_v2(),
	direction default_direction = direction::E)
{
	sprite_part result = {};
	result.texture = texture;
	result.texture_rect = texture_rect;
	result.offset = offset;
	result.default_direction = default_direction;
	return result;
}

animation* get_player_walk_animation(sdl_game_data* sdl_game, memory_arena* arena)
{
	animation* new_animation = push_struct(arena, animation);
	new_animation->frames_count = 4;
	new_animation->frames = push_array(arena, new_animation->frames_count, animation_frame);

	SDL_Rect legs_rect = {};
	legs_rect.x = 0;
	legs_rect.y = 24;
	legs_rect.w = 24;
	legs_rect.h = 24;

	SDL_Rect head_rect = {};
	head_rect.x = 0;
	head_rect.y = 0;
	head_rect.w = 24;
	head_rect.h = 24;

	v2 legs_offset = get_v2(0.0f, -4.0f);
	v2 head_offset = get_v2(5.0f, -20.0f);

	r32 frame_duration = 0.2f;
	SDL_Texture* texture = sdl_game->player_texture;

	new_animation->frames[0].sprite.parts = push_array(arena, 2, sprite_part);
	new_animation->frames[1].sprite.parts = push_array(arena, 2, sprite_part);
	new_animation->frames[2].sprite.parts = push_array(arena, 2, sprite_part);
	new_animation->frames[3].sprite.parts = push_array(arena, 2, sprite_part);

	new_animation->frames[0].sprite.parts_count = 2;
	new_animation->frames[1].sprite.parts_count = 2;
	new_animation->frames[2].sprite.parts_count = 2;
	new_animation->frames[3].sprite.parts_count = 2;

	legs_rect.x = 24;
	fill_animation_frame(new_animation, 0, 0, get_sprite_part(texture, legs_rect, legs_offset), frame_duration);
	fill_animation_frame(new_animation, 0, 1, get_sprite_part(texture, head_rect, head_offset), frame_duration);
	head_offset.x += 1;
	fill_animation_frame(new_animation, 1, 0, get_sprite_part(texture, legs_rect, legs_offset), frame_duration);
	fill_animation_frame(new_animation, 1, 1, get_sprite_part(texture, head_rect, head_offset), frame_duration);
	legs_rect.x = 48;
	fill_animation_frame(new_animation, 2, 0, get_sprite_part(texture, legs_rect, legs_offset), frame_duration);
	fill_animation_frame(new_animation, 2, 1, get_sprite_part(texture, head_rect, head_offset), frame_duration);
	head_offset.x -= 1;
	//legs_rect.x = 0;
	fill_animation_frame(new_animation, 3, 0, get_sprite_part(texture, legs_rect, legs_offset), frame_duration);
	fill_animation_frame(new_animation, 3, 1, get_sprite_part(texture, head_rect, head_offset), frame_duration);
	//head_offset.x -= 1;

	return new_animation;
}

sprite get_player_idle_pose(sdl_game_data* sdl_game, memory_arena* arena)
{
	sprite result = {};

	SDL_Rect player_head_rect = {};
	player_head_rect.x = 0;
	player_head_rect.y = 0;
	player_head_rect.w = 24;
	player_head_rect.h = 24;

	SDL_Rect player_legs_rect = {};
	player_legs_rect.x = 0;
	player_legs_rect.y = 24;
	player_legs_rect.w = 24;
	player_legs_rect.h = 24;

	v2 legs_offset = get_v2(0.0f, -4.0f);
	v2 head_offset = get_v2(5.0f, -20.0f);

	result.parts_count = 2;
	result.parts = push_array(arena, result.parts_count, sprite_part);
	result.parts[0].texture = sdl_game->player_texture;
	result.parts[0].texture_rect = player_head_rect;
	result.parts[0].offset = head_offset;
	result.parts[0].default_direction = direction::E;

	result.parts[1].texture = sdl_game->player_texture;
	result.parts[1].texture_rect = player_legs_rect;
	result.parts[1].offset = legs_offset;
	result.parts[1].default_direction = direction::E;

	return result;
}

sprite get_bullet_graphics(sdl_game_data* sdl_game, memory_arena* arena, u32 x, u32 y)
{
	sprite result = {};

	SDL_Rect texture_rect = {};
	texture_rect.w = 10;
	texture_rect.h = 10;
	texture_rect.x = y * 10;
	texture_rect.y = x * 10;

	result.parts_count = 1;
	result.parts = push_array(arena, result.parts_count, sprite_part);
	result.parts[0].texture = sdl_game->bullets_texture;
	result.parts[0].texture_rect = texture_rect;
	result.parts[0].offset = get_zero_v2();

	return result;
}

void load_game_data(sdl_game_data* sdl_game, game_data* game, memory_arena* arena)
{
	std::string collision_file_path = "data/collision_map.tmx";
	read_file_result collision_file = read_file(collision_file_path);
	game->collision_reference = read_level_from_tmx_file(arena, collision_file, "collision");

	std::string map_file_path = "data/map_01.tmx";
	read_file_result map_file = read_file(map_file_path);
	game->current_level = read_level_from_tmx_file(arena, map_file, "map");

	delete map_file.contents;

	game->entity_types_count = 5;
	game->entity_types = push_array(arena, game->entity_types_count, entity_type);
	game->entities_count = 0;
	game->entities_max_count = 1000;
	game->entities = push_array(arena, game->entities_max_count, entity);

	game->default_player_invincibility_cooldown = 2.0f;
	game->player_invincibility_cooldown = 0.0f;

	entity_type* player_entity_type = &game->entity_types[0];
	player_entity_type->idle_pose = get_player_idle_pose(sdl_game, arena);
	player_entity_type->flags = (entity_flags)(entity_flags::COLLIDES | entity_flags::PLAYER);
	player_entity_type->max_health = 100;
	player_entity_type->velocity_multiplier = 40.0f;
	player_entity_type->slowdown_multiplier = 0.80f;
	player_entity_type->default_attack_cooldown = 0.1f;
	player_entity_type->walk_animation = get_player_walk_animation(sdl_game, arena);
	player_entity_type->collision_rect_dim = get_v2(0.35f, 1.6f);
	player_entity_type->collision_rect_offset =
		get_standing_collision_rect_offset(player_entity_type->collision_rect_dim);

	game->player_movement.current_mode = movement_mode::WALK;

	entity_type* default_entity_type = &game->entity_types[1];
	default_entity_type->idle_pose = get_tile_graphics(sdl_game, arena, 837);
	default_entity_type->flags = (entity_flags)(entity_flags::COLLIDES | entity_flags::ENEMY);
	default_entity_type->max_health = 10;
	default_entity_type->damage_on_contact = 10;
	default_entity_type->default_attack_cooldown = 0.5f;
	default_entity_type->player_acceleration_on_collision = 5.0f;
	default_entity_type->collision_rect_dim = get_v2(1.0f, 1.0f);
	default_entity_type->collision_rect_offset =
		get_standing_collision_rect_offset(default_entity_type->collision_rect_dim);

	entity_type* moving_enemy_type = &game->entity_types[2];
	moving_enemy_type->idle_pose = get_tile_graphics(sdl_game, arena, 1992);
	moving_enemy_type->flags = (entity_flags)(entity_flags::COLLIDES | entity_flags::WALKS_HORIZONTALLY | entity_flags::ENEMY);
	moving_enemy_type->max_health = 10;
	moving_enemy_type->damage_on_contact = 10;
	moving_enemy_type->default_attack_cooldown = 0.2f;
	moving_enemy_type->velocity_multiplier = 5.0f;
	moving_enemy_type->player_acceleration_on_collision = 5.0f;
	moving_enemy_type->collision_rect_dim = get_v2(1.0f, 1.0f);
	moving_enemy_type->collision_rect_offset =
		get_standing_collision_rect_offset(moving_enemy_type->collision_rect_dim);

	game->bullet_types_count = 5;
	game->bullet_types = push_array(arena, game->bullet_types_count, entity_type);
	game->bullets_count = 0;
	game->bullets_max_count = 5000;
	game->bullets = push_array(arena, game->bullets_max_count, bullet);

	entity_type* player_bullet_type = &game->bullet_types[0];
	player_bullet_type->damage_on_contact = 5;
	player_bullet_type->constant_velocity = 12.0f;
	player_bullet_type->idle_pose = get_bullet_graphics(sdl_game, arena, 1, 1);

	entity_type* enemy_bullet_type = &game->bullet_types[1];
	enemy_bullet_type->damage_on_contact = 5;
	enemy_bullet_type->flags = entity_flags::DAMAGES_PLAYER;
	enemy_bullet_type->constant_velocity = 12.0f;
	enemy_bullet_type->idle_pose = get_bullet_graphics(sdl_game, arena, 1, 1);

	player_entity_type->fired_bullet_type = player_bullet_type;
	default_entity_type->fired_bullet_type = enemy_bullet_type;

	game->visual_effects_count = 5;
	game->visual_effects = push_array(arena, game->visual_effects_count, sprite_effect);

	sprite_effect* damage_tint_effect = &game->visual_effects[0];
	damage_tint_effect->stages_count = 1;
	damage_tint_effect->stages = push_array(arena, damage_tint_effect->stages_count, sprite_effect_stage);
	damage_tint_effect->color = { 255, 0, 0, 0 };

	add_sprite_effect_stage(damage_tint_effect, 1.0f, 0.0f, 0.0f, 5.0f, 5.0f);
	add_constant_tint_sprite_effect_stage(damage_tint_effect, 0.5f, 5.0f);

	// debug entities

	add_entity(game, 0, 0, player_entity_type);

	add_entity(game, 16, 6, default_entity_type);
	add_entity(game, 18, 6, default_entity_type);
	add_entity(game, 20, 6, default_entity_type);

	add_entity(game, 14, 6, moving_enemy_type);
}