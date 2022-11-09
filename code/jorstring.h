#pragma once

#include "jorutils.h"
#include "jormath.h"

typedef struct string_ref
{
    i32 string_size;
    char* ptr;
} string_ref;

typedef struct string_builder
{
    char* ptr;
    u32 current_size;
    u32 max_size;
} string_builder;

typedef struct text_lines
{
    string_ref* lines;
    u32 lines_count;
} text_lines;

string_ref get_string_ref(const char*, u32);
string_ref get_string_ref_length_unknown(const char*);
b32 is_empty_string(string_ref);
b32 equals_string_ref(string_ref a, string_ref b);
string_ref copy_string(memory_arena*, string_ref);
string_ref copy_string_to_buffer(char* buffer, u32 buffer_length, string_ref str);
char* get_c_string(memory_arena*, string_ref);
b32 compare_to_c_string(string_ref, const char*);
u32 get_c_string_length_with_limit(const char*, u32);
u32 get_c_string_length(const char*);
string_ref copy_c_string_with_length(memory_arena*, const char*, u32);
string_ref copy_c_string(memory_arena*, const char*);
string_ref copy_c_string_buffer(memory_arena*, const char*, u32);
b32 ends_with(string_ref, const char*);
b32 is_whitespace(char);
b32 is_digit(char);
b32 is_letter(char);
b32 is_all_digits(string_ref);
b32 check_if_string_is_whitespace(string_ref);
string_ref omit_leading_whitespace(string_ref);
string_ref omit_trailing_whitespace(string_ref);
i32 parse_i32(string_ref);
i32 parse_i32_from_range(char*, char*);
u32 how_many_digits(u32);
r32 parse_r32(string_ref, char);
i32* parse_array_of_i32(memory_arena*, u32, string_ref, char);
v4 parse_color_from_hexadecimal(string_ref);
void string_function_test(memory_arena*);

string_builder get_string_builder(memory_arena* arena, u32 max_size);
void push_string_to_builder(string_builder* builder, string_ref str);
void push_c_string_to_builder(string_builder* builder, const char* str);
void push_char_to_builder(string_builder* builder, char c);
void safe_push_null_terminator_to_builder(string_builder* builder);
string_ref get_string_from_string_builder(string_builder* builder);
void empty_string_builder(string_builder* builder);