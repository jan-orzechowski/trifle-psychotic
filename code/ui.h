#pragma once

rect get_whole_screen_text_area(r32 margin);
b32 was_rect_clicked(game_input* input, rect screen_rect);
void set_rect_length_to_fit_text(rect* option_rect, font font, string_ref text);
void render_hitpoint_bar(static_game_data* static_data, render_list* render, entity* player, b32 draw_white_bars);
void render_crosshair(static_game_data* static_data, render_list* render, game_input* input);
void render_counter(static_game_data* static_data, render_list* render, memory_arena* transient_arena, i32 counter, i32 counter_max_value);
rect render_menu_option(font font, game_state* game, rect text_area, string_ref title, b32 tint_completed = false);
rect render_menu_option(font font, game_state* game, u32 x_coord, u32 y_coord, string_ref caption, b32 tint_completed = false);
void render_ui_box(static_game_data* static_data, render_list* render, rect textbox_rect);
void update_and_render_skippable_indicator(render_list* render, static_game_data* static_data,
    r32* message_dots_timer, i32* message_dots_index, r32 delta_time, v2 indicator_position);
void update_and_render_message_box(render_list* render, level_state* level, memory_arena* transient_arena, r32 delta_time);
void render_victory_text(render_list* render, memory_arena* transient_arena, static_game_data* static_data);