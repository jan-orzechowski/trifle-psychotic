#pragma once

#include "main.h"

struct font
{
    u32 pixel_width;
    u32 pixel_height;
    u32 offset;
};

void render_text(render_group* render, memory_arena* transient_arena, font font, rect writing_area, string_ref text, b32 wrap = true);
void render_text(render_group* render, memory_arena* transient_arena, font font, rect writing_area, char* buffer, u32 buffer_size, b32 wrap = true);