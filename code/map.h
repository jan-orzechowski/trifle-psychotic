#pragma once

#include "jorutils.h"
#include "jormath.h"
#include "map_types.h"

b32 tile_pos_equals(tile_position a, tile_position b);
tile_position get_tile_pos(i32 tile_x, i32 tile_y);
tile_position get_tile_pos_from_world_pos(world_position world_pos);
tile_position get_tile_pos_from_index(map* level, u32 tile_index);
chunk_position get_chunk_pos_from_tile_pos(tile_position tile_pos);
chunk_position get_chunk_pos_from_world_pos(world_position world_pos);
v2 get_tile_offset_in_chunk(chunk_position chunk_pos, tile_position tile_pos);
world_position get_world_pos_from_tile_pos(tile_position tile_pos);
world_position get_world_pos_from_offset_in_chunk(chunk_position chunk_pos, v2 offset_in_chunk);
world_position get_world_pos_from_chunk_pos(chunk_position chunk_pos);
world_position renormalize_world_position(world_position world_pos);
v2 get_tile_pos_diff(tile_position a, tile_position b);
v2 get_world_pos_diff(world_position a, world_position b);
v2 get_world_pos_and_chunk_pos_diff(world_position a, chunk_position b);
v2 get_chunk_pos_and_world_pos_diff(chunk_position a, world_position b);
v2 get_world_pos_and_tile_pos_dff(world_position a, tile_position b);
v2 get_tile_pos_and_world_pos_diff(tile_position a, world_position b);
v2 get_tile_pos_and_chunk_pos_dff(tile_position a, chunk_position b);
world_position add_to_world_pos(world_position pos, v2 offset);
tile_position add_to_tile_pos(tile_position tile_pos, i32 x_offset, i32 y_offset);
v2 get_tile_offset_in_chunk(chunk_position chunk_pos, tile_position tile_pos);
b32 is_in_neighbouring_chunk(chunk_position reference_chunk, world_position position_to_check);
u32 get_tile_value_in_layer_from_coords(map_layer layer, i32 x_coord, i32 y_coord);
u32 get_tile_value_from_coords(map* level, i32 x_coord, i32 y_coord);
u32 get_tile_value_in_layer(map_layer layer, tile_position tile);
u32 get_tile_value(map* level, tile_position tile);
b32 is_tile_value_colliding(map* level, u32 tile_value);
b32 is_tile_at_coords_colliding(map* level, u32 tile_x, u32 tile_y);
b32 is_tile_colliding(map* level, tile_position tile_pos);
tile_range get_tile_range(tile_position start, tile_position end);
tile_range find_horizontal_range_of_free_tiles(map* level, tile_position starting_tile, u32 length_limit);
tile_range find_vertical_range_of_free_tiles(map* level, tile_position starting_tile, u32 length_limit);
tile_range find_vertical_range_of_free_tiles_downwards(map* level, tile_position starting_tile, u32 length_limit);
b32 is_good_for_walk_path(map* level, u32 tile_x, u32 tile_y);
tile_position get_closest_end_from_tile_range(tile_range range, world_position position);
tile_range get_invalid_tile_range(void);
b32 is_tile_range_valid(tile_range range);