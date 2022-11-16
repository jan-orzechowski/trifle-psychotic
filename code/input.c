#include "main.h"
#include "input.h"

input_buffer initialize_input_buffer(memory_arena* arena)
{
    input_buffer result = {0};
    result.size = 60 * 2; // 2 sekundy
    result.buffer = push_array(arena, result.size, game_input);
    return result;
}

void write_to_input_buffer(input_buffer* buffer, game_input* new_input)
{
    buffer->buffer[buffer->current_index] = *new_input;
    buffer->current_index++;
    if (buffer->current_index == buffer->size)
    {
        buffer->current_index = 0;
    }
}

game_input* get_past_input(input_buffer* buffer, u32 how_many_frames_backwards)
{
    i32 input_index = buffer->current_index - 1 - how_many_frames_backwards;
    while (input_index < 0)
    {
        input_index = buffer->size + input_index;
    }
    game_input* result = &buffer->buffer[input_index];
    return result;
}

game_input* get_last_frame_input(input_buffer* buffer)
{
    game_input* result = get_past_input(buffer, 0);
    return result;
}

// żeby działało na dowolnych przyciskach, trzeba dodać nowy enum
b32 was_up_key_pressed_in_last_frames(input_buffer* buffer, u32 number_of_frames)
{
    b32 result = false;
    for (u32 frame = 1; frame <= number_of_frames; frame++)
    {
        game_input* input = get_past_input(buffer, frame);
        if (input->up.number_of_presses > 0)
        {
            result = true;
            break;
        }
    }
    return result;
}

b32 was_any_key_pressed_in_last_frames(input_buffer* buffer, u32 number_of_frames)
{
    b32 result = false;
    for (u32 frame = 1; frame <= number_of_frames; frame++)
    {
        game_input* input = get_past_input(buffer, frame - 1);
        if (input->is_fire_button_held
            || input->up.number_of_presses > 0
            || input->down.number_of_presses > 0
            || input->left.number_of_presses > 0
            || input->right.number_of_presses > 0)
        {
            result = true;
            break;
        }
    }
    return result;
}

void circular_buffer_test(memory_arena* arena)
{
    temporary_memory test_memory = begin_temporary_memory(arena);

    i32 test_input_count = 200;

    input_buffer* input_buf = push_struct(test_memory.arena, input_buffer);
    input_buf->size = 100;
    input_buf->buffer = push_array(test_memory.arena, input_buf->size, game_input);

    for (i32 input_index = 0; input_index < test_input_count; input_index++)
    {
        game_input* new_test_input = push_struct(test_memory.arena, game_input);
        new_test_input->up.number_of_presses = input_index;
        write_to_input_buffer(input_buf, new_test_input);
    }

    game_input* test_input = 0;
    test_input = get_past_input(input_buf, 0);
    assert(test_input->up.number_of_presses == test_input_count - 1);

    test_input = get_past_input(input_buf, input_buf->size);
    assert(test_input->up.number_of_presses == test_input_count - 1);

    test_input = get_past_input(input_buf, 1);
    assert(test_input->up.number_of_presses == test_input_count - 2);

    end_temporary_memory(test_memory, true);
}