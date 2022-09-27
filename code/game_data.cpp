#include "main.h"
#include "tmx_parsing.h"
#include "jorutils.h"
#include "sdl_platform.h"
#include "entities.h"

#define ENTITY_TYPES_MAX_COUNT 20
#define BULLET_TYPES_MAX_COUNT 10

sprite_part get_square_sprite_part(v2 offset, u32 square_side, textures texture, u32 tile_x, u32 tile_y)
{
	sprite_part result = {};
	result.texture = texture;
	result.texture_rect = get_rect_from_min_corner(
		offset.x + (i32)tile_x * square_side, 
		offset.y + (i32)tile_y * square_side, 
		square_side, 
		square_side);
	return result;
}

animation_frame get_square_animation_frame(v2 offset, u32 square_side, memory_arena* arena, textures texture, u32 tile_x, u32 tile_y)
{
	animation_frame result = {};
	result.sprite.parts_count = 1;
	result.sprite.parts = push_array(arena, result.sprite.parts_count, sprite_part);
	result.sprite.parts[0].texture = texture;
	result.sprite.parts[0].texture_rect = get_rect_from_min_corner(
		offset.x + (i32)tile_x * square_side,
		offset.y + (i32)tile_y * square_side,
		square_side,
		square_side);
	return result;
}

sprite_part get_16x16_sprite_part(v2 offset, textures texture, u32 tile_x, u32 tile_y)
{
	sprite_part result = get_square_sprite_part(offset, 16, texture, tile_x, tile_y);
	return result;
}

animation_frame get_16x16_animation_frame(v2 offset, memory_arena* arena, textures texture, u32 tile_x, u32 tile_y)
{
	animation_frame result = get_square_animation_frame(offset, 16, arena, texture, tile_x, tile_y);
	return result;
}

sprite get_square_sprite(memory_arena* arena, u32 square_side, textures texture, u32 tile_x, u32 tile_y, v2 offset = get_zero_v2())
{
	sprite result = {};
	result.parts_count = 1;
	result.parts = push_array(arena, result.parts_count, sprite_part);
	result.parts[0] = get_square_sprite_part(get_zero_v2(), square_side, texture, tile_x, tile_y);
	result.parts[0].offset_in_pixels = offset;
	return result;
}

