#pragma once

#include "main.h"
#include "text_rendering.h"

typedef struct tmx_parsing_error tmx_parsing_error;
struct tmx_parsing_error
{
    string_ref message;
    tmx_parsing_error* next;
};

typedef struct tmx_parsing_error_report
{
    tmx_parsing_error* first_error;
    tmx_parsing_error* last_error;
    u32 errors_count;
} tmx_parsing_error_report;

typedef struct tmx_map_parsing_result
{
    map parsed_map;
    tmx_parsing_error_report* errors;
} tmx_map_parsing_result;

tmx_map_parsing_result load_map(platform_api* platform, string_ref map_name, memory_arena* arena, memory_arena* transient_arena);
map_layer load_collision_map(memory_arena* permanent_arena, memory_arena* transient_arena, read_file_result file);
string_ref get_parsing_errors_message(memory_arena* arena, render_text_options* text_options, tmx_parsing_error_report* errors);