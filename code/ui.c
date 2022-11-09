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

b32 was_rect_clicked(game_input* input, rect screen_rect)
{
    b32 result = false;
    if (input->is_left_mouse_key_held)
    {
        v2 relative_mouse_pos = scalar_divide_v2(get_v2(input->mouse_x, input->mouse_y), SCALING_FACTOR);
        result = is_point_inside_rect(screen_rect, relative_mouse_pos);
    }
    return result;
}

void set_rect_length_to_fit_text(rect* option_rect, font font, string_ref text)
{
    r32 new_width = get_text_area_for_single_line(font, text).x;
    option_rect->max_corner.x = option_rect->min_corner.x + new_width;
}

void render_hitpoint_bar(static_game_data* static_data, render_list* render, entity* player, b32 draw_white_bars)
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

    render_bitmap(render, TEXTURE_CHARSET, static_data->ui_gfx.healthbar_icon, icon_screen_rect);

    for (u32 health_bar_index = 0;
        health_bar_index < filled_health_bars;
        health_bar_index++)
    {
        bar_screen_rect = move_rect(bar_screen_rect, get_v2(4, 0));
        render_bitmap(render, TEXTURE_CHARSET, bar_texture_rect, bar_screen_rect);
    }

    bar_texture_rect = static_data->ui_gfx.healthbar_empty_bar;
    for (u32 health_bar_index = filled_health_bars;
        health_bar_index < max_health_bars;
        health_bar_index++)
    {
        bar_screen_rect = move_rect(bar_screen_rect, get_v2(4, 0));
        render_bitmap(render, TEXTURE_CHARSET, bar_texture_rect, bar_screen_rect);
    }
}

void render_crosshair(static_game_data* static_data, render_list* render, game_input* input)
{
    v2 relative_mouse_pos = scalar_divide_v2(get_v2(input->mouse_x, input->mouse_y), SCALING_FACTOR);
    rect screen_rect = get_rect_from_center_and_dimensions(relative_mouse_pos, get_v2(13, 13));
    render_bitmap(render, TEXTURE_CHARSET, static_data->ui_gfx.crosshair, screen_rect);
}

void render_counter(static_game_data* static_data, render_list* render, memory_arena* transient_arena,
    i32 counter, i32 counter_max_value)
{
    if (counter > counter_max_value)
    {
        counter = counter_max_value;
    }

    if (counter < 0)
    {
        counter = 0;
    }
    
    char buffer[10];	
    snprintf(buffer, 10, "%d", counter);

    string_ref counter_value_str = {0};
    counter_value_str.ptr = buffer;
    counter_value_str.string_size = how_many_digits(counter_max_value);
    
    v2 box_size = get_text_area_for_single_line(static_data->ui_font, counter_value_str);
    v2 box_position = get_v2((SCREEN_WIDTH / SCALING_FACTOR) - box_size.x - 12, 12);
    rect text_area = get_rect_from_min_corner(box_position, box_size);
    rect box_area = add_side_length(text_area, get_v2(4, 4));
    box_area = move_rect(box_area, get_v2(2, 0));

    render_ui_box(static_data, render, box_area);
    render_text_basic(render, transient_arena, static_data->ui_font, text_area, counter_value_str, false);
}

rect render_menu_option(font font, game_state* game, rect text_area, string_ref title, b32 tint_as_completed)
{
#if TRIFLE_DEBUG
    render_bitmap(&game->render, TEXTURE_BACKGROUND_CLOUDS,
        get_rect_from_corners(get_zero_v2(), get_v2(100, 20)),
        text_area);
#endif

    if (tint_as_completed)
    {
        render_text_options options = {0};
        options.font = font;
        options.writing_area = text_area;
        options.allow_horizontal_overflow = false;
        options.add_tint = true;
        options.text_tint = get_v4(0.0f, 0.8f, 0.0f, 0.0f);
        render_text(&game->render, game->transient_arena, &options, title);
    }
    else
    {
        render_text_basic(&game->render, game->transient_arena, font, text_area, title, false);
    }

    return text_area;
}

rect render_menu_option_at_coords(font font, game_state* game, u32 x_coord, u32 y_coord, string_ref caption, b32 tint_as_completed)
{
    rect textbox_area = get_rect_from_corners(
        get_v2(x_coord, y_coord),
        get_v2(x_coord + 70, y_coord + 10));

    set_rect_length_to_fit_text(&textbox_area, font, caption);

    render_menu_option(font, game, textbox_area, caption, tint_as_completed);

    return textbox_area;
}

