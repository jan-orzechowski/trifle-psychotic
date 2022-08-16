#pragma once

#include "main.h"

void parse_tilemap(read_file_result);

enum tmx_node_type // już wszystkie
{
	TMX_NODE_OTHER,
	TMX_NODE_MAP,
	TMX_NODE_TILESET,
	TMX_NODE_LAYER,
	TMX_NODE_DATA,
	TMX_NODE_OBJECTGROUP,
	TMX_NODE_OBJECT,
};

enum tmx_attr_type // do uzupełnienia
{
	TMX_ATTR_TYPE_UNKNOWN,
	TMX_ATTR_ID,
	TMX_ATTR_NAME,
	TMX_ATTR_CLASS,
	TMX_ATTR_X,
	TMX_ATTR_Y,
	TMX_ATTR_WIDTH,
	TMX_ATTR_HEIGHT,
	TMX_ATTR_TILEWIDTH,
	TMX_ATTR_TILEHEIGHT,
	TMX_ATTR_ORIENTATION,
	TMX_ATTR_RENDERORDER,
	TMX_ATTR_SOURCE,
	TMX_ATTR_ENCODING,
};

struct tmx_attribute
{
	tmx_attr_type type;
	char* value;
};

struct tmx_node
{
	tmx_node_type type;
	tmx_attribute* attributes;
	int attributes_count;
	char* inner_text;
};