#pragma once

rect get_whole_screen_text_area(r32 margin);
void render_hitpoint_bar(static_game_data* static_data, render_group* render, entity* player, b32 draw_white_bars);
void render_counter(static_game_data* static_data, render_group* render, memory_arena* transient_arena, i32 counter, u32 counter_max_value);
void render_menu_option(font font, game_state* game, u32 x_coord, u32 y_coord, string_ref title);
void render_ui_box(static_game_data* static_data, render_group* group, rect textbox_rect);