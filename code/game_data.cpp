#include "main.h"
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

sprite_part get_16x16_sprite_part(SDL_Texture* texture, u32 tile_x, u32 tile_y)
{
	sprite_part result = {};
	result.texture = texture;
	result.texture_rect = {(i32)tile_x * 16, (i32)tile_y * 16, 16, 16};
	return result;
}

animation_frame get_tile_graphics(sdl_game_data* sdl_game, memory_arena* arena, u32 tile_value)
{
	animation_frame result = {};
	result.sprite.parts_count = 1;
	result.sprite.parts = push_array(arena, result.sprite.parts_count, sprite_part);
	result.sprite.parts[0].texture = sdl_game->tileset_texture;
	result.sprite.parts[0].texture_rect = get_tile_rect(tile_value);
	return result;
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
	result.offset_in_pixels = offset;
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

	v2 legs_offset = get_v2(0.0f, 0.0f);
	v2 head_offset = get_v2(5.0f, -16.0f);

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

animation_frame get_player_idle_pose(sdl_game_data* sdl_game, memory_arena* arena)
{
	animation_frame result = {};	

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

	v2 legs_offset = get_v2(0.0f, 0.0f);
	v2 head_offset = get_v2(5.0f, -16.0f);

	result.sprite.parts_count = 2;
	result.sprite.parts = push_array(arena, result.sprite.parts_count, sprite_part);
	result.sprite.parts[0].texture = sdl_game->player_texture;
	result.sprite.parts[0].texture_rect = player_head_rect;
	result.sprite.parts[0].offset_in_pixels = head_offset;
	result.sprite.parts[0].default_direction = direction::E;

	result.sprite.parts[1].texture = sdl_game->player_texture;
	result.sprite.parts[1].texture_rect = player_legs_rect;
	result.sprite.parts[1].offset_in_pixels = legs_offset;
	result.sprite.parts[1].default_direction = direction::E;

	return result;
}

animation_frame get_bullet_graphics(sdl_game_data* sdl_game, memory_arena* arena, u32 x, u32 y)
{
	animation_frame result = {};

	SDL_Rect texture_rect = {};
	texture_rect.w = 10;
	texture_rect.h = 10;
	texture_rect.x = y * 10;
	texture_rect.y = x * 10;

	result.sprite.parts_count = 1;
	result.sprite.parts = push_array(arena, result.sprite.parts_count, sprite_part);
	result.sprite.parts[0].texture = sdl_game->bullets_texture;
	result.sprite.parts[0].texture_rect = texture_rect;
	return result;
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

void test_if_all_types_loaded(entity_type_dictionary dictionary)
{
	for (i32 enum_value = (i32)entity_type_enum::UNKNOWN + 1;
		enum_value < (i32)entity_type_enum::_LAST;
		enum_value++)
	{
		assert(enum_value < dictionary.type_ptrs_count);
		entity_type* type = dictionary.type_ptrs[enum_value];
		assert(type != NULL);
	}
}

void load_game_data(sdl_game_data* sdl_game, game_data* game, memory_arena* arena, memory_arena* transient_arena)
{
	temporary_memory transient_memory = begin_temporary_memory(transient_arena);

	std::string collision_file_path = "data/collision_map.tmx";
	read_file_result collision_file = read_file(collision_file_path);
	game->collision_reference = read_level_from_tmx_file(arena, transient_arena, collision_file, "collision");

	std::string map_file_path = "data/map_01.tmx";
	read_file_result map_file = read_file(map_file_path);
	game->current_level = read_level_from_tmx_file(arena, transient_arena, map_file, "map");

	delete map_file.contents;
	delete collision_file.contents;

	game->gate_sprite = get_16x16_sprite_part(sdl_game->gates_texture, 8, 1);
	game->gate_display_lower_sprite = get_16x16_sprite_part(sdl_game->gates_texture, 7, 1);
	game->gate_display_upper_sprite = get_16x16_sprite_part(sdl_game->gates_texture, 7, 0);
	game->gate_frame_lower_sprite = get_16x16_sprite_part(sdl_game->gates_texture, 8, 0);
	game->gate_frame_upper_sprite = get_16x16_sprite_part(sdl_game->gates_texture, 8, 2);
	game->switch_frame_left_sprite = get_16x16_sprite_part(sdl_game->gates_texture, 0, 0);
	game->switch_frame_middle_sprite = get_16x16_sprite_part(sdl_game->gates_texture, 1, 0);
	game->switch_frame_right_sprite = get_16x16_sprite_part(sdl_game->gates_texture, 2, 0);	
	game->switch_display_left_sprite = get_16x16_sprite_part(sdl_game->gates_texture, 3, 0);
	game->switch_display_middle_sprite = get_16x16_sprite_part(sdl_game->gates_texture, 4, 0);
	game->switch_display_right_sprite = get_16x16_sprite_part(sdl_game->gates_texture, 5, 0);

	game->entity_types_count = 20;
	game->entity_types = push_array(arena, game->entity_types_count, entity_type);
	game->entities_count = 0;
	game->entities_max_count = 1000;
	game->entities = push_array(arena, game->entities_max_count, entity);

	game->default_player_invincibility_cooldown = 2.0f;
	game->player_invincibility_cooldown = 0.0f;

	entity_type* player_entity_type = &game->entity_types[0];
	player_entity_type->idle_pose = get_player_idle_pose(sdl_game, arena);
	player_entity_type->flags = (entity_flags)((u32)entity_flags::COLLIDES | (u32)entity_flags::PLAYER);
	player_entity_type->max_health = 100;
	player_entity_type->velocity_multiplier = 40.0f;
	player_entity_type->slowdown_multiplier = 0.80f;
	player_entity_type->default_attack_cooldown = 0.2f;
	player_entity_type->walk_animation = get_player_walk_animation(sdl_game, arena);
	player_entity_type->fired_bullet_offset = get_v2(0.85f, -0.60f); // nie w pikselach!
	player_entity_type->collision_rect_dim = get_v2(0.35f, 1.6f);

	game->player_movement.current_mode = movement_mode::WALK;

	entity_type* static_enemy_type = &game->entity_types[1];
	static_enemy_type->idle_pose = get_tile_graphics(sdl_game, arena, 837);
	static_enemy_type->flags = (entity_flags)((u32)entity_flags::COLLIDES | (u32)entity_flags::ENEMY);
	static_enemy_type->max_health = 10;
	static_enemy_type->damage_on_contact = 10;
	static_enemy_type->default_attack_cooldown = 0.5f;
	static_enemy_type->player_detecting_distance = 10.0f;
	static_enemy_type->player_acceleration_on_collision = 3.0f;
	static_enemy_type->collision_rect_dim = get_v2(1.0f, 1.0f);

	entity_type* moving_enemy_type = &game->entity_types[2];
	moving_enemy_type->idle_pose = get_tile_graphics(sdl_game, arena, 1992);
	moving_enemy_type->flags = (entity_flags)((u32)entity_flags::COLLIDES 
		| (u32)entity_flags::WALKS_HORIZONTALLY 
		| (u32)entity_flags::ENEMY);
	moving_enemy_type->max_health = 10;
	moving_enemy_type->damage_on_contact = 10;
	moving_enemy_type->default_attack_cooldown = 0.2f;
	moving_enemy_type->velocity_multiplier = 5.0f;
	moving_enemy_type->player_acceleration_on_collision = 3.0f;
	moving_enemy_type->collision_rect_dim = get_v2(1.0f, 1.0f);

	game->entity_types_dict = create_entity_types_dictionary(arena);
	set_entity_type_ptr(game->entity_types_dict, entity_type_enum::PLAYER, player_entity_type);
	set_entity_type_ptr(game->entity_types_dict, entity_type_enum::STATIC_ENEMY, static_enemy_type);
	set_entity_type_ptr(game->entity_types_dict, entity_type_enum::MOVING_ENEMY, moving_enemy_type);
	//test_if_all_types_loaded(game->entity_types_dict);

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
	static_enemy_type->fired_bullet_type = enemy_bullet_type;

	game->visual_effects_count = 5;
	game->visual_effects = push_array(arena, game->visual_effects_count, sprite_effect);

	sprite_effect* gate_display_fade_effect = &game->visual_effects[0];
	gate_display_fade_effect->stages_count = 1;
	gate_display_fade_effect->stages = push_array(arena, gate_display_fade_effect->stages_count, sprite_effect_stage);
	gate_display_fade_effect->color = get_v4(100, 100, 100, 0);

	sprite_effect* damage_tint_effect = &game->visual_effects[1];
	damage_tint_effect->stages_count = 1;
	damage_tint_effect->stages = push_array(arena, damage_tint_effect->stages_count, sprite_effect_stage);
	damage_tint_effect->color = get_v4(255, 0, 0, 0);

	add_sprite_effect_stage(damage_tint_effect, 1.0f, 0.0f, 0.0f, 5.0f, 5.0f);
	add_constant_tint_sprite_effect_stage(damage_tint_effect, 0.5f, 5.0f);

	end_temporary_memory(transient_memory, true);
}