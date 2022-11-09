#pragma once

#include "main.h"

typedef enum render_list_entry_type
{
    RENDER_LIST_ENTRY_CLEAR,
    RENDER_LIST_ENTRY_FADE,
    RENDER_LIST_ENTRY_BITMAP,
    RENDER_LIST_ENTRY_BITMAP_WITH_EFFECTS,
    RENDER_LIST_ENTRY_DEBUG_RECTANGLE,
} render_list_entry_type;

typedef struct render_list_entry_header
{
    render_list_entry_type type;
} render_list_entry_header;

typedef struct render_list_entry_clear
{
    v4 color;
} render_list_entry_clear;

typedef struct render_list_entry_fade
{
    v4 color;
    r32 percentage;
} render_list_entry_fade;

typedef struct render_list_entry_bitmap
{
    rect source_rect;
    rect destination_rect;
    textures texture;
} render_list_entry_bitmap;

typedef struct render_list_entry_bitmap_with_effects
{
    rect source_rect;
    rect destination_rect;
    textures texture;

    v4 tint_color;
    b32 render_in_additive_mode;
    b32 flip_horizontally;
} render_list_entry_bitmap_with_effects;

typedef struct render_list_entry_debug_rectangle
{
    v4 color;
    b32 render_outline_only;
    rect destination_rect;
} render_list_entry_debug_rectangle;

void* push_render_element(render_list* render, u32 size, render_list_entry_type type);
void render_bitmap(render_list* render, textures texture, rect source_rect, rect destination_rect);
void render_rectangle(render_list* render, rect screen_rect_to_fill, v4 color, b32 render_outline_only);
void render_point(render_list* render, v2 point, v4 color);
void render_clear(render_list* render, v4 color);
void render_fade(render_list* render, v4 color, r32 percentage);
void render_bitmap_with_effects(render_list* render, textures texture, 
    rect source_rect, rect destination_rect, v4 tint_color, b32 render_in_additive_mode, b32 flip_horizontally);

rect get_tile_bitmap_rect(u32 tile_id);
rect get_screen_rect(v2 position_relative_to_camera, v2 rect_size);
rect get_tile_screen_rect(v2 position_relative_to_camera);

void render_map_layer(render_list* render, map_layer layer, tile_position camera_tile_pos, v2 camera_offset_in_tile);
void render_entity_sprite(render_list* render, world_position camera_position, world_position entity_position, direction entity_direction,
    sprite_effect* visual_effect, r32 visual_effect_duration, sprite sprite);