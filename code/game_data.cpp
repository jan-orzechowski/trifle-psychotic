#include "main.h"
#include "level_parsing.h"
#include "jorutils.h"
#include "sdl_platform.h"
#include "entities.h"
#include "rendering.h"

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
	result.sprite.parts[0].texture_rect = get_tile_bitmap_rect(tile_value);
	return result;
}

u32 count_sprite_effects_types()
{
	u32 result = 0;
	for (i32 enum_value = (i32)sprite_effects_types::_FIRST;
		enum_value < (i32)sprite_effects_types::_LAST;
		enum_value++)
	{
		result++;
	}
	return result;
}

sprite_effect* add_sprite_effect(static_game_data* data, sprite_effects_types type)
{
	i32 index = (i32)type;
	assert(index > 0 && index < data->visual_effects_count);
	sprite_effect* result = &data->visual_effects[index];
	assert((i32)result->type == 0); // sprawdzamy, czy typ był nieużywany
	result->type = type;
	return result;
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

animation* get_walk_animation(memory_arena* arena, v2 bitmap_offset, b32 add_head, v2 display_offset = get_zero_v2())
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

	rect legs_rect = get_rect_from_min_corner(bitmap_offset.x + 24, bitmap_offset.y + 24, 24, 24);
	fill_animation_frame(new_animation, 0, 0, get_sprite_part(texture, legs_rect, display_offset), &frame_duration);
	legs_rect = move_rect(legs_rect, get_v2(24, 0));
	fill_animation_frame(new_animation, 1, 0, get_sprite_part(texture, legs_rect, display_offset), &frame_duration);
	legs_rect = move_rect(legs_rect, get_v2(24, 0));
	fill_animation_frame(new_animation, 2, 0, get_sprite_part(texture, legs_rect, display_offset), &frame_duration);

	if (add_head) 
	{
		new_animation->frames[0].sprite.parts[0].offset_in_pixels = get_v2(0.0f, -5.0f);
		new_animation->frames[1].sprite.parts[0].offset_in_pixels = get_v2(0.0f, -5.0f);
		new_animation->frames[2].sprite.parts[0].offset_in_pixels = get_v2(0.0f, -5.0f);

		rect head_rect = get_rect_from_min_corner(bitmap_offset.x, bitmap_offset.y, 24, 24);
		v2 head_offset = get_v2(5, -19) + display_offset;
		fill_animation_frame(new_animation, 0, 1, get_sprite_part(texture, head_rect, head_offset), NULL);	
		fill_animation_frame(new_animation, 1, 1, get_sprite_part(texture, head_rect, head_offset), NULL);
		fill_animation_frame(new_animation, 2, 1, get_sprite_part(texture, head_rect, head_offset), NULL);
	}

	return new_animation;
}

