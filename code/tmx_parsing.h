#pragma once

#include "main.h"
#include "jorutils.h"

struct tilemap
{
	//string_ref tilemap_source;
	u32 width;
	u32 height;
	i32* tiles;
	u32 tiles_count;
};

tilemap read_level_data_from_tmx_file(memory_arena*, read_file_result);
