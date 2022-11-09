#pragma once

#include "jorutils.h"
#include "jormath.h"

typedef struct tile_position
{
    i32 x;
    i32 y;
} tile_position;

typedef struct chunk_position
{
    i32 x;
    i32 y;
} chunk_position;

typedef struct world_position
{
    chunk_position chunk_pos;
    v2 pos_in_chunk;
} world_position;

typedef struct map_layer
{
    i32* tiles;
    u32 tiles_count;
    u32 width;
    u32 height;
} map_layer;

typedef struct tile_range
{
    tile_position start;
    tile_position end;
} tile_range;

typedef enum direction
{
    DIRECTION_NONE = 0,
    DIRECTION_E = 1,
    DIRECTION_SE = 2,
    DIRECTION_S = 3,
    DIRECTION_SW = 4,
    DIRECTION_W = 5,
    DIRECTION_NW = 6,
    DIRECTION_N = 7,
    DIRECTION_NE = 8
} direction;