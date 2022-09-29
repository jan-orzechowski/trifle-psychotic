#include "jorutils.h"
#include "jormath.h"
#include "main.h"

tile_position get_tile_position(i32 tile_x, i32 tile_y)
{
	tile_position result = {};
	result.x = tile_x;
	result.y = tile_y;
	return result;
}

tile_position get_tile_position(world_position world_pos)
{
	tile_position result = {};
	// origin pola (0,0) to punkt (0,0) - środek wypada w (0,5;0,5) itd.
	i32 tile_in_chunk_x = floor(world_pos.pos_in_chunk.x);
	i32 tile_in_chunk_y = floor(world_pos.pos_in_chunk.y);
	result.x = (world_pos.chunk_pos.x * CHUNK_SIDE_IN_TILES) + tile_in_chunk_x;
	result.y = (world_pos.chunk_pos.y * CHUNK_SIDE_IN_TILES) + tile_in_chunk_y;
	return result;
}

b32 operator ==(tile_position a, tile_position b)
{
	b32 result = (a.x == b.x && a.y == b.y);
	return result;
}

b32 operator !=(tile_position a, tile_position b)
{
	b32 result = !(a.x == b.x && a.y == b.y);
	return result;
}

chunk_position get_tile_chunk_position(tile_position tile_pos)
{
	chunk_position result = {};
	// np. (3,15) -> (0,0), (4,16) -> (1,2)
	result.x = floor((r32)(tile_pos.x + 1) / (r32)CHUNK_SIDE_IN_TILES);
	result.y = floor((r32)(tile_pos.y + 1) / (r32)CHUNK_SIDE_IN_TILES);
	return result;
}

v2 get_tile_offset_in_chunk(chunk_position chunk_pos, tile_position tile_pos);

world_position get_world_position(tile_position tile_pos)
{
	world_position result = {};
	result.chunk_pos = get_tile_chunk_position(tile_pos);
	result.pos_in_chunk = get_tile_offset_in_chunk(result.chunk_pos, tile_pos);
	return result;
}

world_position get_world_position(chunk_position chunk_pos, v2 pos_in_chunk)
{
	world_position result = {};
	result.chunk_pos = chunk_pos;
	result.pos_in_chunk = pos_in_chunk;
	return result;
}

world_position get_world_position(chunk_position chunk_pos)
{
	world_position result = {};
	result.chunk_pos = chunk_pos;
	return result;
}

world_position renormalize_position(world_position world_pos)
{
	while (world_pos.pos_in_chunk.x >= (r32)CHUNK_SIDE_IN_TILES)
	{
		world_pos.pos_in_chunk.x -= (r32)CHUNK_SIDE_IN_TILES;
		world_pos.chunk_pos.x++;
	}

	while (world_pos.pos_in_chunk.y >= (r32)CHUNK_SIDE_IN_TILES)
	{
		world_pos.pos_in_chunk.y -= (r32)CHUNK_SIDE_IN_TILES;
		world_pos.chunk_pos.y++;
	}

	while (world_pos.pos_in_chunk.x < 0)
	{
		world_pos.pos_in_chunk.x = (r32)CHUNK_SIDE_IN_TILES + world_pos.pos_in_chunk.x;
		world_pos.chunk_pos.x--;
	}

	while (world_pos.pos_in_chunk.y < 0)
	{
		world_pos.pos_in_chunk.y = (r32)CHUNK_SIDE_IN_TILES + world_pos.pos_in_chunk.y;
		world_pos.chunk_pos.y--;
	}

	return world_pos;
}

v2 get_position_difference(tile_position a, tile_position b)
{
	// +0,5, -0,5 kasują się
	v2 result = get_v2((r32)(a.x - b.x), (r32)(a.y - b.y));
	return result;
}

v2 get_position_difference(world_position a, world_position b)
{
	v2 result =
		a.pos_in_chunk
		+ get_v2((a.chunk_pos.x - b.chunk_pos.x) * CHUNK_SIDE_IN_TILES,
			(a.chunk_pos.y - b.chunk_pos.y) * CHUNK_SIDE_IN_TILES)
		- b.pos_in_chunk;
	return result;
}

v2 get_position_difference(world_position a, chunk_position b)
{
	v2 result = get_position_difference(a, get_world_position(b));
	return result;
}

v2 get_position_difference(chunk_position a, world_position b)
{
	v2 result = get_position_difference(get_world_position(a), b);
	return result;
}

v2 get_position_difference(world_position a, tile_position b)
{
	v2 result = get_position_difference(a, get_world_position(b));
	return result;
}

v2 get_position_difference(tile_position a, world_position b)
{
	v2 result = get_position_difference(get_world_position(a), b);
	return result;
}

v2 get_position_difference(tile_position a, chunk_position b)
{
	v2 result = get_position_difference(get_world_position(a), get_world_position(b));
	return result;
}

world_position add_to_position(world_position pos, v2 offset)
{
	pos.pos_in_chunk += offset;
	pos = renormalize_position(pos);
	return pos;
}

