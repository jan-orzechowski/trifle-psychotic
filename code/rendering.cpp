#include "main.h"
#include "jormath.h"
#include "rendering.h"
#include "map.h"
#include "animation.h"

void* push_render_element(render_group* group, u32 size, render_group_entry_type type)
{
	render_group_entry_header* result = 0;
	u32 header_size = sizeof(render_group_entry_header);

	if ((group->push_buffer_size + size + header_size) < group->max_push_buffer_size)
	{
		result = (render_group_entry_header*)(group->push_buffer_base + group->push_buffer_size);
		result->type = type;

		// dajemy wskaźnik do elementu, nie do headera - ten zostanie odczytany w render_group_to_output
		result = result + 1; // przesuwamy się o rozmiar headera
		group->push_buffer_size += (size + header_size);
	}
	else
	{
		invalid_code_path;
	}

	return result;
}

void render_bitmap(render_group* group, textures texture, rect source_rect, rect destination_rect)
{
	render_group_entry_bitmap* entry = (render_group_entry_bitmap*)push_render_element(
		group, sizeof(render_group_entry_bitmap), render_group_entry_type::BITMAP);

	entry->source_rect = source_rect;
	entry->destination_rect = destination_rect;
	entry->texture = texture;
}

void render_rectangle(render_group* group, rect screen_rect_to_fill, v4 color, b32 render_outline_only)
{
	render_group_entry_debug_rectangle* entry = (render_group_entry_debug_rectangle*)push_render_element(
		group, sizeof(render_group_entry_debug_rectangle), render_group_entry_type::DEBUG_RECTANGLE);

	entry->destination_rect = screen_rect_to_fill;
	entry->color = color;
	entry->render_outline_only = render_outline_only;
}

void render_point(render_group* group, v2 point, v4 color)
{
	rect screen_rect = get_rect_from_center(point, get_v2(2.0f, 2.0f));
	render_rectangle(group, screen_rect, color, false);
}

void render_clear(render_group* group, v4 color)
{
	render_group_entry_clear* entry = (render_group_entry_clear*)push_render_element(
		group, sizeof(render_group_entry_clear), render_group_entry_type::CLEAR);

	entry->color = color;
}

void render_fade(render_group* group, v4 color, r32 percentage)
{
	render_group_entry_fade* entry = (render_group_entry_fade*)push_render_element(
		group, sizeof(render_group_entry_fade), render_group_entry_type::FADE);

	entry->color = color;
	entry->percentage = percentage;
}

void render_bitmap_with_effects(render_group* group,
	textures texture, rect source_rect, rect destination_rect, v4 tint_color, b32 render_in_additive_mode, b32 flip_horizontally)
{
	render_group_entry_bitmap_with_effects* entry = (render_group_entry_bitmap_with_effects*)push_render_element(
		group, sizeof(render_group_entry_bitmap_with_effects), render_group_entry_type::BITMAP_WITH_EFFECTS);

	entry->source_rect = source_rect;
	entry->destination_rect = destination_rect;
	entry->texture = texture;
	entry->tint_color = tint_color;
	entry->render_in_additive_mode = render_in_additive_mode;
	entry->flip_horizontally = flip_horizontally;
}

rect get_tile_bitmap_rect(u32 tile_id)
{
	rect result = {};
	if (tile_id == 0)
	{

	}
	else
	{
		// id liczą się od 1, nie od zera
		u32 column = (tile_id - 1) % TILESET_WIDTH_IN_TILES;
		u32 row = (tile_id - 1) / TILESET_WIDTH_IN_TILES;

		u32 x = column * TILE_SIDE_IN_PIXELS;
		u32 y = row * TILE_SIDE_IN_PIXELS;

		result = get_rect_from_min_corner(get_v2(x, y), get_v2(TILE_SIDE_IN_PIXELS, TILE_SIDE_IN_PIXELS));
	}

	return result;
}

rect get_screen_rect(v2 position_relative_to_camera, v2 rect_size)
{
	r32 w = rect_size.x;
	r32 h = rect_size.y;
	r32 x = SCREEN_CENTER_IN_PIXELS.x + (position_relative_to_camera.x * TILE_SIDE_IN_PIXELS)
		- (rect_size.x / 2);
	r32 y = SCREEN_CENTER_IN_PIXELS.y + (position_relative_to_camera.y * TILE_SIDE_IN_PIXELS)
		- (rect_size.y / 2);

	rect result = get_rect_from_min_corner(x, y, w, h);
	return result;
}

