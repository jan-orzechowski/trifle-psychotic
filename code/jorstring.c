﻿#include "jorutils.h"
#include "jorstring.h"
#include "jormath.h"

string_ref get_string_ref(const char* str_ptr, u32 size)
{
    string_ref result = {0};
    result.ptr = (char*)str_ptr;
    result.string_size = size;
    return result;
}

string_ref get_string_ref_length_unknown(const char* str_ptr)
{
    string_ref result = {0};
    result.ptr = (char*)str_ptr;
    result.string_size = get_c_string_length(str_ptr);
    return result;
}

b32 is_empty_string(string_ref str)
{
    b32 result = (str.ptr == 0 || str.string_size == 0);
    return result;
}

b32 equals_string_ref(string_ref a, string_ref b)
{
    b32 result = true;

    if (is_empty_string(a) || is_empty_string(b))
    {
        invalid_code_path;
        result = false;
    }

    if (a.string_size != b.string_size)
    {
        result = false;
    }
    else
    {
        for (u32 char_index = 0;
            char_index < a.string_size;
            char_index++)
        {
            char a_char = *(a.ptr + char_index);
            char b_char = *(b.ptr + char_index);
            if (a_char != b_char)
            {
                result = false;
                break;
            }
        }
    }
    return result;
}

string_ref copy_string(memory_arena* arena, string_ref str)
{
    string_ref new_str = {0};
    new_str.string_size = str.string_size;
    new_str.ptr = (char*)push_size(arena, str.string_size);
    for (u32 char_index = 0; char_index < str.string_size; char_index++)
    {
        *(new_str.ptr + char_index) = *(str.ptr + char_index);
    }
    return new_str;
}

string_ref copy_string_to_buffer(char* buffer, u32 buffer_length, string_ref str)
{
    string_ref result = {0};
    result.ptr = buffer;
    if (str.string_size > buffer_length - 1)
    {
        result.string_size = buffer_length - 1;
    }
    else
    {
        result.string_size = str.string_size;
    }

    for (u32 char_index = 0; char_index < result.string_size; char_index++)
    {
        *(buffer + char_index) = *(str.ptr + char_index);
    }
    *(buffer + result.string_size) = '\0';

    return result;
}

string_ref get_string_from_buffer(char* buffer, u32 buffer_length)
{
    string_ref result = {0};
    result.ptr = buffer;
    result.string_size = get_c_string_length_with_limit(buffer, buffer_length);
    return result;
}

char* get_c_string(memory_arena* arena, string_ref str)
{
    char* new_c_str = (char*)push_size(arena, str.string_size + 1);
    {
        char* new_c_str_char = new_c_str;
        for (u32 char_index = 0; char_index < str.string_size; )
        {
            *new_c_str_char = *(str.ptr + char_index);
            new_c_str_char++;
            char_index++;
        }
        *(new_c_str + str.string_size) = '\0';
    }
    return new_c_str;
}

b32 compare_to_c_string(string_ref my_str, const char* c_str)
{
    b32 result = true;

    if (my_str.ptr == NULL || c_str == NULL)
    {
        result = false;
        invalid_code_path;
    }

    i32 my_str_char_index = 0;
    while (*c_str)
    {
        if (my_str_char_index == my_str.string_size)
        {
            // strings are of different length - we went through my_str, but not through c_str
            result = false;
            break;
        }

        char my_char = *(my_str.ptr + my_str_char_index);
        char c_char = *c_str;
        if (my_char != c_char)
        {
            result = false;
            break;
        }

        my_str_char_index++;
        c_str++;
    }

    if (my_str_char_index < my_str.string_size)
    {
        // strings are of different length - we went through c_str, but not through my_str
        result = false;
    }

    return result;
}

u32 get_c_string_length_with_limit(const char* str, u32 max_string_length)
{
    u32 result = 0;
    const char* str_temp = str;
    while (*str_temp && result <= max_string_length)
    {
        str_temp++;
        result++;
    }
    return result;
}

