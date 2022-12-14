#pragma once

#define TILESET_WIDTH_IN_TILES 64
#define CHARSET_WIDTH 192
#define TILE_SIDE_IN_PIXELS 16

#define SCALING_FACTOR 2

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SCREEN_WIDTH_IN_TILES (SCREEN_WIDTH / (TILE_SIDE_IN_PIXELS * SCALING_FACTOR))
#define SCREEN_HEIGHT_IN_TILES (SCREEN_HEIGHT / (TILE_SIDE_IN_PIXELS * SCALING_FACTOR))

#define HALF_SCREEN_WIDTH_IN_TILES ((SCREEN_WIDTH_IN_TILES / 2) + 1)
#define HALF_SCREEN_HEIGHT_IN_TILES ((SCREEN_HEIGHT_IN_TILES / 2) + 1)

#define SCREEN_CENTER_IN_TILES get_v2(HALF_SCREEN_WIDTH_IN_TILES, HALF_SCREEN_HEIGHT_IN_TILES)
#define SCREEN_CENTER_IN_PIXELS get_v2((SCREEN_WIDTH / SCALING_FACTOR) / 2, (SCREEN_HEIGHT / SCALING_FACTOR) / 2)

#define CHUNK_SIDE_IN_TILES 16

#define MAX_LEVEL_NAME_LENGTH 100

#define ENTITY_STATIC_TYPES_MAX_COUNT 15
#define ENTITY_DYNAMIC_TYPES_MAX_COUNT 200
#define BULLET_TYPES_MAX_COUNT 10
#define MAX_ENTITIES_COUNT 1000
#define MAX_MAP_SIDE_SIZE 500
#define MAX_GATES_COUNT 100
#define MAX_BULLETS_COUNT 1000 /* in offical game levels measured max is about 700 */
#define MAX_EXPLOSIONS_COUNT 200 /* measured max is about 150 */
