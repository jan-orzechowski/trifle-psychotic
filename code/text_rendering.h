#pragma once

#include "main.h"

void render_text(render_group* render, memory_arena* transient_arena, font font, rect writing_area, string_ref text, b32 wrap = true);
void render_text(render_group* render, memory_arena* transient_arena, font font, rect writing_area, const char* buffer, u32 buffer_size, b32 wrap = true);