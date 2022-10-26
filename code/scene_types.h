#pragma once

#include "jorstring.h"
#include "jorutils.h"

enum class scene
{
	GAME,
	MAIN_MENU,
	LEVEL_CHOICE,
	DEATH,
	CREDITS,
	MAP_ERRORS,
	EXIT
};

struct scene_change
{
	b32 change_scene;
	scene new_scene;
	b32 restore_checkpoint;
	string_ref map_to_load;
};

struct introduction_scene_state
{
	r32 text_y_offset;
	b32 can_be_skipped;
	r32 can_be_skipped_timer;
	r32 fade_out_perc;
	r32 fade_in_perc;
	b32 skipped;

	i32 skippable_indicator_index;
	r32 skippable_indicator_timer;
};

struct main_menu_state
{
	r32 time_to_first_interaction;
	r32 fade_out_perc;
	r32 fade_in_perc;
	scene_change active_scene_change;
};

struct level_choice
{
	string_ref name;
	string_ref map_name;
	b32 completed;
};

struct level_choice_menu_state
{
	r32 time_to_first_interaction;
	r32 fade_out_perc;
	r32 fade_in_perc;
	scene_change active_scene_change;
};

struct death_screen_state
{
	b32 initialized;
	string_ref prompt;
	r32 timer;
	b32 transition_to_game;
	r32 fade_out_perc;
	r32 fade_in_perc;

	i32 skippable_indicator_index;
	r32 skippable_indicator_timer;
};

struct credits_screen_state
{
	r32 time_to_first_interaction;
	r32 fade_out_perc;
	r32 fade_in_perc;
	b32 transition_to_main_menu;
	r32 text_y_offset;

	i32 skippable_indicator_index;
	r32 skippable_indicator_timer;
};

struct save
{
	b32 used;
	string_ref map_name;
	u32 player_max_health;
};