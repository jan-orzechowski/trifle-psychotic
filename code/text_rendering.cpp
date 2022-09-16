#include "main.h"
#include "text_rendering.h"

struct lines_to_render
{
    string_ref* lines;
    u32 max_lines_count;
    u32 used_lines_count;
};

b32 is_letter(char letter)
{
    u32 code = (u32)letter;
    b32 result = (code >= 33 && code <= 126);
    return result;
}

string_ref peek_next_word(string_ref text, u32 current_index_in_text)
{
    assert(current_index_in_text < text.string_size);
    string_ref result = {};
    for (u32 word_char_index = current_index_in_text;
        word_char_index < text.string_size;
        word_char_index++)
    {
        char word_char = *(text.ptr + word_char_index);
        if (is_letter(word_char))
        {
            if (result.ptr)
            {
                result.string_size++;
            }
            else
            {
                result.ptr = text.ptr + word_char_index;
                result.string_size = 1;
            }
        }
        else
        {
            break;
        }
    }
    return result;
}

SDL_Rect get_glyph_rect(font font, u32 code)
{
    assert(is_letter(code));
    u32 index = code - 33;
    SDL_Rect rect = {};
    rect.w = font.pixel_width;
    rect.h = font.pixel_height;
    rect.x = 1 + (index * (font.pixel_width + 1));
    rect.y = 1;
    return rect;
}

void render_text(sdl_game_data* sdl_game, font font, rect area, lines_to_render lines)
{
    u32 x = 0;
    u32 y = 0;

    i32 letter_spacing = -1;
    i32 line_spacing = 4;

    y = area.min_corner.y;
    for (u32 line_index = 0; line_index < lines.used_lines_count; line_index++)
    {
        string_ref line = lines.lines[line_index];
        
        x = area.min_corner.x;
        for (u32 char_index = 0;
            char_index < line.string_size;
            char_index++)
        {
            char char_to_render = *(line.ptr + char_index);
            if (is_letter(char_to_render))
            {
                SDL_Rect src_rect = get_glyph_rect(font, (u32)char_to_render);
                SDL_Rect dst_rect = {};
                dst_rect.w = font.pixel_width;
                dst_rect.h = font.pixel_height;
                dst_rect.x = x;
                dst_rect.y = y;
                SDL_RenderCopy(sdl_game->renderer, sdl_game->font_texture, &src_rect, &dst_rect);
            }
            x += font.pixel_width + letter_spacing;
        }

        y += font.pixel_height + line_spacing;
    }
}

string_ref* get_next_line(lines_to_render* lines)
{
    string_ref* result = NULL;
    if (lines->used_lines_count < lines->max_lines_count)
    {
        result = &lines->lines[lines->used_lines_count];
        lines->used_lines_count++;
    }
    return result;
}

void write(memory_arena* transient_arena, sdl_game_data* sdl_game, font font, rect area, string_ref text)
{
    u32 max_text_length = 1000;
    if (text.string_size > max_text_length)
    {
        text.string_size = max_text_length;
    }

    temporary_memory writing_memory = begin_temporary_memory(transient_arena);

    v2 area_dim = get_rect_dimensions(area);
    u32 max_line_length = area_dim.x / font.pixel_width;
    
    lines_to_render text_lines = {};
    text_lines.max_lines_count = (area_dim.y / font.pixel_height);
    text_lines.lines = push_array(transient_arena, text_lines.max_lines_count, string_ref);

    string_ref* current_line = get_next_line(&text_lines);    
    current_line->ptr = text.ptr;
    current_line->string_size = 0;

    u32 char_index = 0;
    while (char_index < text.string_size) 
    {
        char letter = *(text.ptr + char_index);
        string_ref next_word = peek_next_word(text, char_index);
        if (next_word.string_size)
        {
            // mamy słowo            
            if (next_word.string_size > (max_line_length - current_line->string_size))
            {            
                // nowa linia
                current_line = get_next_line(&text_lines);
                if (current_line)
                {
                    current_line->ptr = next_word.ptr;
                    current_line->string_size = next_word.string_size;              
                }
                else
                {
                    // skończyło nam się miejsce - nie czytamy dalej
                    break;
                }
            }
            else
            {
                current_line->string_size += next_word.string_size;
            }
            char_index += next_word.string_size;
        }
        else
        {
            // spacja
            if (max_line_length - current_line->string_size < 1)
            {
                // koniec linii - pomijamy
            }
            else
            {
                current_line->string_size++;
            }   
            char_index++;
        }
    }

    render_text(sdl_game, font, area, text_lines);

    end_temporary_memory(writing_memory);
}