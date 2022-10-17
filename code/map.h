#pragma once

#include "jorutils.h"
#include "jormath.h"
#include "main.h"

tile_position get_tile_position(i32 tile_x, i32 tile_y);
tile_position get_tile_position(world_position world_pos);
tile_position get_tile_position_from_index(map* level, u32 tile_index);
b32 operator ==(tile_position a, tile_position b);
b32 operator !=(tile_position a, tile_position b);
chunk_position get_tile_chunk_position(tile_position tile_pos);
v2 get_tile_offset_in_chunk(chunk_position chunk_pos, tile_position tile_pos);
world_position get_world_position(tile_position tile_pos);
world_position get_world_position(chunk_position chunk_pos, v2 pos_in_chunk);
world_position get_world_position(chunk_position chunk_pos);
world_position renormalize_position(world_position world_pos);
v2 get_position_difference(tile_position a, tile_position b);
v2 get_position_difference(world_position a, world_position b);
v2 get_position_difference(world_position a, chunk_position b);
v2 get_position_difference(chunk_position a, world_position b);
v2 get_position_difference(world_position a, tile_position b);
v2 get_position_difference(tile_position a, world_position b);
v2 get_position_difference(tile_position a, chunk_position b);
world_position add_to_position(world_position pos, v2 offset);
tile_position add_to_tile_position(tile_position tile_pos, i32 x_offset, i32 y_offset);
v2 get_tile_offset_in_chunk(chunk_position chunk_pos, tile_position tile_pos);
b32 is_in_neighbouring_chunk(chunk_position reference_chunk, world_position position_to_check);
u32 get_tile_value(map_layer layer, i32 x_coord, i32 y_coord);
u32 get_tile_value(map_layer layer, tile_position tile);
u32 get_tile_value(map* level, i32 x_coord, i32 y_coord);
u32 get_tile_value(map* level, tile_position tile);
b32 is_tile_colliding(map* level, u32 tile_value);
b32 is_tile_colliding(map* level, u32 tile_x, u32 tile_y);
b32 is_tile_colliding(map* level, tile_position tile_pos);
tile_range get_tile_range(tile_position start, tile_position end);
tile_range find_horizontal_range_of_free_tiles(map* level, tile_position starting_tile, u32 length_limit);
tile_range find_vertical_range_of_free_tiles(map* level, tile_position starting_tile, u32 length_limit);
tile_range find_vertical_range_of_free_tiles_downwards(map* level, tile_position starting_tile, u32 length_limit);
b32 is_good_for_walk_path(map* level, u32 tile_x, u32 tile_y);
tile_position get_closest_end_from_tile_range(tile_range range, world_position position);
tile_range get_invalid_tile_range();
b32 is_tile_range_valid(tile_range range);