u32 get_c_string_length(const char* str)
{
    u32 result = 0;
    const char* str_temp = str;
    while (*str_temp)
    {
        str_temp++;
        result++;
    }
    return result;
}

string_ref copy_c_string_with_length(memory_arena* arena, const char* str, u32 str_size)
{
    string_ref result = {0};
    result.string_size = str_size;
    result.ptr = (char*)push_size(arena, str_size + 1);

    for (u32 index = 0;
        index < str_size;
        index++)
    {
        *(result.ptr + index) = *(str + index);
    }
    *(result.ptr + str_size) = '\0';

    return result;
}

string_ref copy_c_string(memory_arena* arena, const char* str)
{
    u32 c_str_length = get_c_string_length(str);
    string_ref result = copy_c_string_with_length(arena, str, c_str_length);
    return result;
}

string_ref copy_c_string_buffer(memory_arena* arena, const char* str, u32 buffer_size)
{
    u32 actual_str_size = 0;
    const char* str_temp = str;
    while (*str_temp && actual_str_size < buffer_size)
    {
        str_temp++;
        actual_str_size++;
    }

    string_ref result = {0};
    result.string_size = actual_str_size;
    result.ptr = (char*)push_size(arena, actual_str_size + 1);

    for (u32 index = 0;
        index < actual_str_size;
        index++)
    {
        *(result.ptr + index) = *(str + index);
    }
    *(result.ptr + actual_str_size) = '\0';

    return result;
}

b32 ends_with(string_ref str, const char* suffix)
{
    b32 result = false;	
    if (str.string_size > 0)
    {
        u32 suffix_length = get_c_string_length_with_limit(suffix, str.string_size);
        if (suffix_length <= str.string_size)
        {
            result = true;				
            for (i32 char_index_from_end = 0;
                char_index_from_end < suffix_length;
                char_index_from_end++)
            {
                char suffix_char = *(suffix + suffix_length - char_index_from_end - 1);
                char str_char = *(str.ptr + str.string_size - char_index_from_end - 1);
                if (suffix_char != str_char)
                {
                    result = false;
                }
            }
        }	
    }
    return result;
}

b32 is_whitespace(char c)
{
    b32 result = (c == ' ' || c == '\r' || c == '\t' || c == '\n');
    return result;
}

b32 is_digit(char c)
{
    b32 result = (c >= '0' && c <= '9');
    return result;
}

b32 is_letter(char c)
{
    u32 code = (u32)c;
    b32 result = (code >= 33 && code <= 126);
    return result;
}

b32 is_all_digits(string_ref str)
{
    b32 result = true;
    for (u32 char_index = 0;
        char_index < str.string_size;
        char_index++)
    {
        char c = *(str.ptr + char_index);
        if (false == is_digit(c))
        {
            result = false;
            break;
        }
    }
    return result;
}

b32 check_if_string_is_whitespace(string_ref str)
{
    b32 result = true;
    for (u32 char_index = 0;
        char_index < str.string_size;
        char_index++)
    {
        if (false == is_whitespace(*(str.ptr + char_index)))
        {
            result = false;
            break;
        }
    }

    return result;
}

string_ref omit_leading_whitespace(string_ref str)
{
    string_ref result = str;
    if (false == is_empty_string(str))
    {
        char* new_start = 0;
        for (u32 char_index = 0;
            char_index < str.string_size;
            char_index++)
        {
            char* c = (str.ptr + char_index);
            if (is_whitespace(*c))
            {
                new_start = c + 1;
            }
            else
            {
                break;
            }
        }

        if (new_start != 0)
        {
            if (new_start >= str.ptr + str.string_size)
            {
                result.ptr = 0;
                result.string_size = 0;
            }
            else
            {
                char* old_end = str.ptr + str.string_size - 1;
                u32 new_size = old_end - new_start;
                result.ptr = new_start;
                result.string_size = new_size;
            }
        }
    }
    return result;
}

