#pragma once

#include "jorutils.h"

union v2
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
};

union v3
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
        r32 _z; // ze względu na GCC nie można nazwać zmiennej tak samo jak w innym strukcie w unii
    };
    r32 e[3];
};

union v4
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
        r32 _w;  // ze względu na GCC nie można nazwać zmiennej tak samo jak w innym strukcie w unii
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
};

struct rect
{
    v2 min_corner;
    v2 max_corner;
};

struct cuboid
{
    v3 min_corner;
    v3 max_corner;
};

struct coordinate_system
{
    v2 origin;
    v2 x_axis; // względem origin (0,0)
    v2 y_axis; // względem origin (0,0)
};

i32 sign(i32);
i32 sign(r32);
v2 get_v2(r32, r32);
v3 get_v3(r32, r32, r32);
v3 get_v3(v2, r32);
v4 get_v4(r32, r32, r32, r32);
b32 operator ==(v2, v2);
b32 operator ==(v3, v3);
b32 operator ==(v4, v4);
b32 operator !=(v2, v2);
b32 operator !=(v3, v3);
b32 operator !=(v4, v4);
v2 operator +(v2, v2);
v3 operator +(v3, v3);
v4 operator +(v4, v4);
v2& operator +=(v2&, v2);
v3& operator +=(v3&, v3);
v4& operator +=(v4&, v4);
v2 operator -(v2);
v3 operator -(v3);
v4 operator -(v4);
v2 operator -(v2, v2);
v3 operator -(v3, v3);
v4 operator -(v4, v4);
v2& operator -=(v2&, v2);
v3& operator -=(v3&, v3);
v4& operator -=(v4&, v4);
v2 operator *(r32, v2);
v3 operator *(r32, v3);
v4 operator *(r32, v4);
v2 operator *(v2, r32);
v3 operator *(v3, r32);
v4 operator *(v4, r32);
v2& operator *=(v2&, r32);
v3& operator *=(v3&, r32);
v4& operator *=(v4&, r32);
v2 operator /(v2, r32);
v3 operator /(v3, r32);
v4 operator /(v4, r32);
v2& operator /=(v2&, r32);
v3& operator /=(v3&, r32);
v4& operator /=(v4&, r32);
r32 inner(v2, v2);
r32 inner(v3, v3);
r32 inner(v4, v4);
r32 length_squared(v2);
r32 length_squared(v3);
r32 length_squared(v4);
r32 length(v2);
r32 length(v3);
r32 length(v4);
b32 is_zero(v2);
b32 is_zero(v3);
b32 is_zero(v4);
v2 get_zero_v2();
v3 get_zero_v3();
v4 get_zero_v4();
v2 get_unit_vector(v2);
v3 get_unit_vector(v3);
v4 get_unit_vector(v4);
v2 multiply(r32, v2*);
v2 multiply(v2*, r32);
v2 add(v2, v2*);
v2 subtract(v2, v2*);
v2 scale(v2, r32);
v3 scale(v3, r32);
v4 scale(v4, r32);
v2 hadamard(v2, v2);
v3 hadamard(v3, v3);
v4 hadamard(v4, v4);
v2 normalize(v2);
v3 normalize(v3);
v4 normalize(v4);
v2 get_rotated_unit_vector(r32, b32 radians = false);
v2 rotate_vector(v2, r32, b32 radians = false);
v2 reflection_over_x_axis(v2);
v2 reflection_over_y_axis(v2);
r32 square(r32);
i32 square(i32);
b32 is_nan(r32);
v2 get_min_corner(rect);
v2 get_max_corner(rect);
v2 get_center(rect);
rect get_rect_from_corners(v2, v2);
rect get_rect_from_min_corner(v2, v2);
rect get_rect_from_min_corner(r32, r32, r32, r32);
rect get_rect_from_center(v2, v2);
rect get_rect_from_center(v2, r32);
b32 is_point_inside_rect(rect, v2);
rect add_side_length(rect, v2);
rect move_rect(rect, v2);
rect move_rect(rect*, v2);
v2 get_rect_dimensions(rect);
v2 get_rect_center(rect);
b32 are_intersecting(rect, rect);
v3 get_min_corner(cuboid);
v3 get_max_corner(cuboid);
v3 get_center(cuboid);
cuboid get_cuboid_from_corners(v3, v3);
cuboid get_cuboid_from_dimensions(v3, v3);
cuboid get_cuboid_from_center(v3, v3);
cuboid get_cuboid_from_center(v3, r32);
b32 is_point_inside_cuboid(cuboid, v3);
v3 get_cuboid_dimensions(cuboid);
v3 get_cuboid_center(cuboid);
cuboid get_cuboid_from_rect(rect, r32, r32);
rect get_xy_rect_from_cuboid(cuboid);
cuboid add_side_length(cuboid, v3);
cuboid move_cuboid(cuboid, v3);
b32 are_intersecting(cuboid, cuboid);
r32 safe_ratio_n(r32, r32, r32);
r32 safe_ratio_1(r32, r32);
r32 safe_ratio_0(r32, r32);
v2 get_normal_coordinates_relative_to_rect(v2, rect);
v3 get_normal_coordinates_relative_to_cuboid(v3, cuboid);
r32 clamp(r32, r32, r32);
r32 clamp_01(r32);
v2 clamp_01(v2);
v3 clamp_01(v3);
v4 clamp_01(v4);
r32 lerp(r32, r32, r32);
v2 lerp(v2, r32, v2);
v3 lerp(v3, r32, v3);
v4 lerp(v4, r32, v4);
v2 perp(v2);
i32 multiply(i32, i32, u32);
i32 power(i32, u32);