#include "main.h"
#include "rendering.h"
#include "text_rendering.h"

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
	// zabezpieczenie na uint wrapping
	if (player->health < 0.0f)
	{
		player->health = 0.0f;
	}

	u32 filled_health_bars = (u32)(player->health / 10);
	u32 max_health_bars = (u32)(player->type->max_health / 10);

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

void render_menu_option(font font, game_state* game, u32 x_coord, u32 y_coord, string_ref title)
{
	rect textbox_area = get_rect_from_corners(
		get_v2(x_coord, y_coord),
		get_v2(x_coord + 100, y_coord + 20));

	render_text(&game->render, game->transient_arena, font, textbox_area, title);
}