string_ref omit_trailing_whitespace(string_ref str)
{
    string_ref result = str;
    if (false == is_empty_string(str))
    {
        char* new_end = 0;
        for (i32 char_index = str.string_size - 1;
            char_index >= 0;
            char_index--)
        {
            char* c = (str.ptr + char_index);
            if (is_whitespace(*c))
            {
                new_end = c;
            }
            else
            {
                break;
            }
        }

        if (new_end != 0)
        {
            if (new_end < str.ptr)
            {
                result.ptr = 0;
                result.string_size = 0;
            }
            else
            {
                u32 new_size = new_end - str.ptr;
                result.ptr = str.ptr;
                result.string_size = new_size;
            }
        }
    }
    return result;
}

i32 power(i32, u32);

i32 parse_i32(string_ref str)
{
    i32 result = 0;
    b32 negative = false;

    if (is_empty_string(str))
    {
        goto function_parse_i32_end;
    }

    str = omit_leading_whitespace(str);
    if (*str.ptr == '-')
    {
        negative = true;
        str.ptr++;
        str.string_size--;
    }
    str = omit_trailing_whitespace(str);
    if (false == is_empty_string(str) && is_all_digits(str))
    {
        i32 digits_count = str.string_size;
        if (*(str.ptr + str.string_size) == '\0')
        {
            digits_count--;
        }
        i32 decimal_place_multiplier = power(10, digits_count - 1);

        for (u32 char_index = 0; char_index < str.string_size; char_index++)
        {
            u8 digit = 0;
            char c = *(str.ptr + char_index);
            switch (c)
            {
                case '0': digit = 0; break;
                case '1': digit = 1; break;
                case '2': digit = 2; break;
                case '3': digit = 3; break;
                case '4': digit = 4; break;
                case '5': digit = 5; break;
                case '6': digit = 6; break;
                case '7': digit = 7; break;
                case '8': digit = 8; break;
                case '9': digit = 9; break;
                case '\0':
                default: goto function_parse_i32_end;
            }
            result += digit * decimal_place_multiplier;
            decimal_place_multiplier = decimal_place_multiplier / 10;
        }
    }

function_parse_i32_end:
    if (negative)
    {
        result = -result;
    }
    return result;
}

i32 parse_i32_from_range(char* start, char* end)
{
    i32 result = 0;
    string_ref str = {0};
    str.ptr = start;
    str.string_size = end - start;
    if (start && end && str.string_size > 0)
    {
        result = parse_i32(str);
    }
    return result;
}

u32 how_many_digits(u32 num)
{
    u32 result = 0;
    if (num < 10) { result = 1; }
    else if (num < 100) { result = 2; }
    else if (num < 1000) { result = 3; }
    else if (num < 10000) { result = 4; }
    else if (num < 100000) { result = 5; }
    else if (num < 1000000) { result = 6; }
    else if (num < 10000000) { result = 7; }
    else if (num < 100000000) { result = 8; }
    else if (num < 1000000000) { result = 9; }
    else { result = 10; };
    return result;
}

r32 parse_r32(string_ref str, char delimiter)
{
    r32 result = 0;
    char* delimiter_ptr = 0;
    b32 negative = false;

    str = omit_leading_whitespace(str);
    if (*str.ptr == '-')
    {
        negative = true;
        str.ptr++;
        str.string_size--;
    }
    str = omit_trailing_whitespace(str);

    if (false == is_empty_string(str))
    {
        for (u32 char_index = 0; char_index < str.string_size; char_index++)
        {
            char* c = (str.ptr + char_index);
            if (*c == delimiter)
            {
                delimiter_ptr = c;
                break;
            }
        }

        if (delimiter_ptr)
        {
            string_ref fraction_part = {0};
            fraction_part.ptr = delimiter_ptr + 1;
            fraction_part.string_size = (str.ptr + str.string_size) - fraction_part.ptr;
            i32 fraction = parse_i32(fraction_part);

            string_ref integer_part = {0};
            integer_part.ptr = str.ptr;
            integer_part.string_size = delimiter_ptr - str.ptr;
            i32 integer = parse_i32(integer_part);

            u32 fraction_digits_number = how_many_digits(fraction);
            result = integer + (r32)fraction / (r32)power(10, fraction_digits_number);
        }
    }

    if (delimiter_ptr == 0)
    {
        string_ref integer_part = str;
        i32 integer = parse_i32(integer_part);
        result = integer;
    }

    if (negative)
    {
        result = -result;
    }
    return result;
}

