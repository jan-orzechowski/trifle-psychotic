#if TRIFLE_DEBUG

#include "main.h"
#include "entities.h"
#include "map.h"
#include "collision.h"
#include "rendering.h"
#include "text_rendering.h"
#include "ui.h"
#include "player.h"

void debug_render_tile(render_group* render, tile_position tile_pos, v4 color, world_position camera_pos)
{
	v2 position = get_position_difference(tile_pos, camera_pos);
	rect screen_rect = get_tile_screen_rect(position);
	render_rectangle(render, screen_rect, color, false);
}

void debug_render_player_information(game_state* game, level_state* level)
{
	entity* player = get_player(level);

	collision_result collision = {};
	b32 is_standing = is_standing_on_ground(level, player, &collision);

	b32 is_standing_on_platform = false;
	if (collision.collided_platform)
	{
		is_standing_on_platform = true;
	}

	char buffer[200];
	v4 text_color = get_v4(1, 1, 1, 0);
	int error = snprintf(buffer, 200, "Chunk:(%d,%d),Pos:(%0.2f,%0.2f),Acc: (%0.2f,%0.2f) Standing: %d, Platform: %d, Direction: %s",
		player->position.chunk_pos.x, player->position.chunk_pos.y, player->position.pos_in_chunk.x, player->position.pos_in_chunk.y,
		player->acceleration.x, player->acceleration.y, is_standing, is_standing_on_platform, (player->direction == direction::W ? "W" : "E"));

	rect area = get_rect_from_corners(
		get_v2(10, 200), get_v2((SCREEN_WIDTH / 2) - 10, 260)
	);

	string_ref debug_str = {};
	debug_str.ptr = buffer;
	debug_str.string_size = 200;

	render_text(&game->render, game->transient_arena,
		game->level_state->static_data->ui_font, area, debug_str, true);
}

void debug_render_tile_collision_boxes(render_group* render, level_state* level, world_position camera_pos)
{
	i32 screen_half_width = ceil(HALF_SCREEN_WIDTH_IN_TILES) + 2;
	i32 screen_half_height = ceil(HALF_SCREEN_HEIGHT_IN_TILES) + 2;
	tile_position camera_tile_pos = get_tile_position(camera_pos);

	for (i32 y_coord_relative = -screen_half_height;
		y_coord_relative < screen_half_height;
		y_coord_relative++)
	{
		i32 y_coord_on_screen = y_coord_relative;
		i32 y_coord_in_world = camera_tile_pos.y + y_coord_relative;

		for (i32 x_coord_relative = -screen_half_width;
			x_coord_relative < screen_half_width;
			x_coord_relative++)
		{
			i32 x_coord_on_screen = x_coord_relative;
			i32 x_coord_in_world = camera_tile_pos.x + x_coord_relative;

			u32 tile_value = get_tile_value(&level->current_map, x_coord_in_world, y_coord_in_world);
			if (is_tile_colliding(&level->current_map, tile_value))
			{
				tile_position tile_pos = get_tile_position(x_coord_in_world, y_coord_in_world);
				entity_collision_data tile_collision = get_tile_collision_data(camera_pos.chunk_pos, tile_pos);
				v2 relative_position = get_position_difference(tile_pos, camera_pos);
				v2 center = relative_position + tile_collision.collision_rect_offset;
				v2 size = tile_collision.collision_rect_dim;
				rect collision_rect = get_rect_from_center(
					SCREEN_CENTER_IN_PIXELS + (center * TILE_SIDE_IN_PIXELS),
					(size * TILE_SIDE_IN_PIXELS));

				render_rectangle(render, collision_rect, get_zero_v4(), true);
			}
		}
	}
}

void debug_render_entity_collision_boxes(render_group* render, level_state* level, world_position camera_pos)
{
	for (i32 entity_index = 0; entity_index < level->entities_count; entity_index++)
	{
		entity* entity = level->entities + entity_index;
		if (false == entity->used)
		{
			continue;
		}

		if (is_in_neighbouring_chunk(camera_pos.chunk_pos, entity->position))
		{
			// istotne - offset sprite'a nie ma tu znaczenia
			v2 relative_position = get_position_difference(entity->position, camera_pos);
			v2 center = relative_position + entity->type->collision_rect_offset;
			v2 size = entity->type->collision_rect_dim;
			rect collision_rect = get_rect_from_center(
				SCREEN_CENTER_IN_PIXELS + (center * TILE_SIDE_IN_PIXELS),
				size * TILE_SIDE_IN_PIXELS);

			render_rectangle(render, collision_rect, { 0, 0, 0, 0 }, true);

			v2 entity_position = SCREEN_CENTER_IN_PIXELS + relative_position * TILE_SIDE_IN_PIXELS;
			render_point(render, entity_position, get_v4(1, 0, 0, 0));
		}
	}
}

void debug_render_bullet_collision_boxes(render_group* render, level_state* level, world_position camera_pos)
{
	for (i32 bullet_index = 0; bullet_index < level->bullets_count; bullet_index++)
	{
		bullet* bullet = level->bullets + bullet_index;
		v2 relative_position = get_position_difference(bullet->position, camera_pos);		
		v2 entity_position = SCREEN_CENTER_IN_PIXELS + relative_position * TILE_SIDE_IN_PIXELS;
		render_point(render, entity_position, get_v4(1, 0, 0, 0));		
	}
}

#endif