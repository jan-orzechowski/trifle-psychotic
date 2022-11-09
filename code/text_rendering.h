#pragma once

#include "main.h"

typedef struct text_area_limits
{
    u32 max_lines_count;
    u32 max_line_length;
    u32 max_character_count;
} text_area_limits;

typedef struct text_viewport
{
    rect cropped_writing_area;
    text_lines text_lines;
    u32 first_line_to_render_index;
    u32 last_line_to_render_index;
} text_viewport;

text_area_limits get_text_area_limits(render_text_options* options);
v2 get_text_area_for_single_line(font font, string_ref text_line);
text_lines* get_division_of_text_into_lines(memory_arena* arena, render_text_options* options, string_ref text);

void render_text(render_list* render, memory_arena* transient_arena, render_text_options* options, string_ref text);
void render_text_basic(render_list* render, memory_arena* transient_arena, font font, rect writing_area, string_ref text, b32 wrap);
void render_large_text(render_list* render, render_text_options* options, text_lines text_lines, r32 y_offset);