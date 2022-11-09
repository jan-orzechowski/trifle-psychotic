#include "main.h"
#include "rendering.h"
#include "map.h"

void render_static_repeated_backdrop(render_list* render, backdrop_properties backdrop)
{
    assert(backdrop.texture != TEXTURE_NONE);
    assert(false == is_zero_v2(backdrop.size));

    i32 size_y = (i32)backdrop.size.y;
    i32 size_x = (i32)backdrop.size.x;
    for (i32 y = -size_y; y < SCREEN_HEIGHT + size_y; y += size_y)
    {
        for (i32 x = -size_x; x < SCREEN_WIDTH + size_x; x += size_x)
        {
            render_bitmap(render, backdrop.texture,
                get_rect_from_corners(get_zero_v2(), backdrop.size),
                get_rect_from_min_corner(get_v2(x, y), backdrop.size));
        }
    }
}

void render_scrolling_repeated_backdrop(render_list* render, backdrop_properties backdrop,
    v2 offset_in_tiles, b32 repeat_only_horizontally, world_position camera_position)
{
    assert(backdrop.texture != TEXTURE_NONE);
    assert(false == is_zero_v2(backdrop.size));

    v2 backdrop_size_in_tiles = scalar_divide_v2(backdrop.size, TILE_SIDE_IN_PIXELS);
    if (backdrop.x_slowdown != 0)
    {
        backdrop_size_in_tiles.x *= backdrop.x_slowdown;
    }
    if (backdrop.y_slowdown != 0)
    {
        backdrop_size_in_tiles.y *= backdrop.y_slowdown;
    }

    tile_position camera_tile_pos = get_tile_pos_from_world_pos(camera_position);
    tile_position backdrop_origin_tile_pos = get_tile_pos(
        camera_tile_pos.x - (camera_tile_pos.x % (i32)backdrop_size_in_tiles.x),
        camera_tile_pos.y - (camera_tile_pos.y % (i32)backdrop_size_in_tiles.y));

    if (repeat_only_horizontally)
    {
        backdrop_origin_tile_pos.y = 0;
    }

    v2 backdrop_offset = add_v2(get_world_pos_and_tile_position_difference(camera_position, backdrop_origin_tile_pos),
        offset_in_tiles);

    if (backdrop.x_slowdown != 0)
    {
        backdrop_offset.x /= backdrop.x_slowdown;
    }
    if (backdrop.y_slowdown != 0)
    {
        backdrop_offset.y /= backdrop.y_slowdown;
    }

    if (repeat_only_horizontally)
    {
        r32 max_y_offset = (backdrop.size.y - (SCREEN_HEIGHT / SCALING_FACTOR)) / TILE_SIDE_IN_PIXELS;
        if (backdrop_offset.y > max_y_offset)
        {
            backdrop_offset.y = max_y_offset;
        }
        if (backdrop_offset.y < 0)
        {
            backdrop_offset.y = 0;
        }
    }

    i32 size_y = (i32)backdrop.size.y;
    i32 size_x = (i32)backdrop.size.x;

    i32 starting_y = -(backdrop_offset.y * TILE_SIDE_IN_PIXELS);
    i32 starting_x = -(backdrop_offset.x * TILE_SIDE_IN_PIXELS);
    if (starting_x > 0)
    {
        starting_x -= size_x;
    }
    if (starting_y > 0)
    {
        starting_y -= size_y;
    }

    for (i32 y = starting_y; y < SCREEN_HEIGHT; y += size_y)
    {
        for (i32 x = starting_x; x < SCREEN_WIDTH; x += size_x)
        {
            render_bitmap(render, backdrop.texture,
                get_rect_from_corners(get_zero_v2(), backdrop.size),
                get_rect_from_min_corner(get_v2(x, y), backdrop.size));
        }
    }
}

void update_backdrops_movement(backdrop_properties* backdrop, v2* backdrop_offset, entity* player, r32 delta_time)
{
    if (backdrop->texture != TEXTURE_NONE)
    {
        backdrop_offset->x += (backdrop->x_speed * delta_time);
        backdrop_offset->y += (backdrop->y_speed * delta_time);

        // w przypadku ruchu gracza w przeciwnym kierunku dodajemy dodatkowe przesunięcie, żeby ruchy nie zniwelowały się
        // i nie powstało wrażenie, że tło jest statyczne - wygląda to lepiej z perspektywy gracza
        v2 player_moved_distance = scalar_multiply_v2(scalar_multiply_v2(player->velocity, player->type->velocity_multiplier), delta_time);
        if (length_v2(player_moved_distance) > 0.01f)
        {
            if (backdrop->x_speed != 0 && sign_r32(backdrop->x_speed) != sign_r32(player_moved_distance.x))
            {
                backdrop_offset->x -= player_moved_distance.x;
            }
        }

        r32 max_x_offset = backdrop->size.x / TILE_SIDE_IN_PIXELS;
        if (backdrop_offset->x > max_x_offset)
        {
            backdrop_offset->x = 0.0f;
        }

        r32 max_y_offset = backdrop->size.y / TILE_SIDE_IN_PIXELS;
        if (backdrop_offset->y > max_y_offset)
        {
            backdrop_offset->y = 0.0f;
        }
    }
}

void render_backdrops(render_list* render, level_state* level, world_position camera_position)
{
    if (level->current_map.second_backdrop.texture != TEXTURE_NONE)
    {
        if (level->current_map.second_backdrop.x_slowdown == 0
            && level->current_map.second_backdrop.y_slowdown == 0)
        {
            render_static_repeated_backdrop(render, level->current_map.second_backdrop);
        }
        else
        {
            render_scrolling_repeated_backdrop(render, level->current_map.second_backdrop,
                level->current_map.second_backdrop_offset, true, camera_position);
        }
    }

    if (level->current_map.first_backdrop.texture != TEXTURE_NONE)
    {
        if (level->current_map.first_backdrop.x_slowdown == 0
            && level->current_map.first_backdrop.y_slowdown == 0)
        {
            render_static_repeated_backdrop(render, level->current_map.first_backdrop);
        }
        else
        {
            render_scrolling_repeated_backdrop(render, level->current_map.first_backdrop,
                level->current_map.first_backdrop_offset, true, camera_position);
        }
    }
}
