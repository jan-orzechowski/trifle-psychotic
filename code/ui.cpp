#include "main.h"
#include "rendering.h"
#include "text_rendering.h"
#include "ui.h"

rect get_whole_screen_text_area(r32 margin)
{
	r32 window_border = 4.0f * 2;
	rect result = get_rect_from_corners(
		get_v2(window_border + margin, window_border + margin),
		get_v2((SCREEN_WIDTH  - window_border - margin) / SCALING_FACTOR,
			   (SCREEN_HEIGHT - window_border - margin) / SCALING_FACTOR));
	return result;
}

void render_hitpoint_bar(static_game_data* static_data, render_group* render, entity* player, b32 draw_white_bars)
{
	r32 health = player->health;
	if (health < 0.0f)
	{
		health = 0.0f;
	}

	u32 filled_health_bars = (u32)(health / 10);
	u32 max_health_bars = (u32)(player->type->max_health / 10);

	r32 box_width = 16 + (max_health_bars * 4);
	r32 box_height = 8;
	rect box = get_rect_from_min_corner(get_v2(10, 10), get_v2(box_width, box_height));
	render_ui_box(static_data, render, box);

	rect icon_screen_rect = get_rect_from_min_corner(get_v2(10, 10), get_v2(10, 10));
	rect bar_screen_rect = get_rect_from_min_corner(get_v2(18, 10), get_v2(4, 8));

	rect bar_texture_rect = static_data->ui_gfx.healthbar_red_bar;
	if (draw_white_bars)
	{
		bar_texture_rect = static_data->ui_gfx.healthbar_white_bar;
	}

	render_bitmap(render, textures::CHARSET, static_data->ui_gfx.healthbar_icon, icon_screen_rect);

	for (u32 health_bar_index = 0;
		health_bar_index < filled_health_bars;
		health_bar_index++)
	{
		bar_screen_rect = move_rect(bar_screen_rect, get_v2(4, 0));
		render_bitmap(render, textures::CHARSET, bar_texture_rect, bar_screen_rect);
	}

	bar_texture_rect = static_data->ui_gfx.healthbar_empty_bar;
	for (u32 health_bar_index = filled_health_bars;
		health_bar_index < max_health_bars;
		health_bar_index++)
	{
		bar_screen_rect = move_rect(bar_screen_rect, get_v2(4, 0));
		render_bitmap(render, textures::CHARSET, bar_texture_rect, bar_screen_rect);
	}
}

void render_counter(static_game_data* static_data, render_group* render, memory_arena* transient_arena,
	i32 counter, u32 counter_max_value)
{
	if (counter > counter_max_value)
	{
		counter = counter_max_value;
	}

	if (counter < 0)
	{
		counter = 0;
	}
	
	u32 digits_count = how_many_digits(counter_max_value);
	v2 box_size = get_text_area_for_line_of_text(static_data->ui_font, digits_count);
	v2 box_position = get_v2((SCREEN_WIDTH / SCALING_FACTOR) - box_size.x - 8, 12);
	rect box = get_rect_from_min_corner(box_position, box_size);
	
	char buffer[10];
	snprintf(buffer, 10, "%d", counter);

	render_ui_box(static_data, render, add_side_length(box, get_v2(4, 4)));
	render_text(render, transient_arena, static_data->ui_font, box, buffer, 10, false);
}

void render_menu_option(font font, game_state* game, u32 x_coord, u32 y_coord, string_ref title)
{
	rect textbox_area = get_rect_from_corners(
		get_v2(x_coord, y_coord),
		get_v2(x_coord + 100, y_coord + 20));

	render_text(&game->render, game->transient_arena, font, textbox_area, title);
}

void render_ui_box(static_game_data* static_data, render_group* group, rect textbox_rect)
{
	v2 dimensions = get_rect_dimensions(textbox_rect);

	v4 background_color = get_v4(255, 255, 255, 255);
	render_rectangle(group, textbox_rect, background_color, false);

	v2 tile_dimensions = get_v2(4, 4);
	u32 tile_x_count = ceil(dimensions.x / tile_dimensions.x);
	u32 tile_y_count = ceil(dimensions.y / tile_dimensions.y);

	rect first_destination_rect = get_rect_from_min_corner(
		textbox_rect.min_corner - tile_dimensions,
		tile_dimensions);
	rect destination_rect = first_destination_rect;

	for (u32 y = 0; y <= tile_y_count + 1; y++)
	{
		destination_rect = move_rect(first_destination_rect, get_v2(0, y * tile_dimensions.y));

		if (y == 0)
		{
			for (u32 x = 0; x <= tile_x_count; x++)
			{
				rect source_bitmap;
				if (x == 0)
				{
					source_bitmap = static_data->ui_gfx.msgbox_frame_upper_left;
				}
				else if (x == tile_x_count)
				{
					source_bitmap = static_data->ui_gfx.msgbox_frame_upper_right;
				}
				else
				{
					source_bitmap = static_data->ui_gfx.msgbox_frame_upper;
				}

				render_bitmap(group, textures::CHARSET, source_bitmap, destination_rect);
				move_rect(&destination_rect, get_v2(tile_dimensions.x, 0));
			}
		}
		else if (y == tile_y_count + 1)
		{
			for (u32 x = 0; x <= tile_x_count; x++)
			{
				rect source_bitmap;
				if (x == 0)
				{
					source_bitmap = static_data->ui_gfx.msgbox_frame_lower_left;
				}
				else if (x == tile_x_count)
				{
					source_bitmap = static_data->ui_gfx.msgbox_frame_lower_right;
				}
				else
				{
					source_bitmap = static_data->ui_gfx.msgbox_frame_lower;
				}

				render_bitmap(group, textures::CHARSET, source_bitmap, destination_rect);
				move_rect(&destination_rect, get_v2(tile_dimensions.x, 0));
			}
		}
		else
		{
			render_bitmap(group, textures::CHARSET, static_data->ui_gfx.msgbox_frame_left, destination_rect);

			destination_rect = move_rect(destination_rect, get_v2(tile_x_count * tile_dimensions.x, 0));
			render_bitmap(group, textures::CHARSET, static_data->ui_gfx.msgbox_frame_right, destination_rect);
		}
	}
}