rect get_tile_screen_rect(v2 position_relative_to_camera)
{
	r32 w = TILE_SIDE_IN_PIXELS;
	r32 h = TILE_SIDE_IN_PIXELS;
	r32 x = SCREEN_CENTER_IN_PIXELS.x + (position_relative_to_camera.x * TILE_SIDE_IN_PIXELS)
		- (TILE_SIDE_IN_PIXELS / 2);
	r32 y = SCREEN_CENTER_IN_PIXELS.y + (position_relative_to_camera.y * TILE_SIDE_IN_PIXELS)
		- (TILE_SIDE_IN_PIXELS / 2);

	rect result = get_rect_from_min_corner(x, y, w, h);
	return result;
}

void render_map_layer(render_group* render, level_state* level, map_layer layer, tile_position camera_tile_pos, v2 camera_offset_in_tile)
{
	if (layer.tiles_count > 0)
	{
		i32 screen_half_width = ceil(HALF_SCREEN_WIDTH_IN_TILES) + 2;
		i32 screen_half_height = ceil(HALF_SCREEN_HEIGHT_IN_TILES) + 2;

		for (i32 y_coord_relative = -screen_half_height;
			y_coord_relative < screen_half_height;
			y_coord_relative++)
		{
			i32 y_coord_on_screen = y_coord_relative;
			i32 y_coord_in_world = camera_tile_pos.y + y_coord_relative;

			for (i32 x_coord_relative = -screen_half_width;
				x_coord_relative < screen_half_width;
				x_coord_relative++)
			{
				i32 x_coord_on_screen = x_coord_relative;
				i32 x_coord_in_world = camera_tile_pos.x + x_coord_relative;

				u32 tile_value = get_tile_value(layer, x_coord_in_world, y_coord_in_world);
				if (tile_value != 0 && tile_value != 1) // przezroczyste pola
				{
					rect tile_bitmap = get_tile_bitmap_rect(tile_value);

					v2 position = get_v2(x_coord_on_screen, y_coord_on_screen) - camera_offset_in_tile;
					rect screen_rect = get_tile_screen_rect(position);
					render_bitmap(render, textures::TILESET, tile_bitmap, screen_rect);
				}
			}
		}
	}
}

void render_entity_sprite(render_group* render, world_position camera_position, world_position entity_position, direction entity_direction,
	sprite_effect* visual_effect, r32 visual_effect_duration, sprite sprite)
{
	b32 tint_modified = false;
	v4 tint = get_zero_v4();

	if (visual_effect)
	{
		tint_modified = true;
		tint = get_tint(visual_effect, visual_effect_duration);
	}

	for (u32 part_index = 0; part_index < sprite.parts_count; part_index++)
	{
		sprite_part* part = &sprite.parts[part_index];
		v2 offset = part->offset_in_pixels;

		b32 flip = false;
		if (part->default_direction != direction::NONE && entity_direction != part->default_direction)
		{
			flip = !flip;
			offset = reflection_over_y_axis(part->offset_in_pixels);
		}

		if (sprite.flip_horizontally)
		{
			flip = !flip;
			offset = reflection_over_y_axis(part->offset_in_pixels);
		}

		v2 position = get_position_difference(entity_position, camera_position);
		v2 render_rect_dim = get_rect_dimensions(part->texture_rect);
		rect screen_rect = get_screen_rect(position, render_rect_dim);
		screen_rect = move_rect(screen_rect, offset);

		if (tint_modified)
		{
			assert(tint.r >= 0 && tint.r <= 1 && tint.g >= 0 && tint.g <= 1 && tint.b >= 0 && tint.b <= 1);
			if (are_flags_set(&visual_effect->flags, sprite_effect_flags::ADDITIVE_MODE))
			{
				v4 no_tint = get_v4(1.0f, 1.0f, 1.0f, 1.0f);
				render_bitmap_with_effects(render, part->texture, part->texture_rect, screen_rect, no_tint, false, flip);
				render_bitmap_with_effects(render, part->texture, part->texture_rect, screen_rect, tint, true, flip);
			}
			else
			{
				render_bitmap_with_effects(render, part->texture, part->texture_rect, screen_rect, tint, false, flip);
			}
		}
		else
		{
			render_bitmap_with_effects(render, part->texture, part->texture_rect, screen_rect, get_zero_v4(), false, flip);
		}
	}
}