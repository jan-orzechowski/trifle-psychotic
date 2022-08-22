#pragma once

#include "jorutils.h"

// wektory

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
        r32 z;
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
        r32 w;
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

v2 get_v2(r32 x, r32 y)
{
    v2 result;

    result.x = x;
    result.y = y;

    return result;
}

v3 get_v3(r32 x, r32 y, r32 z)
{
    v3 result;

    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

v3 get_v3(v2 xy, r32 z)
{
    v3 result;

    result.x = xy.x;
    result.y = xy.y;
    result.z = z;

    return result;
}

v4 get_v4(r32 x, r32 y, r32 z, r32 w)
{
    v4 result;

    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;

    return result;
}

v2 operator +(v2 a, v2 b)
{
    v2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

v3 operator +(v3 a, v3 b)
{
    v3 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

v4 operator +(v4 a, v4 b)
{
    v4 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;

    return result;
}

v2& operator +=(v2& a, v2 b)
{
    a = a + b;

    return a;
}

v3& operator +=(v3& a, v3 b)
{
    a = a + b;

    return a;
}

v4& operator +=(v4& a, v4 b)
{
    a = a + b;

    return a;
}

v2 operator -(v2 a)
{
    v2 result;

    result.x = -a.x;
    result.y = -a.y;

    return result;
}

v3 operator -(v3 a)
{
    v3 result;

    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;

    return result;
}

v4 operator -(v4 a)
{
    v4 result;

    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;

    return result;
}

v2 operator -(v2 a, v2 b)
{
    v2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

v3 operator -(v3 a, v3 b)
{
    v3 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return result;
}

v4 operator -(v4 a, v4 b)
{
    v4 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;

    return result;
}

v2& operator -=(v2& a, v2 b)
{
    a = a - b;

    return a;
}

v3& operator -=(v3& a, v3 b)
{
    a = a - b;

    return a;
}

v4& operator -=(v4& a, v4 b)
{
    a = a - b;

    return a;
}

v2 operator *(r32 a, v2 b)
{
    v2 result;

    result.x = a * b.x;
    result.y = a * b.y;

    return result;
}

v3 operator *(r32 a, v3 b)
{
    v3 result;

    result.x = a * b.x;
    result.y = a * b.y;
    result.z = a * b.z;

    return result;
}

v4 operator *(r32 a, v4 b)
{
    v4 result;

    result.x = a * b.x;
    result.y = a * b.y;
    result.z = a * b.z;
    result.w = a * b.w;

    return result;
}

v2 operator *(v2 b, r32 a)
{
    v2 result = a * b;
    return result;
}

v3 operator *(v3 b, r32 a)
{
    v3 result = a * b;
    return result;
}

v4 operator *(v4 b, r32 a)
{
    v4 result = a * b;
    return result;
}

v2& operator *=(v2& b, r32 a)
{
    b = a * b;

    return b;
}

v3& operator *=(v3& b, r32 a)
{
    b = a * b;

    return b;
}

v4& operator *=(v4& b, r32 a)
{
    b = a * b;

    return b;
}

v2 operator /(v2 a, r32 b)
{
    v2 result;

    result.x = a.x / b;
    result.y = a.y / b;

    return result;
}

v3 operator /(v3 a, r32 b)
{
    v3 result;

    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;

    return result;
}

v4 operator /(v4 a, r32 b)
{
    v4 result;

    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;
    result.w = a.w / b;

    return result;
}

v2& operator /=(v2& a, r32 b)
{
    a = a / b;

    return a;
}

v3& operator /=(v3& a, r32 b)
{
    a = a / b;

    return a;
}

v4& operator /=(v4& a, r32 b)
{
    a = a / b;

    return a;
}

r32 inner(v2 a, v2 b)
{
    r32 result = (a.x * b.x) + (a.y * b.y);
    return result;
}

r32 inner(v3 a, v3 b)
{
    r32 result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    return result;
}

r32 inner(v4 a, v4 b)
{
    r32 result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
    return result;
}

r32 length_squared(v2 a)
{
    r32 result = inner(a, a);
    return result;
}

r32 length_squared(v3 a)
{
    r32 result = inner(a, a);
    return result;
}

r32 length_squared(v4 a)
{
    r32 result = inner(a, a);
    return result;
}

r32 length(v2 a)
{
    r32 result = length_squared(a);
    result = SDL_sqrt(result);
    return result;
}

r32 length(v3 a)
{
    r32 result = length_squared(a);
    result = SDL_sqrt(result);
    return result;
}

r32 length(v4 a)
{
    r32 result = length_squared(a);
    result = SDL_sqrt(result);
    return result;
}

b32 is_zero(v2 a)
{
    b32 result = false;
    result = (a.x == 0.0f && a.y == 0.0f);
    return result;
}

b32 is_zero(v3 a)
{
    b32 result = false;
    result = (a.x == 0.0f && a.y == 0.0f && a.z == 0.0f);
    return result;
}

b32 is_zero(v4 a)
{
    b32 result = false;
    result = (a.x == 0.0f && a.y == 0.0f && a.z == 0.0f && a.w == 0.0f);
    return result;
}

v2 get_zero_v2()
{
    v2 result = {};
    return result;
}

v3 get_zero_v3()
{
    v3 result = {};
    return result;
}

v4 get_zero_v4()
{
    v4 result = {};
    return result;
}

v2 get_unit_vector(v2 a)
{
    v2 result = a / length(a);
    return result;
}

v3 get_unit_vector(v3 a)
{
    v3 result = a / length(a);
    return result;
}

v4 get_unit_vector(v4 a)
{
    v4 result = a / length(a);
    return result;
}

// dodane tylko i wyłącznie dlatego, że skurwiałe VS ma od 2019 bug w watch window 
// i nie jest w stanie ewaluować wyrażeń z przeciążonymi operatorami
// podobnie podaję tutaj v2* zamiast v2 - inaczej VS mówi, że
// "Passing class, struct, or union types by value to a function evaluation is not supported."
v2 multiply(r32 a, v2* b)
{
    v2 result;

    result.x = a * b->x;
    result.y = a * b->y;

    return result;
}

v2 multiply(v2* a, r32 b)
{
    v2 result = *a * b;
    return result;
}

v2 add(v2 a, v2* b)
{
    v2 result;

    result.x = a.x + b->x;
    result.y = a.y + b->y;

    return result;
}

v2 subtract(v2 a, v2* b)
{
    v2 result;

    result.x = a.x - b->x;
    result.y = a.y - b->y;

    return result;
}

v2 scale(v2 a, r32 new_length)
{
    r32 ratio = new_length / SDL_sqrt(length(a));
    v2 result = a * ratio;
    return result;
}

v3 scale(v3 a, r32 new_length)
{
    r32 ratio = new_length / SDL_sqrt(length(a));
    v3 result = a * ratio;
    return result;
}

v4 scale(v4 a, r32 new_length)
{
    r32 ratio = new_length / SDL_sqrt(length(a));
    v4 result = a * ratio;
    return result;
}

v2 hadamard(v2 a, v2 b)
{
    v2 result = {};
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    return result;
}

v3 hadamard(v3 a, v3 b)
{
    v3 result = {};
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    return result;
}

v4 hadamard(v4 a, v4 b)
{
    v4 result = {};
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    result.w = a.w * b.w;
    return result;
}

v2 normalize(v2 a)
{
    v2 result = a / length(a);
    return result; 
}

v3 normalize(v3 a)
{
    v3 result = a / length(a);
    return result;
}

v4 normalize(v4 a)
{
    v4 result = a / length(a);
    return result;
}

// arytmetyka

r32 square(r32 a)
{
    r32 result = a * a;
    return result;
}

i32 square(i32 a)
{
    i32 result = a * a;
    return result;
}

b32 is_nan(r32 a)
{
    b32 result = (a != a);
    return result;
}

// prostokąty

struct rect
{
    v2 min_corner;
    v2 max_corner;
};

v2 get_min_corner(rect rect)
{
    v2 result = rect.min_corner;
    return result;
}

v2 get_max_corner(rect rect)
{
    v2 result = rect.max_corner;
    return result;
}

v2 get_center(rect rect)
{
    v2 result = 0.5f * (rect.min_corner + rect.max_corner);
    return result;
}

rect get_rect_from_corners(v2 min_corner, v2 max_corner)
{
    rect result = {};
    result.min_corner = min_corner;
    result.max_corner = max_corner;
    return result;
}

rect get_rect_from_dimensions(v2 min_corner, v2 dimensions)
{
    rect result = {};
    result.min_corner = min_corner;
    result.max_corner = min_corner + dimensions;
    return result;
}

rect get_rect_from_center(v2 center, v2 dimensions)
{
    rect result = {};
    result.min_corner = get_v2(center.x - (dimensions.x / 2), center.y - (dimensions.y / 2));
    result.max_corner = get_v2(center.x + (dimensions.x / 2), center.y + (dimensions.y / 2));
    return result;
}

rect get_rect_from_center(v2 center, r32 half_side)
{
    rect result = {};
    result.min_corner = get_v2(center.x - half_side, center.y - half_side);
    result.max_corner = get_v2(center.x + half_side, center.y + half_side);
    return result;
}

b32 is_point_inside_rect(rect rectangle, v2 point)
{
    b32 result = false;
    result = (point.x >= rectangle.min_corner.x
           && point.y >= rectangle.min_corner.y
           && point.x <  rectangle.max_corner.x
           && point.y <  rectangle.max_corner.y);
    return result;
}

rect add_side_length(rect rectangle, v2 sides_length)
{
    rect result = {};

    r32 half_x = sides_length.x / 2.0f;
    r32 half_y = sides_length.y / 2.0f;

    result.min_corner.x = rectangle.min_corner.x - half_x;
    result.min_corner.y = rectangle.min_corner.y - half_y;
    
    result.max_corner.x = rectangle.max_corner.x + half_x;
    result.max_corner.y = rectangle.max_corner.y + half_y;

    return result;
}

v2 get_rect_dimensions(rect rect)
{
    v2 result = {};

    result.x = rect.max_corner.x - rect.min_corner.x;
    result.y = rect.max_corner.y - rect.min_corner.y;

    return result;
}

v2 get_rect_center(rect rect)
{
    v2 result = {};

    result.x = rect.min_corner.x + ((rect.max_corner.x - rect.min_corner.x) / 2);
    result.y = rect.min_corner.y + ((rect.max_corner.y - rect.min_corner.y) / 2);
    
    return result;
}

inline b32 are_intersecting(rect a, rect b)
{
    // sprawdzamy, czy odcinki na osi przecinają się
    b32 x_axis_intersect = (false == (b.max_corner.x <= a.min_corner.x || b.min_corner.x >= a.max_corner.x));
    b32 y_axis_intersect = (false == (b.max_corner.y <= a.min_corner.y || b.min_corner.y >= a.max_corner.y));
    b32 result = (x_axis_intersect && y_axis_intersect);
    return result;
}

// prostopadłościan

struct cuboid
{
    v3 min_corner;
    v3 max_corner;
};

v3 get_min_corner(cuboid cub)
{
    v3 result = cub.min_corner;
    return result;
}

v3 get_max_corner(cuboid cub)
{
    v3 result = cub.max_corner;
    return result;
}

v3 get_center(cuboid cub)
{
    v3 result = 0.5f * (cub.min_corner + cub.max_corner);
    return result;
}

cuboid get_cuboid_from_corners(v3 min_corner, v3 max_corner)
{
    cuboid result = {};
    result.min_corner = min_corner;
    result.max_corner = max_corner;
    return result;
}

cuboid get_cuboid_from_dimensions(v3 min_corner, v3 dimensions)
{
    cuboid result = {};
    result.min_corner = min_corner;
    result.max_corner = min_corner + dimensions;
    return result;
}

cuboid get_cuboid_from_center(v3 center, v3 dimensions)
{
    cuboid result = {};
    result.min_corner = get_v3(center.x - (dimensions.x / 2), center.y - (dimensions.y / 2), center.z - (dimensions.z / 2));
    result.max_corner = get_v3(center.x + (dimensions.x / 2), center.y + (dimensions.y / 2), center.z + (dimensions.z / 2));
    return result;
}

cuboid get_cuboid_from_center(v3 center, r32 half_side)
{
    cuboid result = {};
    result.min_corner = get_v3(center.x - half_side, center.y - half_side, center.z - half_side);
    result.max_corner = get_v3(center.x + half_side, center.y + half_side, center.z - half_side);
    return result;
}

b32 is_point_inside_cuboid(cuboid cub, v3 point)
{
    b32 result = false;
    result = 
          (point.x >= cub.min_corner.x
        && point.y >= cub.min_corner.y
        && point.z >= cub.min_corner.z
        && point.x <  cub.max_corner.x
        && point.y <  cub.max_corner.y
        && point.z <  cub.max_corner.z);
    return result;
}

v3 get_cuboid_dimensions(cuboid cub)
{
    v3 result = {};

    result.x = cub.max_corner.x - cub.min_corner.x;
    result.y = cub.max_corner.y - cub.min_corner.y;
    result.z = cub.max_corner.z - cub.min_corner.z;

    return result;
}

v3 get_cuboid_center(cuboid cub)
{
    v3 result = {};

    result.x = cub.min_corner.x + ((cub.max_corner.x - cub.min_corner.x) / 2);
    result.y = cub.min_corner.y + ((cub.max_corner.y - cub.min_corner.y) / 2);
    result.z = cub.min_corner.z + ((cub.max_corner.z - cub.min_corner.z) / 2);

    return result;
}

cuboid get_cuboid_from_rect(rect rect, r32 min_z, r32 max_z)
{
    cuboid result = {};
    result.min_corner = get_v3(rect.min_corner.x, rect.min_corner.y, min_z);
    result.max_corner = get_v3(rect.max_corner.x, rect.max_corner.y, max_z);
    return result;
}

rect get_xy_rect_from_cuboid(cuboid cub)
{
    rect result = {};
    result.min_corner = cub.min_corner.xy;
    result.max_corner = cub.max_corner.xy;
    return result;
}

cuboid add_side_length(cuboid cub, v3 sides_lengths)
{
    cuboid result = {};

    r32 half_x = sides_lengths.x / 2.0f;
    r32 half_y = sides_lengths.y / 2.0f;
    r32 half_z = sides_lengths.z / 2.0f;

    result.min_corner.x = cub.min_corner.x - half_x;
    result.min_corner.y = cub.min_corner.y - half_y;
    result.min_corner.z = cub.min_corner.z - half_z;

    result.max_corner.x = cub.max_corner.x + half_x;
    result.max_corner.y = cub.max_corner.y + half_y;
    result.max_corner.z = cub.max_corner.z + half_z;

    return result;
}

inline b32 are_intersecting(cuboid a, cuboid b)
{
    // sprawdzamy, czy odcinki na osi przecinają się
    b32 x_axis_intersect = (false == (b.max_corner.x <= a.min_corner.x || b.min_corner.x >= a.max_corner.x));
    b32 y_axis_intersect = (false == (b.max_corner.y <= a.min_corner.y || b.min_corner.y >= a.max_corner.y));
    b32 z_axis_intersect = (false == (b.max_corner.z <= a.min_corner.z || b.min_corner.z >= a.max_corner.z));
    b32 result = (x_axis_intersect && y_axis_intersect && z_axis_intersect);
    return result;
}

inline r32 safe_ratio_n(r32 numerator, r32 denominator, r32 value_if_denominator_is_zero)
{
    r32 result = value_if_denominator_is_zero;

    if (denominator != 0)
    {
        result = numerator / denominator;
    }

    return result;
}

inline r32 safe_ratio_1(r32 numerator, r32 denominator)
{
    r32 result = safe_ratio_n(numerator, denominator, 1.0f);
    return result;
}

inline r32 safe_ratio_0(r32 numerator, r32 denominator)
{
    r32 result = safe_ratio_n(numerator, denominator, 0.0f);
    return result;
}

inline v2 get_normal_coordinates_relative_to_rect(v2 world_coords, rect rect)
{
    r32 new_x = safe_ratio_0((world_coords.x - rect.min_corner.x), (rect.max_corner.x - rect.min_corner.x));
    r32 new_y = safe_ratio_0((world_coords.y - rect.min_corner.y), (rect.max_corner.y - rect.min_corner.y));
    v2 result = get_v2(new_x, new_y);
    return result;
}

inline v3 get_normal_coordinates_relative_to_cuboid(v3 world_coords, cuboid cub)
{
    r32 new_x = safe_ratio_0((world_coords.x - cub.min_corner.x), (cub.max_corner.x - cub.min_corner.x));
    r32 new_y = safe_ratio_0((world_coords.y - cub.min_corner.y), (cub.max_corner.y - cub.min_corner.y));
    r32 new_z = safe_ratio_0((world_coords.z - cub.min_corner.z), (cub.max_corner.z - cub.min_corner.z));
    v3 result = get_v3(new_x, new_y, new_z);
    return result;
}

inline r32 clamp(r32 min, r32 value, r32 max)
{
    r32 result = value;

    if (result < min)
    {
        result = min;
    }

    if (result > max)
    {
        result = max;
    }

    return result;
}

inline r32 clamp_01(r32 value)
{
    r32 result = clamp(0.0f, value, 1.0f);
    return result;
}

inline v2 clamp_01(v2 vec)
{
    v2 result = {};
    result.x = clamp_01(vec.x);
    result.y = clamp_01(vec.y);
    return result;
}

inline v3 clamp_01(v3 vec)
{
    v3 result = {};
    result.x = clamp_01(vec.x);
    result.y = clamp_01(vec.y);
    result.z = clamp_01(vec.z);
    return result;
}

inline v4 clamp_01(v4 vec)
{
    v4 result = {};
    result.x = clamp_01(vec.x);
    result.y = clamp_01(vec.y);
    result.z = clamp_01(vec.z);
    result.w = clamp_01(vec.w);
    return result;
}

inline r32 lerp(r32 a, r32 perc, r32 b)
{
    r32 result = a + ((b - a) * perc);
    //alternatywnie: r32 result = (1.0f - perc) * a + (perc * b);
    return result;
}

inline v2 lerp(v2 a, r32 perc, v2 b)
{
    v2 result = a + ((b - a) * perc);
    return result;
}

inline v3 lerp(v3 a, r32 perc, v3 b)
{
    v3 result = a + ((b - a) * perc);
    return result;
}

inline v4 lerp(v4 a, r32 perc, v4 b)
{
    v4 result = a + ((b - a) * perc);
    return result;
}

// od perpendicular - prostopadły wektor
v2 perp(v2 a)
{
    v2 result = get_v2(-a.y, a.x);
    return result;
}

struct coordinate_system
{
    v2 origin;
    v2 x_axis; // względem origin (0,0)
    v2 y_axis; // względem origin (0,0)
};