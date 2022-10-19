#pragma once

#include "main.h"

struct render_text_options
{
    font font;
    rect writing_area;
    b32 wrap;
    b32 add_tint;
    v4 text_tint;
};

struct text_area_limits
{
    u32 max_lines_count;
    u32 max_line_length;
    u32 max_character_count;
};

text_area_limits get_text_area_limits(font font, rect writing_area);
v2 get_text_area_for_line_of_text(font font, u32 character_count);
void render_text(render_group* render, memory_arena* transient_arena, render_text_options options, string_ref text);
void render_text(render_group* render, memory_arena* transient_arena, render_text_options options, const char* buffer, u32 buffer_size);
void render_text(render_group* render, memory_arena* transient_arena, font font, rect writing_area, string_ref text, b32 wrap = true);
void render_text(render_group* render, memory_arena* transient_arena, font font, rect writing_area, const char* buffer, u32 buffer_size, b32 wrap = true);