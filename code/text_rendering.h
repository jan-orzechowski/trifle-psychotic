#pragma once

#include "main.h"

struct text_area_limits
{
    u32 max_lines_count;
    u32 max_line_length;
    u32 max_character_count;
};

text_area_limits get_text_area_limits(font font, rect writing_area);
void render_text(render_group* render, memory_arena* transient_arena, font font, rect writing_area, string_ref text, b32 wrap = true);
void render_text(render_group* render, memory_arena* transient_arena, font font, rect writing_area, const char* buffer, u32 buffer_size, b32 wrap = true);