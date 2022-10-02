#pragma once

#include "main.h"

enum class render_group_entry_type
{
	CLEAR,
	FADE,
	BITMAP,
	BITMAP_WITH_EFFECTS,
	DEBUG_RECTANGLE,
};

struct render_group_entry_header
{
	render_group_entry_type type;
};

struct render_group_entry_clear
{
	v4 color;
};

struct render_group_entry_fade
{
	v4 color;
	r32 percentage;
};

struct render_group_entry_bitmap
{
	rect source_rect;
	rect destination_rect;
	textures texture;
};

struct render_group_entry_bitmap_with_effects
{
	rect source_rect;
	rect destination_rect;
	textures texture;

	v4 tint_color;
	b32 render_in_additive_mode;
	b32 flip_horizontally;
};

struct render_group_entry_debug_rectangle
{
	v4 color;
	b32 render_outline_only;
	rect destination_rect;
};

void* push_render_element(render_group* group, u32 size, render_group_entry_type type);
void render_bitmap(render_group* group, textures texture, rect source_rect, rect destination_rect);
void render_rectangle(render_group* group, rect screen_rect_to_fill, v4 color, b32 render_outline_only);
void render_point(render_group* group, v2 point, v4 color);
void render_clear(render_group* group, v4 color);
void render_fade(render_group* group, v4 color, r32 percentage);
void render_bitmap_with_effects(render_group* group, textures texture, 
	rect source_rect, rect destination_rect, v4 tint_color, b32 render_in_additive_mode, b32 flip_horizontally);

rect get_tile_bitmap_rect(u32 tile_id);
rect get_screen_rect(v2 position_relative_to_camera, v2 rect_size);
rect get_tile_screen_rect(v2 position_relative_to_camera);

void debug_render_tile(render_group* render, tile_position tile_pos, v4 color, world_position camera_pos);
void render_entity_sprite(render_group* render, world_position camera_position, world_position entity_position, direction entity_direction,
	sprite_effect* visual_effect, r32 visual_effect_duration, sprite sprite);
void render_textbox(static_game_data* static_data, render_group* group, rect textbox_rect);