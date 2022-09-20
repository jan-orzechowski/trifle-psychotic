#pragma once

#include "main.h"

struct font
{
    u32 pixel_width;
    u32 pixel_height;
    u32 offset;
};

void render_text(memory_arena* transient_arena, sdl_game_data* sdl_game, font font, rect area, string_ref text, b32 wrap = true);
void render_text(memory_arena* transient_arena, sdl_game_data* sdl_game, font font, rect area, char* buffer, u32 buffer_size, b32 wrap = true);