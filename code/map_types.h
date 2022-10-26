#pragma once

#include "jorutils.h"
#include "jormath.h"

struct tile_position
{
	i32 x;
	i32 y;
};

struct chunk_position
{
	i32 x;
	i32 y;
};

struct world_position
{
	chunk_position chunk_pos;
	v2 pos_in_chunk;
};

struct map_layer
{
	i32* tiles;
	u32 tiles_count;
	u32 width;
	u32 height;
};

struct tile_range
{
	tile_position start;
	tile_position end;
};

enum class direction
{
	NONE = 0,
	E = 1,
	SE = 2,
	S = 3,
	SW = 4,
	W = 5,
	NW = 6,
	N = 7,
	NE = 8
};