i32* parse_array_of_i32(memory_arena* arena, u32 array_length, string_ref str, char delimiter)
{
    i32* arr = push_array(arena, array_length, i32);
    u32 current_int_index = 0;
    char* start = 0;
    char* end = 0;
    for (u32 char_index = 0;
        char_index < str.string_size;
        char_index++)
    {
        char* c = str.ptr + char_index;
        if (*c == delimiter)
        {
            end = c;
            if (start)
            {
                i32 new_int = (i32)parse_i32_from_range(start, end);
                if (current_int_index < array_length)
                {
                    arr[current_int_index++] = new_int;
                    start = 0;
                    end = 0;
                }
                else
                {
                    // error: there is no more space, but string is not fully parsed
                    break;
                }
            }
            else
            {
                // the case when we have a double delimiter or the file started with a delimiter
                end = 0;
                continue;
            }
        }
        else if (is_digit(*c))
        {
            if (start == 0)
            {
                start = c;
            }
            else
            {
                continue;
            }
        }
        else if (is_whitespace(*c))
        {
            if (start != 0)
            {
                // we treat this as a delimiter
                end = c;
                i32 new_int = (i32)parse_i32_from_range(start, end);
                if (current_int_index < array_length)
                {
                    arr[current_int_index++] = new_int;
                    start = 0;
                    end = 0;
                }
                else
                {
                    // error: there is no more space, but string is not fully parsed
                    break;
                }
            }
            else
            {
                continue;
            }
        }
        else
        {
            // error: we found something that is neither a number nor delimiter
            continue;
        }
    }
    return arr;
}

u32 parse_hexadecimal_digit(char digit)
{
    u32 result = 0;
    switch (digit)
    {
        case '0': result = 0; break;
        case '1': result = 1; break;
        case '2': result = 2; break;
        case '3': result = 3; break;
        case '4': result = 4; break;
        case '5': result = 5; break;
        case '6': result = 6; break;
        case '7': result = 7; break;
        case '8': result = 8; break;
        case '9': result = 9; break;
        case 'a': case 'A': result = 10; break;
        case 'b': case 'B': result = 11; break;
        case 'c': case 'C': result = 12; break;
        case 'd': case 'D': result = 13; break;
        case 'e': case 'E': result = 14; break;
        case 'f': case 'F': result = 15; break;
    }
    return result;
}

v4 parse_color_from_hexadecimal(string_ref str)
{
    v4 result = {0};

    str = omit_leading_whitespace(str);	
    str = omit_trailing_whitespace(str);

    if (str.string_size > 0)
    {
        if (*str.ptr == '#')
        {
            str.string_size--;
            str.ptr++;
        }

        if (str.string_size == 6 || str.string_size == 8)
        {
            u32 char_index = 0;
            u32 color_index = 0;
            while (char_index < str.string_size)
            {
                char first_digit = *(str.ptr + char_index);
                char second_digit = *(str.ptr + char_index + 1);

                u32 first_digit_parsed = parse_hexadecimal_digit(first_digit);
                u32 second_digit_parsed = parse_hexadecimal_digit(second_digit);

                assert(color_index < 4);
                result.e[color_index] = first_digit_parsed * 16 + second_digit_parsed;

                char_index += 2;
                color_index++;
            }
        }		
    }

    return result;
}

