#pragma once

#include "main.h"

input_buffer initialize_input_buffer(memory_arena* arena);
void write_to_input_buffer(input_buffer* buffer, game_input* new_input);
game_input* get_past_input(input_buffer* buffer, u32 how_many_frames_backwards);
game_input* get_last_frame_input(input_buffer* buffer);
b32 was_up_key_pressed_in_last_frames(input_buffer* buffer, u32 number_of_frames);