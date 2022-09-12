#pragma once

#include "main.h"

struct font
{
    u32 pixel_width;
    u32 pixel_height;
    u32 offset;
};

void write(memory_arena* temp_arena, sdl_game_data* sdl_game, font font, rect area, string_ref text);