animation_frame get_walk_idle_pose(memory_arena* arena, v2 bitmap_offset, b32 add_head, v2 display_offset = get_zero_v2())
{
	animation_frame result = {};

	r32 parts_count = add_head ? 2 : 1;

	result.sprite.parts_count = parts_count;
	result.sprite.parts = push_array(arena, parts_count, sprite_part);

	rect legs_rect = get_rect_from_min_corner(bitmap_offset.x, bitmap_offset.y + 24, 24, 24);
	result.sprite.parts[0] = get_sprite_part(textures::CHARSET, legs_rect, display_offset);

	if (add_head)
	{
		result.sprite.parts[0].offset_in_pixels = get_v2(0.0f, -5.0f);
		rect head_rect = get_rect_from_min_corner(bitmap_offset.x, bitmap_offset.y, 24, 24);
		v2 head_offset = get_v2(5, -19) + display_offset;
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

animation_frame get_animation_frame_from_sprite(memory_arena* arena, sprite sprite)
{
	animation_frame result = {};
	result.sprite = sprite;
	return result;
}

animation* get_animation_from_sprite(memory_arena* arena, sprite sprite)
{
	animation* new_animation = push_struct(arena, animation);
	new_animation->frames_count = 1;
	new_animation->frames = push_array(arena, new_animation->frames_count, animation_frame);
	new_animation->frames[0].sprite = sprite;
	new_animation->frames[0].duration = 0.0f;
	return new_animation;
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

tmx_map_parsing_result load_map(string_ref map_name, memory_arena* arena, memory_arena* transient_arena)
{
	tmx_map_parsing_result result = {};
	
	std::string map_file_path = "data/";
	map_file_path.append(map_name.ptr, map_name.string_size);
	map_file_path.append(".tmx");

	read_file_result map_file = read_file(map_file_path);
	result = read_map_from_tmx_file(arena, transient_arena, map_file, "map", false);
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

	level->explosions_count = 0;
	level->explosions_max_count = 500;
	level->explosions = push_array(arena, level->explosions_max_count, entity);

	level->player_movement.current_mode = movement_mode::WALK;
	level->player_movement.standing_history.buffer_size = 10;
	level->player_movement.standing_history.buffer = push_array(arena,
		level->player_movement.standing_history.buffer_size, b32);

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

moving_platform_graphics load_moving_platform_graphics(u32 x, u32 y)
{
	v2 offset = get_v2(120.0f, 0.0f);

	moving_platform_graphics result = {};
	result.left = get_16x16_sprite_part(offset, textures::CHARSET, 3 * x, y);
	result.middle = get_16x16_sprite_part(offset, textures::CHARSET, (3 * x) + 1, y);
	result.right = get_16x16_sprite_part(offset, textures::CHARSET, (3 * x) + 2, y);

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

	r = get_rect_from_min_corner(get_v2(192.0f, 68.0f), get_v2(15, 5));
	result.msgbox_dots_1 = r;
	result.msgbox_dots_2 = move_rect(&r, get_v2(0, 5));
	result.msgbox_dots_3 = move_rect(&r, get_v2(0, 5));

	result.crosshair = get_rect_from_min_corner(get_v2(216, 48), get_v2(13, 13));
	result.menu_indicator = get_rect_from_min_corner(136, 176, 16, 16);

	return result;
}

animation* load_explosion_animation(memory_arena* arena, v2 offset, u32 tile_side, u32 frames_count, r32 single_frame_duration)
{
	animation* new_animation = push_struct(arena, animation);
	new_animation->frames_count = frames_count;
	new_animation->frames = push_array(arena, new_animation->frames_count, animation_frame);

	textures texture = textures::EXPLOSION;
	rect frame_rect = get_rect_from_min_corner(offset, get_v2(tile_side, tile_side));

	for (u32 frame_index = 0; frame_index < frames_count; frame_index++)
	{
		new_animation->frames[frame_index].sprite.parts_count = 1;
		new_animation->frames[frame_index].sprite.parts = push_array(arena, 1, sprite_part);
		fill_animation_frame(new_animation, frame_index, 0, get_sprite_part(texture, frame_rect), &single_frame_duration);
		move_rect(&frame_rect, get_v2(tile_side, 0));
	}

	return new_animation;
}

void add_death_animation(memory_arena* arena, entity_type* type, animation* death_animation)
{
	type->death_animation_variants = push_struct(arena, animation*);
	*type->death_animation_variants = death_animation;
	type->death_animation_variants_count = 1;
}

void add_death_animations(memory_arena* arena, entity_type* type, 
	animation* death_animation_1, animation* death_animation_2, animation* death_animation_3)
{
	type->death_animation_variants = push_array(arena, 3, animation*);
	type->death_animation_variants[0] = death_animation_1;
	type->death_animation_variants[1] = death_animation_2;
	type->death_animation_variants[2] = death_animation_3;
	type->death_animation_variants_count = 3;
}

void add_death_animations_standard(memory_arena* arena, entity_type* type, static_game_data* data)
{
	type->death_animation_variants = push_array(arena, 6, animation*);
	type->death_animation_variants[0] = data->explosion_animations.size_16x16_variant_1;
	type->death_animation_variants[1] = data->explosion_animations.size_16x16_variant_2;
	type->death_animation_variants[2] = data->explosion_animations.size_16x16_variant_3;
	type->death_animation_variants[3] = data->explosion_animations.size_16x16_variant_4;
	type->death_animation_variants[4] = data->explosion_animations.size_16x16_variant_5;
	type->death_animation_variants[5] = data->explosion_animations.size_16x16_variant_6;
	type->death_animation_variants_count = 6;
}

shooting_rotation_sprites* load_shooting_rotation_sprites_with_offset(memory_arena* arena, u32 tile_y, v2 offset_in_pixels = get_zero_v2())
{
	shooting_rotation_sprites* result = push_struct(arena, shooting_rotation_sprites);
	v2 offset_in_tiles = offset_in_pixels / TILE_SIDE_IN_PIXELS;

	result->up = get_square_sprite(arena, 24,
		textures::CHARSET, 4, tile_y, get_v2(3.0f, -16.0f) + offset_in_pixels);
	result->up_bullet_offset = get_v2(0.2f, -1.4f) + offset_in_tiles;

	result->right_up = get_square_sprite(arena, 24,
		textures::CHARSET, 1, tile_y, get_v2(5.0f, -16.0f) + offset_in_pixels);
	result->right_up_bullet_offset = get_v2(0.65f, -1.1f) + offset_in_tiles;

	result->right = get_square_sprite(arena, 24,
		textures::CHARSET, 0, tile_y, get_v2(5.0f, -16.0f) + offset_in_pixels);
	result->right_bullet_offset = get_v2(0.85f, -0.60f) + offset_in_tiles;

	result->right_down = get_square_sprite(arena, 24,
		textures::CHARSET, 2, tile_y, get_v2(4.0f, -10.0f) + offset_in_pixels);
	result->right_down_bullet_offset = get_v2(0.65f, -0.15f) + offset_in_tiles;

	result->down = get_square_sprite(arena, 24,
		textures::CHARSET, 3, tile_y, get_v2(2.0f, -6.0f) + offset_in_pixels);
	result->down_bullet_offset = get_v2(0.25f, 0.15f) + offset_in_tiles;

	return result;
}

shooting_rotation_sprites* load_shooting_rotation_sprites(memory_arena* arena, u32 tile_y)
{
	shooting_rotation_sprites* result = push_struct(arena, shooting_rotation_sprites);

	result->up = get_square_sprite(arena, 24, textures::CHARSET, 4, tile_y);
	result->right_up = get_square_sprite(arena, 24, textures::CHARSET, 1, tile_y);
	result->right = get_square_sprite(arena, 24, textures::CHARSET, 0, tile_y);
	result->right_down = get_square_sprite(arena, 24, textures::CHARSET, 2, tile_y);
	result->down = get_square_sprite(arena, 24, textures::CHARSET, 3, tile_y);

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
	data->collision_reference = read_collision_map(arena, transient_arena, collision_file);
	delete collision_file.contents;

	data->gates_gfx.blue = load_gate_graphics(0);
	data->gates_gfx.grey = load_gate_graphics(1);
	data->gates_gfx.red = load_gate_graphics(2);
	data->gates_gfx.green = load_gate_graphics(3);
	data->switches_gfx.blue = load_switch_graphics(0);
	data->switches_gfx.grey = load_switch_graphics(1);
	data->switches_gfx.red = load_switch_graphics(2);
	data->switches_gfx.green = load_switch_graphics(3);
	data->display_gfx = load_gate_switch_displays();
	data->platforms_gfx.blue = load_moving_platform_graphics(0, 0);
	data->platforms_gfx.grey = load_moving_platform_graphics(0, 1);
	data->platforms_gfx.red = load_moving_platform_graphics(1, 0);
	data->platforms_gfx.green = load_moving_platform_graphics(1, 1);

	data->explosion_animations.size_16x16_variant_1 = load_explosion_animation(arena, get_v2(464, 0), 16, 7, 0.1f);
	data->explosion_animations.size_16x16_variant_2 = load_explosion_animation(arena, get_v2(464, 16), 16, 7, 0.1f);
	data->explosion_animations.size_16x16_variant_3 = load_explosion_animation(arena, get_v2(464, 32), 16, 7, 0.1f);
	data->explosion_animations.size_16x16_variant_4 = load_explosion_animation(arena, get_v2(464 + (16 * 7), 0), 16, 7, 0.1f);
	data->explosion_animations.size_16x16_variant_5 = load_explosion_animation(arena, get_v2(464 + (16 * 7), 16), 16, 7, 0.1f);
	data->explosion_animations.size_16x16_variant_6 = load_explosion_animation(arena, get_v2(464 + (16 * 7), 32), 16, 7, 0.1f);
	data->explosion_animations.size_16x16_variant_7_blue = load_explosion_animation(arena, get_v2(464 + (16 * 7) * 2, 32), 16, 7, 0.1f);
	data->explosion_animations.size_16x16_variant_8_blue = load_explosion_animation(arena, get_v2(464 + (16 * 7) * 2, 32), 16, 7, 0.1f);
	data->explosion_animations.size_16x16_variant_9_blue = load_explosion_animation(arena, get_v2(464 + (16 * 7) * 2, 32), 16, 7, 0.1f);
	data->explosion_animations.size_24x24 = load_explosion_animation(arena, get_v2(0, 0), 24, 12, 0.15f);
	data->explosion_animations.size_32x32 = load_explosion_animation(arena, get_v2(0, 24), 32, 12, 0.15f);
	data->explosion_animations.size_48x48 = load_explosion_animation(arena, get_v2(0, 24 + 32), 48, 12, 0.15f);
	data->explosion_animations.size_96x96 = load_explosion_animation(arena, get_v2(0, 24 + 32 + 48), 96, 12, 0.2f);
	
	data->entity_types = push_array(arena, ENTITY_TYPES_MAX_COUNT, entity_type);
	data->entity_types_count = 0;

	data->default_player_invincibility_cooldown = 0.2f;

	data->entity_types_dict = create_entity_types_dictionary(arena);

	data->bullet_types = push_array(arena, BULLET_TYPES_MAX_COUNT, entity_type);
	data->bullet_types_count = 0;

	entity_type* player_entity_type = add_entity_type(data, entity_type_enum::PLAYER);
	player_entity_type->idle_pose = get_player_idle_pose(arena);
	player_entity_type->flags = (entity_flags)((u32)entity_flags::BLOCKS_MOVEMENT | (u32)entity_flags::PLAYER);
	player_entity_type->max_health = 40;
	player_entity_type->velocity_multiplier = 40.0f;
	player_entity_type->slowdown_multiplier = 0.80f;
	player_entity_type->default_attack_cooldown = 0.2f;
	player_entity_type->walk_animation = get_walk_animation(arena, get_zero_v2(), false);
	player_entity_type->collision_rect_dim = get_v2(0.35f, 1.6f);
	player_entity_type->death_animation_offset = get_v2(0.0f, -0.75f);
	player_entity_type->rotation_sprites = load_shooting_rotation_sprites_with_offset(arena, 0);
	add_death_animation(arena, player_entity_type, data->explosion_animations.size_48x48);

	entity_type* player_bullet_type = add_bullet_type(data);
	player_bullet_type->damage_on_contact = 10;
	player_bullet_type->constant_velocity = 14.0f;
	player_bullet_type->idle_pose = get_bullet_graphics(arena, 1, 1);
	add_death_animations_standard(arena, player_bullet_type, data);

	player_entity_type->fired_bullet_type = player_bullet_type;

	entity_type* power_up_bullet_type = add_bullet_type(data);
	power_up_bullet_type->damage_on_contact = 20;
	power_up_bullet_type->constant_velocity = 18.0f;
	power_up_bullet_type->idle_pose = get_bullet_graphics(arena, 3, 1);
	add_death_animations(arena, power_up_bullet_type,
		data->explosion_animations.size_16x16_variant_7_blue,
		data->explosion_animations.size_16x16_variant_8_blue,
		data->explosion_animations.size_16x16_variant_9_blue);

	data->player_normal_bullet_type = player_bullet_type;
	data->player_power_up_bullet_type = power_up_bullet_type;

	entity_type* sentry_type = add_entity_type(data, entity_type_enum::ENEMY_SENTRY);
	sentry_type->flags = (entity_flags)(
		(u32)entity_flags::BLOCKS_MOVEMENT 
		| (u32)entity_flags::ENEMY
		| (u32)entity_flags::DESTRUCTION_ON_PLAYER_CONTACT);
	sentry_type->detection_type = detection_type::DETECT_360_DEGREES;
	sentry_type->detection_distance = 12.0f;
	sentry_type->stop_movement_distance = 4.0f;
	sentry_type->forget_detection_distance = 14.0f;

	sentry_type->default_attack_cooldown = 3.0f;
	sentry_type->default_attack_series_duration = 0.6f;
	sentry_type->default_attack_bullet_interval_duration = 0.2f;

	sentry_type->max_health = 30;
	sentry_type->damage_on_contact = 10;
	sentry_type->velocity_multiplier = 0.0f;
	sentry_type->player_acceleration_on_collision = 3.0f;
	sentry_type->collision_rect_dim = get_v2(0.75f, 0.75f);
	sentry_type->rotation_sprites = load_shooting_rotation_sprites(arena, 6);
	add_death_animation(arena, sentry_type, data->explosion_animations.size_24x24);

	entity_type* sentry_bullet_type = add_bullet_type(data);
	sentry_bullet_type->damage_on_contact = 10;
	sentry_bullet_type->flags = entity_flags::DAMAGES_PLAYER;
	sentry_bullet_type->constant_velocity = 8.0f;
	sentry_bullet_type->idle_pose = get_bullet_graphics(arena, 1, 0);
	add_death_animations_standard(arena, sentry_bullet_type, data);

	sentry_type->fired_bullet_type = sentry_bullet_type;

	entity_type* guardian_type = add_entity_type(data, entity_type_enum::ENEMY_GUARDIAN);
	guardian_type->idle_pose = get_animation_frame_from_sprite(arena, get_square_sprite(arena, 24, textures::CHARSET, 1, 7));
	guardian_type->walk_animation = get_animation_from_sprite(arena, get_square_sprite(arena, 24, textures::CHARSET, 1, 7));
	guardian_type->flags = (entity_flags)(
		(u32)entity_flags::BLOCKS_MOVEMENT 
		| (u32)entity_flags::ENEMY
		| (u32)entity_flags::DESTRUCTION_ON_PLAYER_CONTACT
		| (u32)entity_flags::FLIES_VERTICALLY);
	guardian_type->detection_type = detection_type::DETECT_360_DEGREES;
	guardian_type->detection_distance = 10.0f;
	guardian_type->stop_movement_distance = 4.0f;
	guardian_type->forget_detection_distance = 12.0f;

	guardian_type->default_attack_cooldown = 1.5f;
	guardian_type->default_attack_series_duration = 0.5f;
	guardian_type->default_attack_bullet_interval_duration = 1.0f;

	guardian_type->max_health = 50;
	guardian_type->damage_on_contact = 20;
	guardian_type->velocity_multiplier = 3.0f;
	guardian_type->default_attack_cooldown = 0.5f;
	guardian_type->player_acceleration_on_collision = 3.0f;
	guardian_type->collision_rect_dim = get_v2(1.0f, 1.0f);
	add_death_animation(arena, guardian_type, data->explosion_animations.size_32x32);

	entity_type* guardian_bullet_type = add_bullet_type(data);
	guardian_bullet_type->damage_on_contact = 20;
	guardian_bullet_type->flags = entity_flags::DAMAGES_PLAYER;
	guardian_bullet_type->constant_velocity = 4.0f;
	guardian_bullet_type->idle_pose = get_bullet_graphics(arena, 0, 3);
	add_death_animations(arena, guardian_bullet_type,
		data->explosion_animations.size_16x16_variant_7_blue,
		data->explosion_animations.size_16x16_variant_8_blue,
		data->explosion_animations.size_16x16_variant_9_blue);

	guardian_type->fired_bullet_type = guardian_bullet_type;

	entity_type* flying_bomb_type = add_entity_type(data, entity_type_enum::ENEMY_FLYING_BOMB);
	flying_bomb_type->idle_pose = get_animation_frame_from_sprite(arena, get_square_sprite(arena, 24, textures::CHARSET, 0, 7));
	flying_bomb_type->flags = (entity_flags)(
		(u32)entity_flags::BLOCKS_MOVEMENT 
		| (u32)entity_flags::ENEMY
		| (u32)entity_flags::FLIES_TOWARDS_PLAYER
		| (u32)entity_flags::DESTRUCTION_ON_PLAYER_CONTACT);
	flying_bomb_type->detection_type = detection_type::DETECT_360_DEGREES;
	flying_bomb_type->detection_distance = 10.0f;
	flying_bomb_type->forget_detection_distance = 12.0f;
	
	flying_bomb_type->max_health = 20;
	flying_bomb_type->damage_on_contact = 100.0f;
	flying_bomb_type->velocity_multiplier = 2.0f;
	flying_bomb_type->default_attack_cooldown = 0.5f;
	flying_bomb_type->player_acceleration_on_collision = 3.0f;
	flying_bomb_type->collision_rect_dim = get_v2(1.1f, 0.5f);
	add_death_animation(arena, flying_bomb_type, data->explosion_animations.size_96x96);

	entity_type* robot_type = add_entity_type(data, entity_type_enum::ENEMY_ROBOT);
	robot_type->idle_pose = get_walk_idle_pose(arena, get_v2(0, 2 * 24), true);
	robot_type->flags = (entity_flags)((u32)entity_flags::BLOCKS_MOVEMENT
		| (u32)entity_flags::WALKS_HORIZONTALLY
		| (u32)entity_flags::ENEMY
		| (u32)entity_flags::DESTRUCTION_ON_PLAYER_CONTACT);

	robot_type->detection_type = detection_type::DETECT_180_DEGREES_IN_FRONT;
	robot_type->detection_distance = 7.0f;
	robot_type->stop_movement_distance = 4.0f;
	robot_type->forget_detection_distance = 12.0f;
	robot_type->looking_position_offset = get_v2(0.0f, -1.0f);

	robot_type->max_health = 10;
	robot_type->damage_on_contact = 10;
	robot_type->walk_animation = get_walk_animation(arena, get_v2(0, 2 * 24), true);
	robot_type->default_attack_cooldown = 0.2f;
	robot_type->velocity_multiplier = 3.0f;
	robot_type->player_acceleration_on_collision = 3.0f;
	robot_type->collision_rect_dim = get_v2(0.35f, 1.6f);
	robot_type->default_attack_cooldown = 1.0f;
	robot_type->default_attack_series_duration = 1.5f;
	robot_type->default_attack_bullet_interval_duration = 0.2f;

	add_death_animation(arena, robot_type, data->explosion_animations.size_48x48);
	robot_type->death_animation_offset = get_v2(0.0f, -0.75f);

	entity_type* robot_bullet_type = add_bullet_type(data);
	robot_bullet_type->damage_on_contact = 5;
	robot_bullet_type->flags = entity_flags::DAMAGES_PLAYER;
	robot_bullet_type->constant_velocity = 12.0f;
	robot_bullet_type->idle_pose = get_bullet_graphics(arena, 1, 0);
	add_death_animations_standard(arena, robot_bullet_type, data);

	robot_type->fired_bullet_type = robot_bullet_type;

	entity_type* cultist_type = add_entity_type(data, entity_type_enum::ENEMY_CULTIST);
	cultist_type->idle_pose = get_walk_idle_pose(arena, get_v2(0, 4 * 24), false, get_v2(0.0f, -4.0f));
	cultist_type->flags = (entity_flags)((u32)entity_flags::BLOCKS_MOVEMENT
		| (u32)entity_flags::WALKS_HORIZONTALLY
		| (u32)entity_flags::ENEMY
		| (u32)entity_flags::PLAYER_RECOIL_ON_CONTACT);
	cultist_type->detection_type = detection_type::DETECT_180_DEGREES_IN_FRONT;
	cultist_type->detection_distance = 12.0f;
	cultist_type->stop_movement_distance = 6.0f;
	cultist_type->forget_detection_distance = 15.0f;
	cultist_type->looking_position_offset = get_v2(0.0f, -1.2f);

	cultist_type->max_health = 15;
	cultist_type->damage_on_contact = 10;
	cultist_type->walk_animation = get_walk_animation(arena, get_v2(0, 4 * 24), false, get_v2(0.0f, -5.0f));
	cultist_type->default_attack_cooldown = 0.5f;
	cultist_type->default_attack_series_duration = 0.4f;
	cultist_type->default_attack_bullet_interval_duration = 0.05f;
		
	cultist_type->velocity_multiplier = 4.0f;
	cultist_type->player_acceleration_on_collision = 3.0f;
	cultist_type->collision_rect_dim = get_v2(0.4f, 1.7f);
	cultist_type->collision_rect_offset = get_v2(0.0f, -0.4f);
	add_death_animation(arena, cultist_type, data->explosion_animations.size_48x48);
	cultist_type->death_animation_offset = get_v2(0.0f, -0.75f);
	cultist_type->rotation_sprites = load_shooting_rotation_sprites_with_offset(arena, 4, get_v2(0, -4));

	entity_type* cultist_bullet_type = add_bullet_type(data);
	cultist_bullet_type->damage_on_contact = 10;
	cultist_bullet_type->flags = entity_flags::DAMAGES_PLAYER;
	cultist_bullet_type->constant_velocity = 12.0f;
	cultist_bullet_type->idle_pose = get_bullet_graphics(arena, 3, 0);
	add_death_animations_standard(arena, cultist_bullet_type, data);

	cultist_type->fired_bullet_type = cultist_bullet_type;


	// sprite effects
	{
		data->visual_effects_count = count_sprite_effects_types();
		data->visual_effects = push_array(arena, data->visual_effects_count, sprite_effect);

		sprite_effect* death_effect = add_sprite_effect(data, sprite_effects_types::DEATH);
		death_effect->stages_count = 1;
		death_effect->stages = push_array(arena, death_effect->stages_count, sprite_effect_stage);
		death_effect->stages[0].period = 0.0f;
		death_effect->stages[0].amplitude = 1.0f;
		death_effect->total_duration = 0.0f;
		death_effect->color = get_v4(255, 0, 0, 0);

		sprite_effect* bullet_hit_effect = add_sprite_effect(data, sprite_effects_types::BULLET_HIT);
		bullet_hit_effect->stages_count = 1;
		bullet_hit_effect->stages = push_array(arena, bullet_hit_effect->stages_count, sprite_effect_stage);
		bullet_hit_effect->stages[0].period = 1.0f;
		bullet_hit_effect->stages[0].amplitude = 0.8f;
		bullet_hit_effect->total_duration = 1.0f;
		bullet_hit_effect->flags = sprite_effect_flags::ADDITIVE_MODE;
		bullet_hit_effect->color = get_v4(255, 95, 31, 0);

		sprite_effect* invinvibility_effect = add_sprite_effect(data, sprite_effects_types::INVINCIBILITY);
		invinvibility_effect->stages_count = 1;
		invinvibility_effect->stages = push_array(arena, invinvibility_effect->stages_count, sprite_effect_stage);
		invinvibility_effect->stages[0].period = 10.0f;
		invinvibility_effect->stages[0].amplitude = 1.5f;
		invinvibility_effect->flags = (sprite_effect_flags)((u32)sprite_effect_flags::REPEATS | (u32)sprite_effect_flags::ADDITIVE_MODE);
		invinvibility_effect->color = get_v4(0, 0, 255, 0);

		sprite_effect* shock_effect = add_sprite_effect(data, sprite_effects_types::SHOCK);
		shock_effect->stages_count = 1;
		shock_effect->stages = push_array(arena, shock_effect->stages_count, sprite_effect_stage);
		shock_effect->stages[0].period = 1.5f;
		shock_effect->stages[0].amplitude = 1.5f;
		shock_effect->total_duration = 1.5f;
		shock_effect->flags = sprite_effect_flags::ADDITIVE_MODE;
		shock_effect->color = get_v4(255, 255, 255, 0);

		sprite_effect* recoil_effect = add_sprite_effect(data, sprite_effects_types::RECOIL);
		recoil_effect->stages_count = 1;
		recoil_effect->stages = push_array(arena, shock_effect->stages_count, sprite_effect_stage);
		recoil_effect->stages[0].period = 0.5f;
		recoil_effect->stages[0].amplitude = 1.5f;
		recoil_effect->flags = (sprite_effect_flags)((u32)sprite_effect_flags::REPEATS | (u32)sprite_effect_flags::ADDITIVE_MODE);
		recoil_effect->color = get_v4(255, 255, 255, 0);

		sprite_effect* gate_display_fade_effect = add_sprite_effect(data, sprite_effects_types::GATE_DISPLAY_INACTIVE);
		gate_display_fade_effect->stages_count = 1;
		gate_display_fade_effect->stages = push_array(arena, gate_display_fade_effect->stages_count, sprite_effect_stage);
		gate_display_fade_effect->stages[0].amplitude = 1.2f;
		gate_display_fade_effect->color = get_v4(100, 100, 100, 0);

		sprite_effect* speedup_effect = add_sprite_effect(data, sprite_effects_types::SPEED);
		speedup_effect->stages_count = 1;
		speedup_effect->stages = push_array(arena, shock_effect->stages_count, sprite_effect_stage);
		speedup_effect->stages[0].amplitude = 1.2f;
		speedup_effect->flags = (sprite_effect_flags)((u32)sprite_effect_flags::REPEATS | (u32)sprite_effect_flags::ADDITIVE_MODE);
		speedup_effect->color = get_v4(0, 120, 0, 0);
	}
	
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