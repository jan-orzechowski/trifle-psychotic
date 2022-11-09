#pragma once

#include "jormath.h"

typedef enum textures
{
    TEXTURE_NONE,
    TEXTURE_TILESET,
    TEXTURE_FONT,
    TEXTURE_TITLE_FONT,
    TEXTURE_CHARSET,
    TEXTURE_EXPLOSION,
    TEXTURE_BACKGROUND_DESERT,
    TEXTURE_BACKGROUND_ICE_DESERT,
    TEXTURE_BACKGROUND_CLOUDS,
    TEXTURE_BACKGROUND_RED_PLANET_SKY,
    TEXTURE_BACKGROUND_RED_PLANET_DESERT,
    TEXTURE_BACKGROUND_PLANET_ORBIT,
    TEXTURE_BACKGROUND_TITLE_SCREEN
} textures;

// polegamy na tym, by _FIRST było pierwsze, a _LAST ostatnie, dla policzenia ile potrzebujemy miejsc w słowniku
typedef enum sprite_effects_types
{
    _FIRST_SPRITE_EFFECT_TYPE,
    SPRITE_EFFECT_TYPE_OTHER,
    SPRITE_EFFECT_TYPE_DEATH,
    SPRITE_EFFECT_TYPE_SHOCK,
    SPRITE_EFFECT_TYPE_RECOIL,
    SPRITE_EFFECT_TYPE_SPEED,
    SPRITE_EFFECT_TYPE_BULLET_HIT,
    SPRITE_EFFECT_TYPE_GATE_DISPLAY_ACTIVE,
    SPRITE_EFFECT_TYPE_GATE_DISPLAY_INACTIVE,
    SPRITE_EFFECT_TYPE_INVINCIBILITY,
    _LAST_SPRITE_EFFECT_TYPE
} sprite_effects_types;

typedef enum sprite_effect_flags
{
    SPRITE_EFFECT_FLAG_REPEATS = (1 << 0),
    SPRITE_EFFECT_FLAG_ADDITIVE_MODE = (1 << 1),
    SPRITE_EFFECT_FLAG_REVERSE_VALUES = (1 << 2)
} sprite_effect_flags;

typedef struct sprite_effect_stage
{
    r32 amplitude;
    r32 phase_shift;
    r32 vertical_shift;
    r32 period; // jeśli 0, to mamy stałą funkcję
    r32 stage_duration;
} sprite_effect_stage;

typedef struct sprite_effect
{
    sprite_effects_types type;
    v4 color;
    sprite_effect_stage* stages;
    u32 stages_count;
    r32 total_duration;
    sprite_effect_flags flags;
} sprite_effect;

typedef struct sprite_effect_dictionary
{
    sprite_effect** sprite_effects;
    u32 sprite_effects_count;
    u32 probing_jump;
} sprite_effect_dictionary;

typedef struct sprite_part
{
    textures texture;
    rect texture_rect;
    v2 offset_in_pixels;
    direction default_direction;
} sprite_part;

typedef struct sprite
{
    sprite_part* parts;
    u32 parts_count;
    b32 flip_horizontally;
} sprite;

typedef struct shooting_rotation_sprites
{
    sprite up;
    v2 up_bullet_offset;
    sprite right_up;
    v2 right_up_bullet_offset;
    sprite right;
    v2 right_bullet_offset;
    sprite right_down;
    v2 right_down_bullet_offset;
    sprite down;
    v2 down_bullet_offset;
} shooting_rotation_sprites;

typedef struct animation_frame
{
    sprite sprite;
    r32 duration;
} animation_frame;

typedef  struct animation
{
    animation_frame* frames;
    u32 frames_count;
    r32 total_duration;
} animation;

typedef struct gate_graphics
{
    sprite_part gate;
    sprite_part frame_upper;
    sprite_part frame_lower;
} gate_graphics;

typedef struct gates_graphics
{
    gate_graphics blue;
    gate_graphics grey;
    gate_graphics red;
    gate_graphics green;
} gates_graphics;

typedef struct switch_graphics
{
    sprite_part frame_left;
    sprite_part frame_middle;
    sprite_part frame_right;
} switch_graphics;

typedef struct switches_graphics
{
    switch_graphics blue;
    switch_graphics grey;
    switch_graphics red;
    switch_graphics green;
} switches_graphics;

typedef struct display_graphics
{
    sprite_part gate_upper_display;
    sprite_part gate_lower_display;
    sprite_part switch_left_display;
    sprite_part switch_middle_display;
    sprite_part switch_right_display;
} display_graphics;

typedef struct moving_platform_graphics
{
    sprite_part left;
    sprite_part middle;
    sprite_part right;
} moving_platform_graphics;

typedef struct moving_platforms_graphics
{
    moving_platform_graphics blue;
    moving_platform_graphics grey;
    moving_platform_graphics red;
    moving_platform_graphics green;
} moving_platforms_graphics;

typedef struct explosions
{
    animation* size_16x16_variant_1;
    animation* size_16x16_variant_2;
    animation* size_16x16_variant_3;
    animation* size_16x16_variant_4;
    animation* size_16x16_variant_5;
    animation* size_16x16_variant_6;
    animation* size_16x16_variant_7_blue;
    animation* size_16x16_variant_8_blue;
    animation* size_16x16_variant_9_blue;
    animation* size_24x24;
    animation* size_32x32;
    animation* size_48x48;
    animation* size_96x96;
} explosions;

typedef struct backdrop_properties
{
    // jeśli 0, backdrop jest nieruchomy, jeśli 1, porusza się tak jak gracz
    textures texture;
    v2 size;
    i32 x_slowdown;
    i32 y_slowdown;
    r32 x_speed;
    r32 y_speed;
} backdrop_properties;

typedef struct ui_graphics
{
    rect crosshair;

    rect healthbar_icon;
    rect healthbar_empty_bar;
    rect healthbar_red_bar;
    rect healthbar_white_bar;

    rect msgbox_frame_upper_left;
    rect msgbox_frame_upper;
    rect msgbox_frame_upper_right;
    rect msgbox_frame_right;
    rect msgbox_frame_lower_right;
    rect msgbox_frame_lower;
    rect msgbox_frame_lower_left;
    rect msgbox_frame_left;
    rect msgbox_frame_background;

    rect msgbox_dots_1;
    rect msgbox_dots_2;
    rect msgbox_dots_3;
} ui_graphics;

typedef struct font
{
    u32 width_in_pixels;
    u32 height_in_pixels;
    u32 glyph_spacing_width;
    i32 letter_spacing;
    i32 line_spacing;
    b32 uppercase_only;
    textures texture;
} font;

typedef struct render_text_options
{
    font font;
    rect writing_area;
    b32 allow_horizontal_overflow;
    b32 allow_vertical_overflow;
    b32 add_tint;
    v4 text_tint;
} render_text_options;