v2 get_tile_offset_in_chunk(chunk_position chunk_pos, tile_position tile_pos)
{
	tile_position chunk_origin_tile = get_tile_position(chunk_pos.x * CHUNK_SIDE_IN_TILES, chunk_pos.y * CHUNK_SIDE_IN_TILES);
	v2 position_difference = get_position_difference(get_tile_position(tile_pos.x, tile_pos.y), chunk_origin_tile);
	return position_difference;
}

b32 is_in_neighbouring_chunk(chunk_position reference_chunk, world_position position_to_check)
{
	b32 result = (position_to_check.chunk_pos.x >= reference_chunk.x - 1
		&& position_to_check.chunk_pos.x <= reference_chunk.x + 1
		&& position_to_check.chunk_pos.y >= reference_chunk.y - 1
		&& position_to_check.chunk_pos.y <= reference_chunk.y + 1);
	return result;
}

u32 get_tile_value(map level, i32 x_coord, i32 y_coord)
{
	u32 result = 0;
	if (x_coord >= 0
		&& y_coord >= 0
		&& x_coord < (i32)level.width
		&& y_coord < (i32)level.height)
	{
		u32 tile_index = x_coord + (level.width * y_coord);
		assert(tile_index < level.tiles_count);
		result = level.tiles[tile_index];
	}
	return result;
}

u32 get_tile_value(map level, tile_position tile)
{
	u32 result = get_tile_value(level, tile.x, tile.y);
	return result;
}

b32 is_tile_colliding(map collision_ref, u32 tile_value)
{
	b32 result = false;
	if (tile_value == 0)
	{
		result = true;
	}
	else
	{
		u32 x_coord = (tile_value - 1) % collision_ref.width;
		u32 y_coord = (tile_value - 1) / collision_ref.height;
		u32 collision_tile_value = get_tile_value(collision_ref, x_coord, y_coord);
		result = (collision_tile_value == 2);
	}

	return result;
}

b32 is_tile_colliding(map level, map collision_ref, u32 tile_x, u32 tile_y)
{
	u32 tile_value = get_tile_value(level, tile_x, tile_y);
	b32 result = is_tile_colliding(collision_ref, tile_value);
	return result;
}

v2 get_length_from_tile_range(tile_range path)
{
	v2 result = {};
	v2 distance = get_position_difference(path.end, path.start);
	if (distance.x > 0)
	{
		// pozioma
		result.x = distance.x + 1.0f;
		result.y = 1.0f;
	}
	else
	{
		// pionowa
		result.x = 1.0f;
		result.y = distance.y + 1.0f;
	}
	return result;
}

tile_range find_horizontal_range_of_free_tiles(map level, map collision_ref, tile_position starting_tile, u32 length_limit)
{
	tile_position left_end = starting_tile;
	tile_position right_end = starting_tile;
	tile_position test_tile = starting_tile;
	for (i32 distance = 0; distance <= length_limit; distance++)
	{
		test_tile.x = starting_tile.x - distance;
		if (false == is_tile_colliding(level, collision_ref, test_tile.x, test_tile.y))
		{
			left_end = test_tile;
		}
		else
		{
			break;
		}
	}

	test_tile = starting_tile;
	for (i32 distance = 0; distance <= length_limit; distance++)
	{
		test_tile.x = starting_tile.x + distance;
		if (false == is_tile_colliding(level, collision_ref, test_tile.x, test_tile.y))
		{
			right_end = test_tile;
		}
		else
		{
			break;
		}
	}

	tile_range result = {};
	result.start = left_end;
	result.end = right_end;
	return result;
}

tile_range find_vertical_range_of_free_tiles(map level, map collision_ref, tile_position starting_tile, u32 length_limit)
{
	tile_position upper_end = starting_tile;
	tile_position lower_end = starting_tile;
	tile_position test_tile = starting_tile;
	for (i32 distance = 0; distance <= length_limit; distance++)
	{
		test_tile.y = starting_tile.y - distance;
		if (false == is_tile_colliding(level, collision_ref, test_tile.x, test_tile.y))
		{
			upper_end = test_tile;
		}
		else
		{
			break;
		}
	}

	test_tile = starting_tile;
	for (i32 distance = 0; distance <= length_limit; distance++)
	{
		test_tile.y = starting_tile.y + distance;
		if (false == is_tile_colliding(level, collision_ref, test_tile.x, test_tile.y))
		{
			lower_end = test_tile;
		}
		else
		{
			break;
		}
	}

	tile_range result = {};
	result.start = upper_end;
	result.end = lower_end;
	return result;
}

b32 is_good_for_walk_path(map level, map collision_ref, u32 tile_x, u32 tile_y)
{
	b32 result = (false == is_tile_colliding(level, collision_ref, tile_x, tile_y)
		&& is_tile_colliding(level, collision_ref, tile_x, tile_y + 1));
	return result;
}

tile_position get_closest_end_from_tile_range(tile_range range, world_position position)
{
	r32 end_distance = length(get_position_difference(range.end, position));
	r32 start_distance = length(get_position_difference(range.start, position));
	tile_position result = end_distance < start_distance ? range.end : range.start;
	return result;
}