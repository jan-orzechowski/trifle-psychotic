#pragma once

#include "jorutils.h"

typedef union v2
{
    struct
    {
        r32 x;
        r32 y;
    };
    struct
    {
        r32 u;
        r32 v;
    };
    r32 e[2];
} v2;

typedef union v3
{
    struct
    {
        r32 x;
        r32 y;
        r32 z;
    };
    struct
    {
        r32 r;
        r32 g;
        r32 b;
    };
    struct
    {
        v2 xy;
        r32 _z;
    };
    r32 e[3];
} v3;

typedef union v4
{
    struct
    {
        r32 x;
        r32 y;
        r32 z;
        r32 w;
    };
    struct
    {
        v3 xyz;
        r32 _w;
    };
    struct
    {
        union
        {
            struct
            {
                r32 r;
                r32 g;
                r32 b;
            };
            v3 rgb;
        };
        r32 a;
    };
    r32 e[4];
} v4;

typedef struct rect
{
    v2 min_corner;
    v2 max_corner;
} rect;

i32 sign_i32(i32 value);
i32 sign_r32(r32 value);
i32 multiply(i32 num, i32 multiplier, u32 times);
i32 power(i32 num, u32 exponent);
v2 get_v2(r32 x, r32 y);
v3 get_v3(r32 x, r32 y, r32 z);
v3 get_v3_from_v2(v2 xy, r32 z);
v4 get_v4(r32 x, r32 y, r32 z, r32 w);
b32 equals_v2(v2 a, v2 b);
b32 equals_v3(v3 a, v3 b);
b32 equals_v4(v4 a, v4 b);
b32 not_equals_v2(v2 a, v2 b);
b32 not_equals_v3(v3 a, v3 b);
b32 not_equals_v4(v4 a, v4 b);
v2 add_v2(v2 a, v2 b);
v3 add_v3(v3 a, v3 b);
v4 add_v4(v4 a, v4 b);
void add_v2_in_place(v2* a, v2 b);
void add_v3_in_place(v3* a, v3 b);
void add_v4_in_place(v4* a, v4 b);
v2 minus_v2(v2 a);
v3 minus_v3(v3 a);
v4 minus_v4(v4 a);
v2 subtract_v2(v2 a, v2 b);
v3 subtract_v3(v3 a, v3 b);
v4 subtract_v4(v4 a, v4 b);
v2 multiply_v2(v2 a, r32 b);
v3 multiply_v3(v3 a, r32 b);
v4 multiply_v4(v4 a, r32 b);
v2 divide_v2(v2 a, r32 b);
v3 divide_v3(v3 a, r32 b);
v4 divide_v4(v4 a, r32 b);
r32 inner_v2(v2 a, v2 b);
r32 inner_v3(v3 a, v3 b);
r32 inner_v4(v4 a, v4 b);
r32 length_squared_v2(v2 a);
r32 length_squared_v3(v3 a);
r32 length_squared_v4(v4 a);
r32 length_v2(v2 a);
r32 length_v3(v3 a);
r32 length_v4(v4 a);
b32 is_zero_v2(v2 a);
b32 is_zero_v3(v3 a);
b32 is_zero_v4(v4 a);
v2 get_zero_v2(void);
v3 get_zero_v3(void);
v4 get_zero_v4(void);
v2 get_unit_v2(v2 a);
v3 get_unit_v3(v3 a);
v4 get_unit_v4(v4 a);
v2 get_rotated_unit_v2(r32 angle, b32 radians);
v2 rotate_v2(v2 vec, r32 angle, b32 radians);
v2 reflection_over_x_axis_v2(v2 a);
v2 reflection_over_y_axis_v2(v2 a);
r32 square_r32(r32 a);
i32 square_i32(i32 a);
b32 is_r32_nan(r32 a);
v2 get_rect_min_corner(rect rect);
v2 get_rect_max_corner(rect rect);
v2 get_rect_center(rect rect);
rect get_rect_from_corners(v2 min_corner, v2 max_corner);
rect get_rect_from_min_corner(v2 min_corner, v2 dimensions);
rect get_rect_from_center_and_dimensions(v2 center, v2 dimensions);
rect get_rect_from_center_and_half_side(v2 center, r32 half_side);
b32 is_point_inside_rect(rect rectangle, v2 point);
rect add_side_length(rect rectangle, v2 sides_length);
rect move_rect(rect rect_to_move, v2 distance);
rect move_rect_in_place(rect* rect_to_move, v2 distance);
v2 get_rect_dimensions(rect rect);
v2 get_rect_center(rect rect);
b32 are_intersecting(rect a, rect b);
v2 get_normal_coordinates_relative_to_rect(v2 world_coords, rect rect);