void string_function_test(memory_arena* test_arena)
{
    temporary_memory test = begin_temporary_memory(test_arena);

    string_ref integer = copy_c_string(test_arena, "  100  ");
    i32 integer_parsed = parse_i32(integer);
    assert(integer_parsed == 100);

    string_ref integer2 = copy_c_string(test_arena, "432358192");
    i32 integer2_parsed = parse_i32(integer2);
    assert(integer2_parsed == 432358192);

    string_ref integer3 = copy_c_string(test_arena, "-2137");
    i32 integer3_parsed = parse_i32(integer3);
    assert(integer3_parsed == -2137);

    string_ref integer4 = copy_c_string(test_arena, "   -10   ");
    i32 integer4_parsed = parse_i32(integer4);
    assert(integer4_parsed == -10);

    string_ref fraction1 = copy_c_string(test_arena, "  10,0  ");
    r32 fraction1_parsed = parse_r32(fraction1, ',');
    assert(fraction1_parsed == 10);

    string_ref fraction2 = copy_c_string(test_arena, "10.5");
    r32 fraction2_parsed = parse_r32(fraction2, '.');
    assert(fraction2_parsed == 10.5f);

    string_ref fraction3 = copy_c_string(test_arena, "123.123");
    r32 fraction3_parsed = parse_r32(fraction3, '.');
    assert(fraction3_parsed == 123.123f);

    string_ref fraction4 = copy_c_string(test_arena, "-192.19");
    r32 fraction4_parsed = parse_r32(fraction4, '.');
    assert(fraction4_parsed == -192.19f);

    string_ref hex1 = copy_c_string(test_arena, "#b5d3ff");
    v4 hex1_parsed = parse_color_from_hexadecimal(hex1);
    assert(equals_v4(hex1_parsed, get_v4(181.0f, 211.0f, 255.0f, 0.0f)));

    string_ref hex2 = copy_c_string(test_arena, "   3cff6a6a   ");
    v4 hex2_parsed = parse_color_from_hexadecimal(hex2);
    assert(equals_v4(hex2_parsed, get_v4(60.0f, 255.0f, 106.0f, 106.0f)));

    end_temporary_memory(test, true);
}

string_builder get_string_builder(memory_arena* arena, u32 max_size)
{
    string_builder result = {0};
    result.max_size = max_size;
    result.ptr = (char*)push_size(arena, result.max_size);
    result.current_size = 0;
    return result;
}

void push_string_to_builder(string_builder* builder, string_ref str)
{
    if (str.string_size > 0)
    {
        for (u32 char_index = 0; 
            (char_index < str.string_size && builder->current_size < builder->max_size);
            char_index++)
        {
            char* c = str.ptr + char_index;
            *(builder->ptr + builder->current_size) = *c;
            builder->current_size++;
        }	
    }
}

void push_c_string_to_builder(string_builder* builder, const char* str)
{
    while (*str && builder->current_size < builder->max_size)
    {
        *(builder->ptr + builder->current_size) = *str;
        str++;
        builder->current_size++;
    }
}

void push_char_to_builder(string_builder* builder, char c)
{
    if (builder->current_size < builder->max_size)
    {
        *(builder->ptr + builder->current_size) = c;
        builder->current_size++;
    }
}

void safe_push_null_terminator_to_builder(string_builder* builder)
{
    if (builder->current_size < builder->max_size)
    {
        *(builder->ptr + builder->current_size) = '\0';
        builder->current_size++;
    }
    else
    {
        *(builder->ptr + builder->max_size - 1) = '\0';
    }
}

string_ref get_string_from_string_builder(string_builder* builder)
{
    string_ref result = {0};
    result.ptr = builder->ptr;
    result.string_size = builder->current_size;
    return result;
}

void empty_string_builder(string_builder* builder)
{
    if (builder->current_size > 0)
    {
        zero_memory(builder->current_size, builder->ptr);
        builder->current_size = 0;
    }
}