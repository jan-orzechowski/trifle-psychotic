#pragma once

#include "jorutils.h"
#include "jorstring.h"

struct render_list
{
	u32 max_push_buffer_size;
	u32 push_buffer_size;
	u8* push_buffer_base;
};

struct read_file_result
{
	char* contents;
	int size;
};

struct write_to_tile
{
	void* buffer;
	u32 length;
};

typedef read_file_result read_file_func(const char* path);
typedef void save_file_func(const char* path, write_to_tile contents);
typedef read_file_result load_prefs_func();
typedef void save_prefs_func(write_to_tile contents);
typedef void start_playing_music_func(string_ref audio_file_name);
typedef void stop_playing_music_func(int fade_out_ms);
typedef void render_list_to_output_func(render_list* render);
typedef void render_list_to_output_func(render_list* render);

struct platform_api
{
	read_file_func* read_file;
	save_file_func* save_file;
	load_prefs_func* load_prefs;
	save_prefs_func* save_prefs;
	start_playing_music_func* start_playing_music;
	stop_playing_music_func* stop_playing_music;
	render_list_to_output_func* render_list_to_output;
};

struct key_press
{
	int number_of_presses;
};

struct game_input
{
	b32 is_left_mouse_key_held;
	i32 mouse_x;
	i32 mouse_y;
	key_press up;
	key_press down;
	key_press left;
	key_press right;
	key_press fire;
	key_press escape;
	b32 quit;
};

struct input_buffer
{
	game_input* buffer;
	u32 size;
	u32 current_index;
};