#include "main.h"
#include "text_rendering.h"
#include "rendering.h"

#define MAX_LINES_COUNT 2000

string_ref peek_next_word(string_ref text, u32 current_index_in_text)
{
    assert(current_index_in_text < text.string_size);
    string_ref result = {0};
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

string_ref* get_next_line(memory_arena* arena, text_lines* lines, u32 max_lines_count)
{
    string_ref* result = NULL;
    if (lines->lines_count < max_lines_count)
    {
        result = push_struct(arena, string_ref);
        lines->lines_count++;
    }
    return result;
}

text_area_limits get_text_area_limits(render_text_options* options)
{
    text_area_limits result = {0};

    assert(options->font.width_in_pixels);
    assert(options->font.height_in_pixels);

    v2 area_dim = get_rect_dimensions(options->writing_area);
    assert(area_dim.x >= 0);
    assert(area_dim.y >= 0);

    if (options->allow_horizontal_overflow)
    {
        result.max_line_length = U32_MAX_VALUE;
        result.max_lines_count = 1;
        result.max_character_count = U32_MAX_VALUE;
    }
    else
    {
        r32 character_width = options->font.width_in_pixels + options->font.letter_spacing;
        r32 line_height = options->font.height_in_pixels + options->font.line_spacing;

        result.max_line_length = (u32)(area_dim.x / character_width);

        if (options->allow_vertical_overflow)
        {
            result.max_lines_count = MAX_LINES_COUNT;
            result.max_character_count = result.max_line_length * result.max_lines_count;
        }
        else
        {
            result.max_lines_count = (u32)(area_dim.y / line_height) + 1;
            result.max_character_count = result.max_line_length * result.max_lines_count;
        }
    }

    return result;
}

v2 get_text_area_for_single_line(font font, string_ref text_line)
{
    r32 width = text_line.string_size * (font.width_in_pixels + font.letter_spacing);
    r32 height = font.height_in_pixels;
    v2 result = get_v2(width, height);
    return result;
}

text_lines* get_division_of_text_into_lines(memory_arena* arena, render_text_options* options, string_ref text)
{
    text_lines* lines = push_struct(arena, text_lines);
    text_area_limits limits = get_text_area_limits(options);

    string_ref* current_line = get_next_line(arena, lines, limits.max_lines_count);
    current_line->ptr = text.ptr;
    current_line->string_size = 0;

    lines->lines = current_line;

    u32 char_index = 0;
    while (char_index < text.string_size)
    {
        char letter = *(text.ptr + char_index);
        string_ref next_word = peek_next_word(text, char_index);
        if (next_word.string_size)
        {
            // mamy słowo            
            if (next_word.string_size > (limits.max_line_length - current_line->string_size))
            {
                // nowa linia
                current_line = get_next_line(arena, lines, limits.max_lines_count);
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
            if (letter == '\n')
            {
                if (limits.max_line_length - current_line->string_size < 1)
                {
                    // koniec linii - pomijamy
                }
                else
                {
                    // nowa linia
                    char* last_char = current_line->ptr + current_line->string_size;

                    current_line = get_next_line(arena, lines, limits.max_lines_count);
                    if (current_line)
                    {
                        current_line->ptr = last_char + 1;
                        current_line->string_size = 0;
                    }
                    else
                    {
                        // skończyło nam się miejsce - nie czytamy dalej
                        break;
                    }
                }
            }
            else
            {
                // spacja
                if (limits.max_line_length - current_line->string_size < 1)
                {
                    // koniec linii - pomijamy
                }
                else
                {
                    current_line->string_size++;
                }
            }
            char_index++;
        }
    }

    return lines;
}

text_viewport get_text_viewport(font font, text_lines lines, rect writing_area)
{
    text_viewport result = {0};
    r32 line_height = font.height_in_pixels + font.line_spacing + 1;

    // przycinamy writing area
    {
        r32 total_text_height = lines.lines_count * line_height;
        r32 writing_area_height = writing_area.max_corner.y - writing_area.min_corner.y;
        r32 height_not_used = writing_area_height - total_text_height;
        writing_area.max_corner.y -= height_not_used;
    }

    u32 first_visible_line = 0;
    u32 last_visible_line = 0;

    if (writing_area.min_corner.y < 0)
    {
        r32 height_not_visible = -writing_area.min_corner.y;
        u32 invisible_lines_count = (u32)floor(height_not_visible / line_height);

        r32 height_not_used = invisible_lines_count * line_height;
        writing_area.min_corner.y += height_not_used;

        if (invisible_lines_count < lines.lines_count)
        {
            first_visible_line = invisible_lines_count;
        }
        else
        {
            first_visible_line = lines.lines_count;
        }
    }
    else
    {
        first_visible_line = 0;
    }

    if (writing_area.max_corner.y > (SCREEN_HEIGHT / SCALING_FACTOR))
    {
        r32 height_not_visible = writing_area.max_corner.y - (SCREEN_HEIGHT / SCALING_FACTOR);
        u32 invisible_lines_count = (u32)floor(height_not_visible / line_height);

        r32 height_not_used = invisible_lines_count * line_height;
        writing_area.max_corner.y -= height_not_used;

        if (invisible_lines_count > lines.lines_count)
        {
            last_visible_line = 0;
        }
        else
        {
            last_visible_line = lines.lines_count - invisible_lines_count;
        }
    }
    else
    {
        last_visible_line = lines.lines_count;
    }

    result.first_line_to_render_index = first_visible_line;
    result.last_line_to_render_index = last_visible_line;
    result.cropped_writing_area = writing_area;
    result.text_lines = lines;

    return result;
}

rect get_glyph_rect(font font, u32 code)
{
    assert(is_letter(code));
    u32 index = code - 33;

    if (font.uppercase_only)
    {
        //zamiana liter małych na duże
        if (index > 63)
        {
            index -= 32;
        }
    }

    v2 position = get_v2(
        font.glyph_spacing_width + (index * (font.width_in_pixels + font.glyph_spacing_width)),
        font.glyph_spacing_width);
    v2 dimensions = get_v2(font.width_in_pixels, font.height_in_pixels);
    rect result = get_rect_from_min_corner(position, dimensions);
    return result;
}

void render_text_line(render_list* render, render_text_options* options, string_ref line)
{
    font font = options->font;
    r32 x = options->writing_area.min_corner.x;
    r32 y = options->writing_area.min_corner.y;

    for (u32 char_index = 0;
        char_index < line.string_size;
        char_index++)
    {
        char char_to_render = *(line.ptr + char_index);
        if (is_letter(char_to_render))
        {
            rect src_rect = get_glyph_rect(font, (u32)char_to_render);
            rect dst_rect = get_rect_from_min_corner(get_v2(x, y), get_v2(font.width_in_pixels, font.height_in_pixels));
            if (options->add_tint)
            {
                render_bitmap_with_effects(render, font.texture, src_rect, dst_rect, options->text_tint, false, false);
            }
            else
            {
                render_bitmap(render, font.texture, src_rect, dst_rect);
            }
        }

        x += font.width_in_pixels + font.letter_spacing;
    }
}

void render_text_lines(render_list* render, render_text_options* options, text_viewport* viewport, text_lines* lines)
{    
    assert(viewport != NULL || lines != NULL);

    font font = options->font;
    
    if (viewport != NULL)
    {
        lines = &viewport->text_lines;
    }

    u32 first_line = 0;
    u32 last_line = lines->lines_count;
    r32 first_x = options->writing_area.min_corner.x;
    r32 first_y = options->writing_area.min_corner.y;
    r32 x = 0;
    r32 y = 0;

    if (viewport != NULL)
    {
        first_x = viewport->cropped_writing_area.min_corner.x;
        first_y = viewport->cropped_writing_area.min_corner.y;
        first_line = viewport->first_line_to_render_index;
        last_line = viewport->last_line_to_render_index;
    }

    y = first_y;
    for (u32 line_index = first_line;
        line_index < last_line;
        line_index++)
    {
        string_ref line = lines->lines[line_index];
        
        x = first_x;
        for (u32 char_index = 0;
            char_index < line.string_size;
            char_index++)
        {
            char char_to_render = *(line.ptr + char_index);
            if (is_letter(char_to_render))
            {
                rect src_rect = get_glyph_rect(font, (u32)char_to_render);
                rect dst_rect = get_rect_from_min_corner(get_v2(x, y), get_v2(font.width_in_pixels, font.height_in_pixels));
                if (options->add_tint)
                {
                    render_bitmap_with_effects(render, font.texture, src_rect, dst_rect, options->text_tint, false, false);
                }
                else
                {
                    render_bitmap(render, font.texture, src_rect, dst_rect);
                }
            }

            x += font.width_in_pixels + font.letter_spacing;
        }

        y += font.height_in_pixels + font.line_spacing;
    }
}

void render_text(render_list* render, memory_arena* transient_arena, render_text_options* options, string_ref text)
{
    temporary_memory writing_memory = begin_temporary_memory(transient_arena);
    
    text_area_limits limits = get_text_area_limits(options);
    if (text.string_size > limits.max_character_count)
    {
        text.string_size = limits.max_character_count;
    }

    if (options->allow_horizontal_overflow)
    {        
        string_ref line_to_render = {0};
        line_to_render.ptr = text.ptr;
        line_to_render.string_size = text.string_size;

        if (line_to_render.string_size > limits.max_line_length)
        {
            line_to_render.string_size = limits.max_line_length;
        }

        render_text_line(render, options, line_to_render);   
    }
    else
    {
        text_lines* text_lines = get_division_of_text_into_lines(transient_arena, options, text);
        render_text_lines(render, options, NULL, text_lines);
    }

    end_temporary_memory(writing_memory, true);
}

void render_text_basic(render_list* render, memory_arena* transient_arena, font font, rect writing_area, string_ref text, b32 wrap)
{
    render_text_options options = {0};
    options.font = font;
    options.writing_area = writing_area;
    options.allow_horizontal_overflow = (false == wrap);
    render_text(render, transient_arena, &options, text);
}

void render_large_text(render_list* render, render_text_options* options, text_lines text_lines, r32 y_offset)
{
    rect new_writing_area = move_rect(options->writing_area, get_v2(0.0f, y_offset));

    text_viewport viewport = get_text_viewport(options->font, text_lines, new_writing_area);
    render_text_lines(render, options, &viewport, NULL);
}