animation_frame get_tile_graphics(memory_arena* arena, u32 tile_value)
{
	animation_frame result = {};
	result.sprite.parts_count = 1;
	result.sprite.parts = push_array(arena, result.sprite.parts_count, sprite_part);
	result.sprite.parts[0].texture = textures::TILESET;
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

void fill_animation_frame(animation* animation, u32 frame_index, u32 part_index, sprite_part part, r32* duration = NULL)
{
	animation->frames[frame_index].sprite.parts[part_index] = part;
	if (duration)
	{
		animation->frames[frame_index].duration = *duration;
		animation->total_duration += *duration;
	}
}

sprite_part get_sprite_part(textures texture, rect texture_rect, v2 sprite_offset = get_zero_v2(),
	direction default_direction = direction::E)
{
	sprite_part result = {};
	result.texture = texture;
	result.texture_rect = texture_rect;
	result.offset_in_pixels = sprite_offset;
	result.default_direction = default_direction;
	return result;
}

animation* get_walk_animation(memory_arena* arena, v2 offset, b32 add_head)
{
	animation* new_animation = push_struct(arena, animation);
	new_animation->frames_count = 3;
	new_animation->frames = push_array(arena, new_animation->frames_count, animation_frame);

	r32 frame_duration = 0.2f;
	textures texture = textures::CHARSET;

	r32 parts_count = add_head ? 2 : 1;

	new_animation->frames[0].sprite.parts_count = parts_count;
	new_animation->frames[1].sprite.parts_count = parts_count;
	new_animation->frames[2].sprite.parts_count = parts_count;
	new_animation->frames[0].sprite.parts = push_array(arena, parts_count, sprite_part);
	new_animation->frames[1].sprite.parts = push_array(arena, parts_count, sprite_part);
	new_animation->frames[2].sprite.parts = push_array(arena, parts_count, sprite_part);

	rect legs_rect = get_rect_from_min_corner(offset.x + 24, offset.y + 24, 24, 24);
	fill_animation_frame(new_animation, 0, 0, get_sprite_part(texture, legs_rect), &frame_duration);
	legs_rect = move_rect(legs_rect, get_v2(24, 0));
	fill_animation_frame(new_animation, 1, 0, get_sprite_part(texture, legs_rect), &frame_duration);
	legs_rect = move_rect(legs_rect, get_v2(24, 0));
	fill_animation_frame(new_animation, 2, 0, get_sprite_part(texture, legs_rect), &frame_duration);

	if (add_head) 
	{
		new_animation->frames[0].sprite.parts[0].offset_in_pixels = get_v2(0.0f, -5.0f);
		new_animation->frames[1].sprite.parts[0].offset_in_pixels = get_v2(0.0f, -5.0f);
		new_animation->frames[2].sprite.parts[0].offset_in_pixels = get_v2(0.0f, -5.0f);

		rect head_rect = get_rect_from_min_corner(offset.x, offset.y, 24, 24);
		v2 head_offset = get_v2(5, -19);
		fill_animation_frame(new_animation, 0, 1, get_sprite_part(texture, head_rect, head_offset), NULL);	
		fill_animation_frame(new_animation, 1, 1, get_sprite_part(texture, head_rect, head_offset), NULL);
		fill_animation_frame(new_animation, 2, 1, get_sprite_part(texture, head_rect, head_offset), NULL);
	}

	return new_animation;
}

animation_frame get_walk_idle_pose(memory_arena* arena, v2 offset, b32 add_head)
{
	animation_frame result = {};

	r32 parts_count = add_head ? 2 : 1;

	result.sprite.parts_count = parts_count;
	result.sprite.parts = push_array(arena, parts_count, sprite_part);

	rect legs_rect = get_rect_from_min_corner(offset.x, offset.y + 24, 24, 24);
	result.sprite.parts[0] = get_sprite_part(textures::CHARSET, legs_rect);

	if (add_head)
	{
		result.sprite.parts[0].offset_in_pixels = get_v2(0.0f, -5.0f);
		rect head_rect = get_rect_from_min_corner(offset.x, offset.y, 24, 24);
		v2 head_offset = get_v2(5, -19);
		result.sprite.parts[1] = get_sprite_part(textures::CHARSET, head_rect, head_offset);
	}

	return result;
}

animation_frame get_player_idle_pose(memory_arena* arena)
{
	animation_frame result = {};	

	rect legs_rect = get_rect_from_min_corner(0, 24, 24, 24);
	v2 legs_offset = get_v2(0.0f, 0.0f);

	result.sprite.parts_count = 1;
	result.sprite.parts = push_array(arena, result.sprite.parts_count, sprite_part);

	result.sprite.parts[0].texture = textures::CHARSET;
	result.sprite.parts[0].texture_rect = legs_rect;
	result.sprite.parts[0].offset_in_pixels = legs_offset;
	result.sprite.parts[0].default_direction = direction::E;

	return result;
}

animation_frame get_bullet_graphics(memory_arena* arena, u32 x, u32 y)
{
	animation_frame result = {};

	v2 offset = get_v2(120, 48);

	rect texture_rect = get_rect_from_min_corner(
		offset.x + (x * 10), 
		offset.y + (y * 10), 
		10, 
		10);

	result.sprite.parts_count = 1;
	result.sprite.parts = push_array(arena, result.sprite.parts_count, sprite_part);
	result.sprite.parts[0].texture = textures::CHARSET;
	result.sprite.parts[0].texture_rect = texture_rect;
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

entity_type* add_entity_type(static_game_data* data, entity_type_enum type)
{
	assert(data->entity_types_count < ENTITY_TYPES_MAX_COUNT);

	entity_type* result = &data->entity_types[data->entity_types_count];
	data->entity_types_count++;
	
	result->type_enum = type;
	set_entity_type_ptr(data->entity_types_dict, type, result);

	return result;
}

entity_type* add_bullet_type(static_game_data* data)
{
	assert(data->bullet_types_count < BULLET_TYPES_MAX_COUNT);

	entity_type* result = &data->bullet_types[data->bullet_types_count];
	data->bullet_types_count++;

	return result;
}

map load_map(string_ref map_name, memory_arena* arena, memory_arena* transient_arena)
{
	map result = {};
	
	std::string map_file_path = "data/";
	map_file_path.append(map_name.ptr, map_name.string_size);
	map_file_path.append(".tmx");

	read_file_result map_file = read_file(map_file_path);
	result = read_map_from_tmx_file(arena, transient_arena, map_file, "map");
	delete map_file.contents;

	return result;
}

void initialize_level_state(level_state* level, static_game_data* static_data, string_ref map_name, memory_arena* arena)
{
	*level = {};

	level->current_map_name = copy_string(arena, map_name);

	level->static_data = static_data;
	
	level->entities_count = 0;
	level->entities_max_count = 1000;
	level->entities = push_array(arena, level->entities_max_count, entity);

	level->bullets_count = 0;
	level->bullets_max_count = 5000;
	level->bullets = push_array(arena, level->bullets_max_count, bullet);

	level->player_movement.current_mode = movement_mode::WALK;
	level->current_player_torso = level->static_data->player_shooting_right;
	level->flip_player_torso_horizontally = false;

	level->fade_in_perc = 1.0f;
}

gate_graphics load_gate_graphics(u32 index)
{
	assert(index < 4);

	v2 offset = get_v2(5 * 24.0f, 4 * 24.0f);
	
	u32 tile_x = 3 + index;
	u32 tile_y = 1;

	gate_graphics result = {};
	result.frame_upper = get_16x16_sprite_part(offset, textures::CHARSET, tile_x, tile_y);
	tile_y++;
	result.gate = get_16x16_sprite_part(offset, textures::CHARSET, tile_x, tile_y);
	tile_y++;
	result.frame_lower = get_16x16_sprite_part(offset, textures::CHARSET, tile_x, tile_y);

	return result;
}

switch_graphics load_switch_graphics(u32 index)
{
	v2 offset = get_v2(5 * 24.0f, 4 * 24.0f);

	u32 tile_x = 0;
	u32 tile_y = 1 + index;

	switch_graphics result = {};
	result.frame_left = get_16x16_sprite_part(offset, textures::CHARSET, tile_x, tile_y);
	tile_x++;
	result.frame_middle = get_16x16_sprite_part(offset, textures::CHARSET, tile_x, tile_y);
	tile_x++;
	result.frame_right = get_16x16_sprite_part(offset, textures::CHARSET, tile_x, tile_y);
	return result;
}

display_graphics load_gate_switch_displays()
{
	v2 offset = get_v2(5 * 24.0f, 4 * 24.0f);

	display_graphics result = {};
	result.switch_left_display = get_16x16_sprite_part(offset, textures::CHARSET, 0, 0);
	result.switch_middle_display = get_16x16_sprite_part(offset, textures::CHARSET, 1, 0);
	result.switch_right_display = get_16x16_sprite_part(offset, textures::CHARSET, 2, 0);
	result.gate_upper_display = get_16x16_sprite_part(offset, textures::CHARSET, 3, 0);
	result.gate_lower_display = get_16x16_sprite_part(offset, textures::CHARSET, 4, 0);
	return result;
}

animation_frame load_power_up_graphics(memory_arena* arena, u32 index)
{
	v2 offset = get_v2(5 * 24.0f, 4 * 24.0f);

	u32 tile_x = index;
	u32 tile_y = 5;

	animation_frame result = get_16x16_animation_frame(offset, arena, textures::CHARSET, tile_x, tile_y);
	return result;
}

ui_graphics load_ui_graphics()
{
	v2 offset = get_v2(192.0f, 48.0f);

	ui_graphics result = {};

	result.healthbar_icon = get_rect_from_min_corner(offset, get_v2(8, 8));
	rect r = get_rect_from_min_corner(offset + get_v2(8, 0), get_v2(4, 8));
	result.healthbar_empty_bar = r;
	result.healthbar_white_bar = move_rect(&r, get_v2(4, 0));
	result.healthbar_red_bar = move_rect(&r, get_v2(4, 0));

	r = get_rect_from_min_corner(offset + get_v2(0, 8), get_v2(4, 4));
	result.msgbox_frame_upper_left = r;
	result.msgbox_frame_upper = move_rect(&r, get_v2(4, 0));
	result.msgbox_frame_upper_right = move_rect(&r, get_v2(4, 0));
	result.msgbox_frame_right = move_rect(&r, get_v2(0, 4));
	result.msgbox_frame_lower_right = move_rect(&r, get_v2(0, 4));
	result.msgbox_frame_lower = move_rect(&r, get_v2(-4, 0));
	result.msgbox_frame_lower_left = move_rect(&r, get_v2(-4, 0));
	result.msgbox_frame_left = move_rect(&r, get_v2(0, -4));
	result.msgbox_frame_background = move_rect(&r, get_v2(4, 0));

	return result;
}

void load_static_game_data(static_game_data* data, memory_arena* arena, memory_arena* transient_arena)
{	
	temporary_memory transient_memory = begin_temporary_memory(transient_arena);

	data->menu_font = {};
	data->menu_font.height_in_pixels = 8;
	data->menu_font.width_in_pixels = 8;
	data->menu_font.letter_spacing = -1;
	data->menu_font.line_spacing = 4;
	data->menu_font.texture = textures::FONT;

	data->ui_font = {};
	data->ui_font.height_in_pixels = 8;
	data->ui_font.width_in_pixels = 8;
	data->ui_font.letter_spacing = -1;
	data->ui_font.line_spacing = 4;
	data->ui_font.texture = textures::FONT;

	data->ui_gfx = load_ui_graphics();
	data->menu_new_game_str = copy_c_string_to_memory_arena(arena, "New Game");
	data->menu_continue_str = copy_c_string_to_memory_arena(arena, "Continue");
	data->menu_credits_str = copy_c_string_to_memory_arena(arena, "Credits");
	data->menu_exit_str = copy_c_string_to_memory_arena(arena, "Exit");

	std::string collision_file_path = "data/collision_map.tmx";
	read_file_result collision_file = read_file(collision_file_path);
	data->collision_reference = read_map_from_tmx_file(arena, transient_arena, collision_file, "collision");
	delete collision_file.contents;

	data->blue_gate_graphics = load_gate_graphics(0);
	data->grey_gate_graphics = load_gate_graphics(1);
	data->red_gate_graphics = load_gate_graphics(2);
	data->green_gate_graphics = load_gate_graphics(3);
	data->blue_switch_graphics = load_switch_graphics(0);
	data->grey_switch_graphics = load_switch_graphics(1);
	data->red_switch_graphics = load_switch_graphics(2);
	data->green_switch_graphics = load_switch_graphics(3);
	data->gate_switch_displays = load_gate_switch_displays();

	data->player_shooting_up = get_square_sprite(arena, 24, 
		textures::CHARSET, 4, 0, get_v2(3.0f, -16.0f));
	data->player_shooting_up_bullet_offset = get_v2(0.2f, -1.4f);

	data->player_shooting_right_up = get_square_sprite(arena, 24, 
		textures::CHARSET, 1, 0, get_v2(5.0f, -16.0f));
	data->player_shooting_right_up_bullet_offset = get_v2(0.65f, -1.1f);

	data->player_shooting_right = get_square_sprite(arena, 24, 
		textures::CHARSET, 0, 0, get_v2(5.0f, -16.0f));
	data->player_shooting_right_bullet_offset = get_v2(0.85f, -0.60f);
	
	data->player_shooting_right_down = get_square_sprite(arena, 24, 
		textures::CHARSET, 2, 0, get_v2(4.0f, -10.0f));
	data->player_shooting_right_down_bullet_offset = get_v2(0.65f, -0.15f);

	data->player_shooting_down = get_square_sprite(arena, 24, 
		textures::CHARSET, 3, 0, get_v2(2.0f, -6.0f));
	data->player_shooting_down_bullet_offset = get_v2(0.25f, 0.15f);

	data->entity_types = push_array(arena, BULLET_TYPES_MAX_COUNT, entity_type);
	data->entity_types_count = 0;

	data->default_player_invincibility_cooldown = 2.0f;

	data->entity_types_dict = create_entity_types_dictionary(arena);

	entity_type* player_entity_type = add_entity_type(data, entity_type_enum::PLAYER);
	player_entity_type->idle_pose = get_player_idle_pose(arena);
	player_entity_type->flags = (entity_flags)((u32)entity_flags::BLOCKS_MOVEMENT | (u32)entity_flags::PLAYER);
	player_entity_type->max_health = 100;
	player_entity_type->velocity_multiplier = 40.0f;
	player_entity_type->slowdown_multiplier = 0.80f;
	player_entity_type->default_attack_cooldown = 0.2f;
	player_entity_type->walk_animation = get_walk_animation(arena, get_zero_v2(), false);
	player_entity_type->collision_rect_dim = get_v2(0.35f, 1.6f);

	entity_type* static_enemy_type = add_entity_type(data, entity_type_enum::STATIC_ENEMY);
	static_enemy_type->idle_pose = get_tile_graphics(arena, 837);
	static_enemy_type->flags = (entity_flags)((u32)entity_flags::BLOCKS_MOVEMENT | (u32)entity_flags::ENEMY);
	static_enemy_type->max_health = 10;
	static_enemy_type->damage_on_contact = 10;
	static_enemy_type->default_attack_cooldown = 0.5f;
	static_enemy_type->player_detecting_distance = 10.0f;
	static_enemy_type->player_acceleration_on_collision = 3.0f;
	static_enemy_type->collision_rect_dim = get_v2(1.0f, 1.0f);

	entity_type* moving_enemy_type = add_entity_type(data, entity_type_enum::MOVING_ENEMY);
	//moving_enemy_type->idle_pose = get_walk_idle_pose(arena, get_v2(0, 2 * 24), true);
	moving_enemy_type->idle_pose = get_walk_idle_pose(arena, get_v2(0, 4 * 24), true);
	moving_enemy_type->flags = (entity_flags)((u32)entity_flags::BLOCKS_MOVEMENT
		| (u32)entity_flags::WALKS_HORIZONTALLY
		| (u32)entity_flags::ENEMY);
	moving_enemy_type->max_health = 10;
	moving_enemy_type->damage_on_contact = 10;
	//moving_enemy_type->walk_animation = get_walk_animation(arena, get_v2(0, 2 * 24), true);
	moving_enemy_type->walk_animation = get_walk_animation(arena, get_v2(0, 4 * 24), true);
	moving_enemy_type->default_attack_cooldown = 0.2f;
	moving_enemy_type->velocity_multiplier = 4.0f;
	moving_enemy_type->player_acceleration_on_collision = 3.0f;
	moving_enemy_type->collision_rect_dim = get_v2(1.0f, 1.0f);

	data->bullet_types = push_array(arena, BULLET_TYPES_MAX_COUNT, entity_type);
	data->bullet_types_count = 0;

	entity_type* player_bullet_type = add_bullet_type(data);
	player_bullet_type->damage_on_contact = 5;
	player_bullet_type->constant_velocity = 12.0f;
	player_bullet_type->idle_pose = get_bullet_graphics(arena, 1, 1);

	entity_type* enemy_bullet_type = add_bullet_type(data);
	enemy_bullet_type->damage_on_contact = 5;
	enemy_bullet_type->flags = entity_flags::DAMAGES_PLAYER;
	enemy_bullet_type->constant_velocity = 12.0f;
	enemy_bullet_type->idle_pose = get_bullet_graphics(arena, 1, 0);

	entity_type* power_up_bullet_type = add_bullet_type(data);
	power_up_bullet_type->damage_on_contact = 10;
	power_up_bullet_type->constant_velocity = 16.0f;
	power_up_bullet_type->idle_pose = get_bullet_graphics(arena, 3, 1);

	player_entity_type->fired_bullet_type = player_bullet_type;
	static_enemy_type->fired_bullet_type = enemy_bullet_type;

	data->visual_effects_count = 5;
	data->visual_effects = push_array(arena, data->visual_effects_count, sprite_effect);

	sprite_effect* gate_display_fade_effect = &data->visual_effects[0];
	gate_display_fade_effect->stages_count = 1;
	gate_display_fade_effect->stages = push_array(arena, gate_display_fade_effect->stages_count, sprite_effect_stage);
	gate_display_fade_effect->stages[0].amplitude = 1.0f;
	gate_display_fade_effect->color = get_v4(100, 100, 100, 0);

	sprite_effect* damage_tint_effect = &data->visual_effects[1];
	damage_tint_effect->stages_count = 1;
	damage_tint_effect->stages = push_array(arena, damage_tint_effect->stages_count, sprite_effect_stage);
	damage_tint_effect->stages[0].amplitude = 1.0f;
	damage_tint_effect->color = get_v4(255, 0, 0, 0);

	sprite_effect* invinvibility_tint_effect = &data->visual_effects[2];
	invinvibility_tint_effect->stages_count = 1;
	invinvibility_tint_effect->stages = push_array(arena, invinvibility_tint_effect->stages_count, sprite_effect_stage);
	invinvibility_tint_effect->stages[0].period = 10.0f;
	invinvibility_tint_effect->stages[0].amplitude = 1.5f;
	invinvibility_tint_effect->flags = (sprite_effect_flags)((u32)sprite_effect_flags::REPEATS | (u32)sprite_effect_flags::ADDITIVE_MODE);
	invinvibility_tint_effect->color = get_v4(0, 0, 255, 0);

	add_sprite_effect_stage(damage_tint_effect, 1.0f, 0.0f, 0.0f, 5.0f, 5.0f);
	add_constant_tint_sprite_effect_stage(damage_tint_effect, 0.5f, 5.0f);

	entity_flags power_up_flags = (entity_flags)((u32)entity_flags::POWER_UP | (u32)entity_flags::INDESTRUCTIBLE);
	v2 power_up_size = get_v2(0.5f, 0.5f);
	entity_type* power_up_invincibility_type = add_entity_type(data, entity_type_enum::POWER_UP_INVINCIBILITY);
	power_up_invincibility_type->idle_pose = load_power_up_graphics(arena, 1);
	power_up_invincibility_type->flags = power_up_flags;
	power_up_invincibility_type->collision_rect_dim = power_up_size;

	entity_type* power_up_health_type = add_entity_type(data, entity_type_enum::POWER_UP_HEALTH);
	power_up_health_type->idle_pose = load_power_up_graphics(arena, 0);
	power_up_health_type->flags = power_up_flags;
	power_up_health_type->collision_rect_dim = power_up_size;

	entity_type* power_up_speed_type = add_entity_type(data, entity_type_enum::POWER_UP_SPEED);
	power_up_speed_type->idle_pose = load_power_up_graphics(arena, 2);
	power_up_speed_type->flags = power_up_flags;
	power_up_speed_type->collision_rect_dim = power_up_size;

	entity_type* power_up_damage_type = add_entity_type(data, entity_type_enum::POWER_UP_DAMAGE);
	power_up_damage_type->idle_pose = load_power_up_graphics(arena, 3);
	power_up_damage_type->flags = power_up_flags;
	power_up_damage_type->collision_rect_dim = power_up_size;

	entity_type* power_up_spread_type = add_entity_type(data, entity_type_enum::POWER_UP_SPREAD);
	power_up_spread_type->idle_pose = load_power_up_graphics(arena, 4);
	power_up_spread_type->flags = power_up_flags;
	power_up_spread_type->collision_rect_dim = power_up_size;

	end_temporary_memory(transient_memory, true);
}