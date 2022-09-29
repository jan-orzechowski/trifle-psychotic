#pragma once

#include "main.h"

struct tmx_parsing_error
{
	string_ref message;
	tmx_parsing_error* next;
};

struct tmx_parsing_error_report
{
	tmx_parsing_error* first_error;
	tmx_parsing_error* last_error;
	u32 errors_count;
};

struct tmx_map_parsing_result
{
	map parsed_map;
	tmx_parsing_error_report* errors;
};

tmx_map_parsing_result read_map_from_tmx_file(memory_arena* permanent_arena, memory_arena* transient_arena, 
	read_file_result file, const char* layer_name, b32 clean_up_transient_arena);