void render_ui_box(static_game_data* static_data, render_list* render, rect textbox_rect)
{
    v2 dimensions = get_rect_dimensions(textbox_rect);

    v4 background_color = get_v4(255, 255, 255, 255);
    render_rectangle(render, textbox_rect, background_color, false);

    v2 tile_dimensions = get_v2(4, 4);
    u32 tile_x_count = ceil(dimensions.x / tile_dimensions.x);
    u32 tile_y_count = ceil(dimensions.y / tile_dimensions.y);

    rect first_destination_rect = get_rect_from_min_corner(
        subtract_v2(textbox_rect.min_corner, tile_dimensions),
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

                render_bitmap(render, TEXTURE_CHARSET, source_bitmap, destination_rect);
                move_rect_in_place(&destination_rect, get_v2(tile_dimensions.x, 0));
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

                render_bitmap(render, TEXTURE_CHARSET, source_bitmap, destination_rect);
                move_rect_in_place(&destination_rect, get_v2(tile_dimensions.x, 0));
            }
        }
        else
        {
            render_bitmap(render, TEXTURE_CHARSET, static_data->ui_gfx.msgbox_frame_left, destination_rect);

            destination_rect = move_rect(destination_rect, get_v2(tile_x_count * tile_dimensions.x, 0));
            render_bitmap(render, TEXTURE_CHARSET, static_data->ui_gfx.msgbox_frame_right, destination_rect);
        }
    }
}

void update_and_render_skippable_indicator(render_list* render, static_game_data* static_data,
    r32* message_dots_timer, i32* message_dots_index, r32 delta_time, v2 indicator_position)
{
    *message_dots_timer += delta_time;
    if (*message_dots_timer > 0.4f)
    {
        *message_dots_timer = 0.0f;
        (*message_dots_index)++;
        if (*message_dots_index > 2)
        {
            *message_dots_index = 0;
        }
    }

    rect dots_indicator_rect = get_rect_from_center_and_dimensions(indicator_position, get_v2(15.0f, 5.0f));
    switch (*message_dots_index)
    {
        case 0: render_bitmap(render, TEXTURE_CHARSET,
            static_data->ui_gfx.msgbox_dots_1, dots_indicator_rect);
            break;
        case 1: render_bitmap(render, TEXTURE_CHARSET,
            static_data->ui_gfx.msgbox_dots_2, dots_indicator_rect);
            break;
        case 2: render_bitmap(render, TEXTURE_CHARSET,
            static_data->ui_gfx.msgbox_dots_3, dots_indicator_rect);
            break;
    }
}

void update_and_render_message_box(render_list* render, level_state* level, memory_arena* transient_arena, r32 delta_time)
{
    if (level->show_message && level->message_to_show.string_size)
    {
        if (is_zero_v2(level->messagebox_dimensions))
        {
            level->messagebox_dimensions = get_v2(150, 120);
        }

        rect text_area = get_rect_from_center_and_dimensions(
            SCREEN_CENTER_IN_PIXELS, level->messagebox_dimensions);
        v2 margin = get_v2(8, 8);
        
        render_ui_box(level->static_data, render, add_side_length(text_area, margin));

        render_text_basic(render, transient_arena, level->static_data->ui_font,
            text_area, level->message_to_show, true);

        if (level->min_message_timer <= 0.0f)
        {
            v2 dots_indicator_position = get_v2(SCREEN_CENTER_IN_PIXELS.x, text_area.max_corner.y - 4.5f);
            update_and_render_skippable_indicator(render, level->static_data,
                &level->message_skippable_indicator_timer,
                &level->message_skippable_indicator_index,
                delta_time, dots_indicator_position);
        }
    }
}

void render_victory_text(render_list* render, memory_arena* transient_arena, static_game_data* static_data)
{
    rect text_area = get_rect_from_center_and_dimensions(
        get_v2(SCREEN_CENTER_IN_PIXELS.x, 50.0f),
        get_v2(150.0f, 20.0f));

    render_text_basic(render, transient_arena, static_data->title_font,
        text_area, static_data->victory_str, false);
}