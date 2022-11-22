#pragma once

#include "jorstring.h"
#include "jorutils.h"

typedef enum scene
{
    SCENE_GAME,
    SCENE_MAIN_MENU,
    SCENE_LEVEL_CHOICE,
    SCENE_DEATH,
    SCENE_CREDITS,
    SCENE_MAP_ERRORS,
    SCENE_EXIT
} scene;

typedef struct scene_change
{
    b32 change_scene;
    scene new_scene;
    b32 restore_checkpoint;
    string_ref map_to_load;
} scene_change;

typedef struct introduction_scene_state
{
    r32 text_y_offset;
    b32 can_be_skipped;
    r32 can_be_skipped_timer;
    r32 fade_out_perc;
    r32 fade_in_perc;
    b32 skipped;

    i32 skippable_indicator_index;
    r32 skippable_indicator_timer;
} introduction_scene_state;

typedef struct main_menu_state
{
    r32 time_to_first_interaction;
    r32 fade_out_perc;
    r32 fade_in_perc;
    scene_change active_scene_change;
} main_menu_state;

typedef struct level_choice
{
    string_ref name;
    string_ref map_name;
    b32 completed;
} level_choice;

typedef struct level_choice_menu_state
{
    r32 time_to_first_interaction;
    r32 fade_out_perc;
    r32 fade_in_perc;
    scene_change active_scene_change;
} level_choice_menu_state;

typedef struct death_screen_state
{
    b32 initialized;
    string_ref prompt;
    r32 timer;
    b32 transition_to_game;
    r32 fade_out_perc;
    r32 fade_in_perc;

    i32 skippable_indicator_index;
    r32 skippable_indicator_timer;
} death_screen_state;

typedef struct credits_screen_state
{
    r32 time_to_first_interaction;
    r32 fade_out_perc;
    r32 fade_in_perc;
    b32 transition_to_next_scene;
    r32 text_y_offset;

    b32 ending_text_mode;
    i32 skippable_indicator_index;
    r32 skippable_indicator_timer;
} credits_screen_state;