#pragma once

#include "jorutils.h"
#include "jormath.h"
#include "jorstring.h"

#include "constants.h"
#include "map_types.h"
#include "graphics_types.h"
#include "entity_types.h"
#include "player_types.h"
#include "platform_api_types.h"
#include "scene_types.h"

typedef struct map
{
    u32 width;
    u32 height;
    map_layer map;
    map_layer background;
    map_layer foreground;
    map_layer collision_reference;

    backdrop_properties first_backdrop;
    backdrop_properties second_backdrop;
    v2 first_backdrop_offset;
    v2 second_backdrop_offset;

    // linked list of entities to spawn is deleted after the level initialization
    entity_to_spawn* first_entity_to_spawn;
    entity_to_spawn* last_entity_to_spawn;
    i32 entities_to_spawn_count;
    i32 gate_entities_to_spawn_count;

    r32 initial_max_player_health;

    tile_position starting_tile;
    string_ref next_map;
    string_ref introduction;
    text_lines* introduction_lines;
    string_ref music_file_name;
    b32 complete_when_all_messengers_killed;
} map;

typedef struct static_game_data
{
    level_choice* levels;
    u32 levels_count;
    map_layer collision_reference;

    entity_type_dictionary entity_types_dict;
    entity_type* entity_types;
    u32 entity_types_count;

    entity_type* bullet_types;
    u32 bullet_types_count;

    sprite_effect* sprite_effects;
    u32 sprite_effects_count;

    entity_type* player_normal_bullet_type;
    entity_type* player_power_up_bullet_type;
    animation_frame player_idle_pose;
    animation_frame player_jump_pose;
    animation_frame player_recoil_pose;

    v2 gravity;
    r32 moving_platform_velocity;
    
    r32 player_walking_acceleration;
    r32 player_in_air_acceleration;
    r32 player_jump_acceleration;
    v2 power_up_speed_multipliers;

    r32 default_player_max_health;
    r32 default_player_ignore_enemy_collision_cooldown;
    r32 default_player_invincibility_cooldown;
    r32 default_player_recoil_timer;
    r32 default_player_recoil_acceleration_timer;
    v2 player_as_target_offset;

    r32 default_power_up_invincibility_timer;
    r32 default_power_up_speed_timer;
    r32 default_power_up_damage_timer;
    r32 default_power_up_spread_timer;
    r32 default_power_up_health_bonus;

    ui_graphics ui_gfx;
    gates_graphics gates_gfx;
    switches_graphics switches_gfx;
    display_graphics display_gfx;	
    moving_platforms_graphics platforms_gfx;
    explosions explosion_animations;

    r32 menu_fade_speed;
    r32 game_fade_in_speed;
    r32 game_fade_out_speed;
    r32 game_victory_fade_out_speed;
    r32 death_screen_fade_speed;
    r32 credits_screen_fade_speed;
    r32 introduction_fade_speed;
    r32 default_introduction_can_be_skipped_timer;
    r32 default_time_to_first_menu_interaction;
    
    font ui_font;
    font title_font;

    string_ref title_str;
    string_ref menu_new_game_str;
    string_ref menu_continue_str;
    string_ref menu_credits_str;
    string_ref menu_exit_str;
    string_ref choose_level_message;
    string_ref exit_warning_message;
    string_ref victory_str;
    string_ref checkpoint_str;
    string_ref default_death_message;
    string_ref* death_messages;
    u32 death_messages_count;	
    text_lines* credits_text_lines;

    render_text_options scrolling_text_options;
    render_text_options parsing_errors_text_options;
    r32 introduction_text_speed;
    r32 credits_text_speed;
} static_game_data;

typedef struct level_state
{
    string_ref current_map_name;
    b32 current_map_initialized;
    map current_map;

    player_movement player_movement;
    r32 player_invincibility_cooldown;
    r32 player_ignore_enemy_collision_cooldown;
    
    entity* entities;
    u32 entities_max_count;
    u32 entities_count;

    bullet* bullets; // compact array
    u32 bullets_max_count;
    u32 bullets_count;

    entity* explosions; // compact array
    u32 explosions_max_count;
    u32 explosions_count;

    entity_type* entity_dynamic_types;
    u32 entity_dynamic_types_count;
 
    gate_dictionary gates_dict;
    sprite_effect_dictionary gate_tints_dict;
    entity_type* moving_platform_types[8];

    static_game_data* static_data;

    power_ups power_ups;
    i32 enemies_to_kill_counter;

    r32 screen_shake_duration;
    r32 screen_shake_multiplier;

    b32 show_message;
    b32 show_exit_warning_message;
    string_ref message_to_show;
    v2 messagebox_dimensions;
    r32 min_message_timer;
    i32 message_skippable_indicator_index;
    r32 message_skippable_indicator_timer;

    b32 stop_player_movement;

    scene_change active_scene_change;
    r32 fade_out_perc;
    r32 fade_in_perc;

    b32 show_level_introduction;
    introduction_scene_state introduction;
    b32 show_victory_message;
    r32 show_checkpoint_message_timer;
} level_state;

typedef struct checkpoint
{
    b32 used;
    string_ref map_name;
    
    entity* entities;
    u32 entities_count;
    entity_type* entity_dynamic_types;
    u32 entity_dynamic_types_count;

    i32 enemies_to_kill_counter;
    power_ups power_ups;
} checkpoint;

typedef struct game_state
{
    b32 initialized;
    scene current_scene;

    b32 show_exit_game_option;
    b32 exit_game;
    b32 level_initialized;
    level_state* level_state;
    memory_arena* arena;
    memory_arena* transient_arena;	
     
    char* level_name_buffer;
    main_menu_state main_menu;
    death_screen_state death_screen;
    level_choice_menu_state level_choice_menu;
    credits_screen_state credits_screen;

    input_buffer input_buffer;

    string_ref map_errors;
    temporary_memory game_level_memory;	

    render_list render;

    static_game_data* static_data;
    platform_api platform;

    string_ref cmd_level_to_load;
    b32 exit_level_closes_game;
    b32 skip_introductions;
    b32 skip_deaths_prompts;
 
    checkpoint checkpoint;
} game_state;

void main_game_loop(game_state* game, r32 delta_time);