#include "jormath.h"
#include <SDL.h>

// podstawowe

i32 sign_i32(i32 value)
{
    i32 result = value >= 0 ? 1 : -1;
    return result;
}

i32 sign_r32(r32 value)
{
    i32 result = value >= 0 ? 1 : -1;
    return result;
}

i32 multiply(i32 num, i32 multiplier, u32 times)
{
    i32 result;
    if (times > 1)
    {
        result = multiply(num, multiplier, times - 1) * multiplier;
    }
    else
    {
        result = num * multiplier;
    }
    return result;
}

i32 power(i32 num, u32 exponent)
{
    i32 result;
    if (exponent >= 2)
    {
        result = multiply(num, num, exponent - 1);
    }
    else if (exponent == 1)
    {
        result = num;
    }
    else // 0
    {
        result = 1;
    }
    return result;
}

// wektory

v2 get_v2(r32 x, r32 y)
{
    v2 result = {0};

    result.x = x;
    result.y = y;

    return result;
}

v3 get_v3(r32 x, r32 y, r32 z)
{
    v3 result = {0};

    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

v3 get_v3_from_v2(v2 xy, r32 z)
{
    v3 result = {0};

    result.x = xy.x;
    result.y = xy.y;
    result.z = z;

    return result;
}

v4 get_v4(r32 x, r32 y, r32 z, r32 w)
{
    v4 result = {0};

    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;

    return result;
}

b32 equals_v2(v2 a, v2 b)
{
    b32 result = ((a.x == b.x)
        && (a.y == b.y));
    return result;
}

b32 equals_v3(v3 a, v3 b)
{
    b32 result = ((a.x == b.x)
        && (a.y == b.y)
        && (a.z == b.z));
    return result;
}

b32 equals_v4(v4 a, v4 b)
{
    b32 result = ((a.x == b.x)
        && (a.y == b.y)
        && (a.z == b.z)
        && (a.w == b.w));
    return result;
}

b32 not_equals_v2(v2 a, v2 b)
{
    b32 result = ((a.x != b.x)
        || (a.y != b.y));
    return result;
}

b32 not_equals_v3(v3 a, v3 b)
{
    b32 result = ((a.x != b.x)
        || (a.y != b.y)
        || (a.z != b.z));
    return result;
}

b32 not_equals_v4(v4 a, v4 b)
{
    b32 result = ((a.x != b.x)
        || (a.y != b.y)
        || (a.z != b.z)
        || (a.w != b.w));
    return result;
}

v2 add_v2(v2 a, v2 b)
{
    v2 result = {0};

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

v3 add_v3(v3 a, v3 b)
{
    v3 result = {0};

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;

    return result;
}

v4 add_v4(v4 a, v4 b)
{
    v4 result = {0};

    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;

    return result;
}

void add_v2_in_place(v2* a, v2 b)
{    
    a->x = a->x + b.x;
    a->y = a->y + b.y;
}

void add_v3_in_place(v3* a, v3 b)
{
    a->x = a->x + b.x;
    a->y = a->y + b.y;
    a->z = a->z + b.z;
}

void add_v4_in_place(v4* a, v4 b)
{    
    a->x = a->x + b.x;
    a->y = a->y + b.y;
    a->z = a->z + b.z;
    a->w = a->w + b.w;
}

v2 minus_v2(v2 a)
{
    v2 result = {0};

    result.x = -a.x;
    result.y = -a.y;

    return result;
}

v3 minus_v3(v3 a)
{
    v3 result = {0};

    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;

    return result;
}

v4 minus_v4(v4 a)
{
    v4 result = {0};

    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;

    return result;
}

v2 subtract_v2(v2 a, v2 b)
{
    v2 result = {0};

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

v3 subtract_v3(v3 a, v3 b)
{
    v3 result = {0};

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;

    return result;
}

v4 subtract_v4(v4 a, v4 b)
{
    v4 result = {0};

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;

    return result;
}

v2 multiply_v2(v2 a, r32 b)
{
    v2 result = {0};

    result.x = b * a.x;
    result.y = b * a.y;

    return result;
}

v3 multiply_v3(v3 a, r32 b)
{
    v3 result = {0};

    result.x = b * a.x;
    result.y = b * a.y;
    result.z = b * a.z;

    return result;
}

v4 multiply_v4(v4 a, r32 b)
{
    v4 result = {0};

    result.x = b * a.x;
    result.y = b * a.y;
    result.z = b * a.z;
    result.w = b * a.w;

    return result;
}

v2 divide_v2(v2 a, r32 b)
{
    v2 result = {0};

    result.x = a.x / b;
    result.y = a.y / b;

    return result;
}

v3 divide_v3(v3 a, r32 b)
{
    v3 result = {0};

    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;

    return result;
}

v4 divide_v4(v4 a, r32 b)
{
    v4 result = {0};

    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;
    result.w = a.w / b;

    return result;
}

r32 inner_v2(v2 a, v2 b)
{
    r32 result = (a.x * b.x) + (a.y * b.y);
    return result;
}

r32 inner_v3(v3 a, v3 b)
{
    r32 result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    return result;
}

r32 inner_v4(v4 a, v4 b)
{
    r32 result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
    return result;
}

r32 length_squared_v2(v2 a)
{
    r32 result = inner_v2(a, a);
    return result;
}

r32 length_squared_v3(v3 a)
{
    r32 result = inner_v3(a, a);
    return result;
}

r32 length_squared_v4(v4 a)
{
    r32 result = inner_v4(a, a);
    return result;
}

r32 length_v2(v2 a)
{
    r32 result = length_squared_v2(a);
    result = SDL_sqrt(result);
    return result;
}

r32 length_v3(v3 a)
{
    r32 result = length_squared_v3(a);
    result = SDL_sqrt(result);
    return result;
}

r32 length_v4(v4 a)
{
    r32 result = length_squared_v4(a);
    result = SDL_sqrt(result);
    return result;
}

b32 is_zero_v2(v2 a)
{
    b32 result = 0;
    result = (a.x == 0.0f && a.y == 0.0f);
    return result;
}

b32 is_zero_v3(v3 a)
{
    b32 result = 0;
    result = (a.x == 0.0f && a.y == 0.0f && a.z == 0.0f);
    return result;
}

b32 is_zero_v4(v4 a)
{
    b32 result = 0;
    result = (a.x == 0.0f && a.y == 0.0f && a.z == 0.0f && a.w == 0.0f);
    return result;
}

v2 get_zero_v2()
{
    v2 result = {0};
    return result;
}

v3 get_zero_v3()
{
    v3 result = {0};
    return result;
}

v4 get_zero_v4()
{
    v4 result = {0};
    return result;
}

v2 get_unit_v2(v2 a)
{
    v2 result = divide_v2(a, length_v2(a));
    return result;
}

v3 get_unit_v3(v3 a)
{
    v3 result = divide_v3(a, length_v3(a));
    return result;
}

v4 get_unit_v4(v4 a)
{
    v4 result = divide_v4(a, length_v4(a));
    return result;
}
//
//v2 scale_v2(v2 a, r32 new_length)
//{
//    r32 ratio = new_length / SDL_sqrt(length(a));
//    v2 result = a * ratio;
//    return result;
//}
//
//v3 scale(v3 a, r32 new_length)
//{
//    r32 ratio = new_length / SDL_sqrt(length(a));
//    v3 result = a * ratio;
//    return result;
//}
//
//v4 scale(v4 a, r32 new_length)
//{
//    r32 ratio = new_length / SDL_sqrt(length(a));
//    v4 result = a * ratio;
//    return result;
//}

//v2 hadamard(v2 a, v2 b)
//{
//    v2 result = {0};
//    result.x = a.x * b.x;
//    result.y = a.y * b.y;
//    return result;
//}
//
//v3 hadamard(v3 a, v3 b)
//{
//    v3 result = {0};
//    result.x = a.x * b.x;
//    result.y = a.y * b.y;
//    result.z = a.z * b.z;
//    return result;
//}
//
//v4 hadamard(v4 a, v4 b)
//{
//    v4 result = {0};
//    result.x = a.x * b.x;
//    result.y = a.y * b.y;
//    result.z = a.z * b.z;
//    result.w = a.w * b.w;
//    return result;
//}
//
//v2 normalize_v2(v2 a)
//{
//    v2 result = a / length_v2(a);
//    return result;
//}
//
//v3 normalize_v3(v3 a)
//{
//    v3 result = a / length_v3(a);
//    return result;
//}
//
//v4 normalize_v4(v4 a)
//{
//    v4 result = a / length_v4(a);
//    return result;
//}

v2 get_rotated_unit_v2(r32 angle, b32 radians)
{
    v2 result = {0};
    if (!radians)
    {
        angle = (angle * (pi32 / 180.0f));
    }
    result.x = cos(angle);
    result.y = sin(angle);
    return result;
}

v2 rotate_v2(v2 vec, r32 angle, b32 radians)
{
    v2 result = {0};
    if (!radians)
    {
        angle = (angle * (pi32 / 180.0f));
    }
    result.x = (cos(angle) * vec.x) - (sin(angle) * vec.y);
    result.y = (sin(angle) * vec.x) + (cos(angle)* vec.y);
    return result;
}

v2 reflection_over_x_axis_v2(v2 a)
{
    v2 result = get_v2(a.x, -a.y);
    return result;
}

v2 reflection_over_y_axis_v2(v2 a)
{
    v2 result = get_v2(-a.x, a.y);
    return result;
}

// arytmetyka

r32 square_r32(r32 a)
{
    r32 result = a * a;
    return result;
}

i32 square_i32(i32 a)
{
    i32 result = a * a;
    return result;
}

b32 is_r32_nan(r32 a)
{
    b32 result = (a != a);
    return result;
}

// prostokąty

v2 get_rect_min_corner(rect rect)
{
    v2 result = rect.min_corner;
    return result;
}

v2 get_rect_max_corner(rect rect)
{
    v2 result = rect.max_corner;
    return result;
}

v2 get_rect_center(rect rect)
{
    v2 result = multiply_v2(add_v2(rect.min_corner, rect.max_corner), 0.5f);
    return result;
}

rect get_rect_from_corners(v2 min_corner, v2 max_corner)
{
    rect result = {0};
    result.min_corner = min_corner;
    result.max_corner = max_corner;
    return result;
}

rect get_rect_from_min_corner(v2 min_corner, v2 dimensions)
{
    rect result = {0};
    result.min_corner = min_corner;
    result.max_corner = add_v2(min_corner, dimensions);
    return result;
}

rect get_rect_from_center_and_dimensions(v2 center, v2 dimensions)
{
    rect result = {0};
    result.min_corner = get_v2(center.x - (dimensions.x / 2), center.y - (dimensions.y / 2));
    result.max_corner = get_v2(center.x + (dimensions.x / 2), center.y + (dimensions.y / 2));
    return result;
}

rect get_rect_from_center_and_half_side(v2 center, r32 half_side)
{
    rect result = {0};
    result.min_corner = get_v2(center.x - half_side, center.y - half_side);
    result.max_corner = get_v2(center.x + half_side, center.y + half_side);
    return result;
}

b32 is_point_inside_rect(rect rectangle, v2 point)
{
    b32 result = 0;
    result = (point.x >= rectangle.min_corner.x
        && point.y >= rectangle.min_corner.y
        && point.x < rectangle.max_corner.x
        && point.y < rectangle.max_corner.y);
    return result;
}

rect add_side_length(rect rectangle, v2 sides_length)
{
    rect result = {0};

    r32 half_x = sides_length.x / 2.0f;
    r32 half_y = sides_length.y / 2.0f;

    result.min_corner.x = rectangle.min_corner.x - half_x;
    result.min_corner.y = rectangle.min_corner.y - half_y;

    result.max_corner.x = rectangle.max_corner.x + half_x;
    result.max_corner.y = rectangle.max_corner.y + half_y;

    return result;
}

rect move_rect(rect rect_to_move, v2 distance)
{
    rect result = {0};
    result.min_corner = add_v2(rect_to_move.min_corner, distance);
    result.max_corner = add_v2(rect_to_move.max_corner, distance);
    return result;
}

rect move_rect_in_place(rect* rect_to_move, v2 distance)
{
    rect_to_move->min_corner = add_v2(rect_to_move->min_corner, distance);
    rect_to_move->max_corner = add_v2(rect_to_move->max_corner, distance);
    return *rect_to_move;
}

v2 get_rect_dimensions(rect rect)
{
    v2 result = {0};

    result.x = rect.max_corner.x - rect.min_corner.x;
    result.y = rect.max_corner.y - rect.min_corner.y;

    return result;
}

b32 are_intersecting(rect a, rect b)
{
    // sprawdzamy, czy odcinki na osi przecinają się
    b32 x_axis_intersect = !(b.max_corner.x <= a.min_corner.x || b.min_corner.x >= a.max_corner.x);
    b32 y_axis_intersect = !(b.max_corner.y <= a.min_corner.y || b.min_corner.y >= a.max_corner.y);
    b32 result = (x_axis_intersect && y_axis_intersect);
    return result;
}

r32 safe_ratio_n(r32 numerator, r32 denominator, r32 value_if_denominator_is_zero)
{
    r32 result = value_if_denominator_is_zero;

    if (denominator != 0)
    {
        result = numerator / denominator;
    }

    return result;
}

r32 safe_ratio_1(r32 numerator, r32 denominator)
{
    r32 result = safe_ratio_n(numerator, denominator, 1.0f);
    return result;
}

r32 safe_ratio_0(r32 numerator, r32 denominator)
{
    r32 result = safe_ratio_n(numerator, denominator, 0.0f);
    return result;
}

v2 get_normal_coordinates_relative_to_rect(v2 world_coords, rect rect)
{
    r32 new_x = safe_ratio_0((world_coords.x - rect.min_corner.x), (rect.max_corner.x - rect.min_corner.x));
    r32 new_y = safe_ratio_0((world_coords.y - rect.min_corner.y), (rect.max_corner.y - rect.min_corner.y));
    v2 result = get_v2(new_x, new_y);
    return result;
}
//
//r32 clamp(r32 min, r32 value, r32 max)
//{
//    r32 result = value;
//
//    if (result < min)
//    {
//        result = min;
//    }
//
//    if (result > max)
//    {
//        result = max;
//    }
//
//    return result;
//}
//
//r32 clamp_01(r32 value)
//{
//    r32 result = clamp(0.0f, value, 1.0f);
//    return result;
//}
//
//v2 clamp_01(v2 vec)
//{
//    v2 result = {0};
//    result.x = clamp_01(vec.x);
//    result.y = clamp_01(vec.y);
//    return result;
//}
//
//v3 clamp_01(v3 vec)
//{
//    v3 result = {0};
//    result.x = clamp_01(vec.x);
//    result.y = clamp_01(vec.y);
//    result.z = clamp_01(vec.z);
//    return result;
//}
//
//v4 clamp_01(v4 vec)
//{
//    v4 result = {0};
//    result.x = clamp_01(vec.x);
//    result.y = clamp_01(vec.y);
//    result.z = clamp_01(vec.z);
//    result.w = clamp_01(vec.w);
//    return result;
//}
//
//r32 lerp(r32 a, r32 perc, r32 b)
//{
//    r32 result = a + ((b - a) * perc);
//    //alternatywnie: r32 result = (1.0f - perc) * a + (perc * b);
//    return result;
//}
//
//v2 lerp(v2 a, r32 perc, v2 b)
//{
//    v2 result = a + ((b - a) * perc);
//    return result;
//}
//
//v3 lerp(v3 a, r32 perc, v3 b)
//{
//    v3 result = a + ((b - a) * perc);
//    return result;
//}
//
//v4 lerp(v4 a, r32 perc, v4 b)
//{
//    v4 result = a + ((b - a) * perc);
//    return result;
//}
//
//// od perpendicular - prostopadły wektor
//v2 perp(v2 a)
//{
//    v2 result = get_v2(-a.y, a.x);
//    return result;
//};