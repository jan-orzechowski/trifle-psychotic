#include "main.h"
#include "game_data.h"
#include "text_rendering.h"

b32 are_flags_set(entity_flags* flags, entity_flags flag_values_to_check)
{
	b32 result = are_flags_set((u32*)flags, (u32)flag_values_to_check);
	return result;
}

void set_flags(entity_flags* flags, entity_flags flag_values_to_check)
{
	set_flags((u32*)flags, (u32)flag_values_to_check);
}

void unset_flags(entity_flags* flags, entity_flags flag_values_to_check)
{
	unset_flags((u32*)flags, (u32)flag_values_to_check);
}

b32 are_entity_flags_set(entity* entity, entity_flags flag_values)
{
	b32 result = are_flags_set(&entity->type->flags, flag_values);
	return result;
}

b32 are_flags_set(sprite_effect_flags* flags, sprite_effect_flags flag_values_to_check)
{
	b32 result = are_flags_set((u32*)flags, (u32)flag_values_to_check);
	return result;
}

void set_flags(sprite_effect_flags* flags, sprite_effect_flags flag_values_to_check)
{
	set_flags((u32*)flags, (u32)flag_values_to_check);
}

void unset_flags(sprite_effect_flags* flags, sprite_effect_flags flag_values_to_check)
{
	unset_flags((u32*)flags, (u32)flag_values_to_check);
}

SDL_Color get_sdl_color(v4 color)
{
	SDL_Color result = { (Uint8)color.r, (Uint8)color.g, (Uint8)color.b, (Uint8)color.a };
	return result;
}

void print_sdl_error()
{
	const char* error = SDL_GetError();
	printf("SDL error: %s\n", error);
	invalid_code_path;
}

void print_sdl_image_error()
{
	const char* error = IMG_GetError();
	printf("SDL_image error: %s\n", error);
	invalid_code_path;
}

void print_sdl_ttf_error()
{
	const char* error = TTF_GetError();
	printf("SDL_ttf error: %s\n", error);
	invalid_code_path;
}

void load_image(SDL_Renderer* renderer, SDL_Texture** place_to_load, const char* file_path, b32* success)
{
	SDL_Surface* loaded_surface = IMG_Load(file_path);
	if (loaded_surface)
	{
		*place_to_load = SDL_CreateTextureFromSurface(renderer, loaded_surface);
		SDL_FreeSurface(loaded_surface);
	}
	else 
	{
		print_sdl_image_error();
		*success = false;
	}
}

SDL_Renderer* get_renderer(SDL_Window* window)
{
	SDL_Renderer* renderer = NULL;
	for (int driver_index = 0; driver_index < SDL_GetNumRenderDrivers(); ++driver_index)
	{
		SDL_RendererInfo renderer_info = {};
		SDL_GetRenderDriverInfo(driver_index, &renderer_info);
		// direct3d11 i direct3d powoduje freeze
		if (renderer_info.name == 0 || renderer_info.name != std::string("direct3d11"))
		{
			continue;
		}

		//renderer = SDL_CreateRenderer(window, driver_index, 0);
		//printf("found direct3d11\n");
		break;
	}

	if (renderer == 0)
	{
		// SDL_RENDERER_ACCELERATED powoduje na moim komputerze freeze - trzeba zbadać sprawę
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
		printf("direct3d11 not found - software renderer used\n");
	}

	return renderer;
}

sdl_game_data init_sdl()
{
	sdl_game_data sdl_game = {};
	b32 success = true;

	int init = SDL_Init(SDL_INIT_EVERYTHING);
	if (init == 0) // wg dokumentacji oznacza to sukces
	{
		if (false == SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			print_sdl_error();
		}

		sdl_game.window = SDL_CreateWindow("Trifle Psychotic",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

		if (sdl_game.window)
		{		
			sdl_game.renderer = SDL_CreateRenderer(sdl_game.window, -1, SDL_RENDERER_SOFTWARE); 
			//get_renderer(sdl_game.window);
			if (sdl_game.renderer)
			{
				SDL_RenderSetScale(sdl_game.renderer, SCALING_FACTOR, SCALING_FACTOR);
				SDL_SetRenderDrawColor(sdl_game.renderer, 0xFF, 0xFF, 0xFF, 0xFF);

				int img_flags = IMG_INIT_PNG;
				if (IMG_Init(img_flags) & img_flags)
				{
					load_image(sdl_game.renderer, &sdl_game.tileset_texture,	"gfx/tileset.png", &success);
					load_image(sdl_game.renderer, &sdl_game.player_texture,		"gfx/player.png", &success);
					load_image(sdl_game.renderer, &sdl_game.bullets_texture,	"gfx/bullets.png", &success);
					load_image(sdl_game.renderer, &sdl_game.gates_texture,		"gfx/gates.png", &success);
					load_image(sdl_game.renderer, &sdl_game.misc_texture,		"gfx/misc.png", &success);
					load_image(sdl_game.renderer, &sdl_game.ui_texture,			"gfx/interface.png", &success);
					load_image(sdl_game.renderer, &sdl_game.font_texture,	    "gfx/font.png", &success);
				}
				else
				{
					print_sdl_image_error();
					success = false;
				}

				int ttf_init = TTF_Init();
				if (ttf_init == 0) // wg dokumentacji 0 oznacza sukces
				{
					TTF_Font* font = TTF_OpenFont("gfx/font.ttf", 15);
					if (font)
					{
						sdl_game.font = font;
					}
					else
					{					
						print_sdl_ttf_error();
						success = false;
					}
				}
				else
				{
					print_sdl_ttf_error();
					success = false;
				}
			}
			else
			{
				print_sdl_error();
				success = false;
			}
		}
		else
		{
			print_sdl_error();
			success = false;
		}	
	}
	else
	{
		print_sdl_error();
		success = false;
	}

	if (success)
	{
		sdl_game.initialized = true;
		return sdl_game;
	}
	else
	{
		return {};
	}
}

SDL_Rect get_sdl_rect(rect rect)
{
	SDL_Rect result = {};
	result.x = (int)rect.min_corner.x;
	result.y = (int)rect.min_corner.y;
	result.w = (int)(rect.max_corner.x - rect.min_corner.x);
	result.h = (int)(rect.max_corner.y - rect.min_corner.y);
	return result;
}

void render_rect(sdl_game_data* sdl_game, rect rectangle)
{
	SDL_SetRenderDrawColor(sdl_game->renderer, 255, 255, 255, 0);
	SDL_RenderDrawLine(sdl_game->renderer, // dół
		rectangle.min_corner.x, rectangle.min_corner.y, rectangle.max_corner.x, rectangle.min_corner.y);
	SDL_RenderDrawLine(sdl_game->renderer, // lewa
		rectangle.min_corner.x, rectangle.min_corner.y, rectangle.min_corner.x, rectangle.max_corner.y);
	SDL_RenderDrawLine(sdl_game->renderer, // prawa
		rectangle.max_corner.x, rectangle.min_corner.y, rectangle.max_corner.x, rectangle.max_corner.y);
	SDL_RenderDrawLine(sdl_game->renderer, // góra
		rectangle.min_corner.x, rectangle.max_corner.y, rectangle.max_corner.x, rectangle.max_corner.y);
}

void render_text(sdl_game_data* sdl_game, std::string textureText, int x, int y, v4 color)
{
	SDL_Surface* text_surface = TTF_RenderText_Solid(sdl_game->font, textureText.c_str(), get_sdl_color(color));
	if (text_surface)
	{
		SDL_Texture* font_texture = SDL_CreateTextureFromSurface(sdl_game->renderer, text_surface);
		if (font_texture)
		{
			SDL_Rect dest = {};
			dest.w = text_surface->w;
			dest.h = text_surface->h;
			dest.x = x;
			dest.y = y;

			SDL_RenderCopy(sdl_game->renderer, font_texture, NULL, &dest);
			SDL_DestroyTexture(font_texture);
		}
		else
		{
			print_sdl_error();
		}

		SDL_FreeSurface(text_surface);
	}
	else
	{
		print_sdl_ttf_error();
	}
}

void render_hitpoint_bar(sdl_game_data* sdl_game, entity* player)
{
	u32 filled_health_bars = (u32)(player->health / 10);
	u32 max_health_bars = (u32)(player->type->max_health / 10);

	SDL_Rect texture_rect = {};
	texture_rect.w = 8;
	texture_rect.h = 8;
	texture_rect.x = 0;
	texture_rect.y = 0;

	SDL_Rect icon_screen_rect = {};
	icon_screen_rect.w = 8;
	icon_screen_rect.h = 8;
	icon_screen_rect.x = 10;
	icon_screen_rect.y = 10;
	SDL_RenderCopy(sdl_game->renderer, sdl_game->ui_texture, &texture_rect, &icon_screen_rect);

	/*texture_rect.x += 8;
	icon_screen_rect.y += 8;
	SDL_RenderCopy(sdl_game->renderer, sdl_game->ui_texture, &texture_rect, &icon_screen_rect);*/

	icon_screen_rect.w = 4;
	icon_screen_rect.h = 8;
	texture_rect.w = 4;
	texture_rect.h = 8;
	texture_rect.x = 4;
	texture_rect.y = 16;

	icon_screen_rect.x = 18;
	icon_screen_rect.y = 10;
	for (u32 health_bar_index = 0;
		health_bar_index < filled_health_bars;
		health_bar_index++)
	{
		icon_screen_rect.x += 4;
		SDL_RenderCopy(sdl_game->renderer, sdl_game->ui_texture, &texture_rect, &icon_screen_rect);
	}

	texture_rect.x = 0;
	texture_rect.y = 8;

	for (u32 health_bar_index = filled_health_bars;
		health_bar_index < max_health_bars;
		health_bar_index++)
	{
		icon_screen_rect.x += 4;
		SDL_RenderCopy(sdl_game->renderer, sdl_game->ui_texture, &texture_rect, &icon_screen_rect);
	}
}

r32 get_stage_tint(sprite_effect_stage* stage, r32 total_time)
{
	r32 result = 0.0f;
	if (total_time < stage->stage_duration)
	{
		if (stage->period == 0)
		{			
			result = stage->amplitude; // stała wartość
		}
		else
		{
			// używamy pi zamiast 2pi ze względu na to, że niżej mamy odbicie ujemnych wartości
			result = (stage->amplitude * 
				SDL_sinf((total_time * pi32/ stage->period) + stage->phase_shift));
		}

		if (result < 0)
		{
			result = -result;
		}		

		result += stage->vertical_shift;
	}

	if (result < 0.0f)
	{
		result = 0.0f;
	}

	if (result > 1.0f)
	{
		result = 1.0f;
	}

	return result;
}

v4 get_tint(sprite_effect* effect, r32 time)
{
	v4 result = effect->color / 255;
	if ((time >= 0.0f && time <= effect->total_duration))
	{
		b32 found = false;
		r32 tint_value = 1.0f;
		for (u32 stage_index = 0; stage_index < effect->stages_count; stage_index++)
		{
			sprite_effect_stage* stage = effect->stages + stage_index;
			if (time < stage->stage_duration)
			{
				tint_value = get_stage_tint(stage, time);
				found = true;
				//printf("time: %.02f tint: %.02f\n", time, tint_value);
				break;
			}
			else
			{
				time -= stage->stage_duration;
			}
		}

		if (found)
		{		
			result = (effect->color / 255) * tint_value;

			if (are_flags_set(&effect->flags, sprite_effect_flags::REVERSE_VALUES))
			{
				if (result.r != 0) { result.r = (1.0f - result.r);}
				if (result.g != 0) { result.g = (1.0f - result.g);}
				if (result.b != 0) { result.b = (1.0f - result.b);}				
			}
		}
	}

	return result;
}

void start_visual_effect(entity* entity, sprite_effect* effect, b32 override_current)
{
	if (entity->visual_effect == NULL || override_current)
	{
		entity->visual_effect = effect;
		entity->visual_effect_duration = 0;
	}
}

void start_visual_effect(game_data* game, entity* entity, u32 sprite_effect_index, b32 override_current)
{
	assert(sprite_effect_index < game->visual_effects_count);
	sprite_effect* effect = &game->visual_effects[sprite_effect_index];
	start_visual_effect(entity, effect, override_current);	
}

SDL_Rect get_tile_rect(u32 tile_id)
{
	SDL_Rect tile_rect = {};
	if (tile_id == 0)
	{
		
	}
	else
	{
		// id liczą się od 1, nie od zera
		u32 column = (tile_id - 1) % TILESET_WIDTH;
		u32 row = (tile_id - 1) / TILESET_WIDTH;

		u32 x = column * TILE_SIDE_IN_PIXELS;
		u32 y = row * TILE_SIDE_IN_PIXELS;

		tile_rect.x = x;
		tile_rect.y = y;
		tile_rect.w = TILE_SIDE_IN_PIXELS;
		tile_rect.h = TILE_SIDE_IN_PIXELS;
	}
	
	return tile_rect;
}

u32 get_current_animation_frame_index(animation* animation, r32 elapsed_time)
{
	b32 result = 0;
	while (elapsed_time > animation->total_duration)
	{
		elapsed_time -= animation->total_duration;
	}

	r32 time_within_frame = elapsed_time;
	for (u32 frame_index = 0; frame_index < animation->frames_count; frame_index++)
	{
		animation_frame* frame = animation->frames + frame_index;
		if (time_within_frame > frame->duration)
		{
			time_within_frame -= frame->duration;
			continue;
		}
		else
		{
			result = frame_index;
			break;
		}
	}

	return result;
}

animation_frame* get_current_animation_frame(entity* entity)
{
	animation_frame* result = NULL; 
	if (entity->current_animation 
		&& entity->current_animation->current_frame_index < entity->current_animation->frames_count)
	{
		result = &entity->current_animation->frames[entity->current_animation->current_frame_index];
	}
	return result;
}

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
	i32 tile_in_chunk_x = SDL_floor(world_pos.pos_in_chunk.x);
	i32 tile_in_chunk_y = SDL_floor(world_pos.pos_in_chunk.y);
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
	result.x = SDL_floor((tile_pos.x + 1) / CHUNK_SIDE_IN_TILES);
	result.y = SDL_floor((tile_pos.y + 1) / CHUNK_SIDE_IN_TILES);
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

u32 get_tile_value(level map, i32 x_coord, i32 y_coord)
{
	u32 result = 0;
	if (x_coord >= 0
		&& y_coord >= 0
		&& x_coord < (i32)map.width
		&& y_coord < (i32)map.height)
	{
		u32 tile_index = x_coord + (map.width * y_coord);
		result = map.tiles[tile_index];
	}
	return result;
}

u32 get_tile_value(level map, tile_position tile)
{
	u32 result = get_tile_value(map, tile.x, tile.y);
	return result;
}

b32 is_tile_colliding(level collision_ref, u32 tile_value)
{
	u32 x_coord = (tile_value - 1) % collision_ref.width;
	u32 y_coord = (tile_value - 1) / collision_ref.width;

	debug_breakpoint;

	b32 collides = false;
	u32 collision_tile_value = get_tile_value(collision_ref, x_coord, y_coord);
	//u32 first_gid = 1; // tymczasowe, potrzebna jest obsługa GID w tmx
	//collision_tile_value -= first_gid;
	//collision_tile_value++;

	switch (collision_tile_value)
	{
		case 2:
		case 3:
		{
			collides = true;
		}
		break;
	}
	return collides;
}

b32 is_tile_colliding(level map, level collision_ref, u32 tile_x, u32 tile_y)
{
	u32 tile_value = get_tile_value(map, tile_x, tile_y);
	b32 result = is_tile_colliding(collision_ref, tile_value);
	return result;
}

b32 is_good_for_walk_path(level map, level collision_ref, u32 tile_x, u32 tile_y)
{
	b32 result = (false == is_tile_colliding(map, collision_ref, tile_x, tile_y)
		&& is_tile_colliding(map, collision_ref, tile_x, tile_y + 1));
	return result;
}

tile_range find_walking_path_for_enemy(level map, level collision_ref, tile_position start_tile)
{
	b32 found_good_start_pos = false;
	tile_position good_start_tile = {};

	tile_position test_tile = start_tile;
	i32 distance_checking_limit = 10;
	for (i32 distance = 0; distance < distance_checking_limit; distance++)
	{
		test_tile.y = start_tile.y + distance;
		if (is_good_for_walk_path(map, collision_ref, test_tile.x, test_tile.y))
		{
			good_start_tile = test_tile;
			found_good_start_pos = true;
			break;
		}
	}

	if (good_start_tile.x == 0 && good_start_tile.y == 0)
	{
		return {};
	}

	tile_position left_end = good_start_tile;
	tile_position right_end = good_start_tile;

	test_tile = good_start_tile;
	for (i32 distance = 0; distance <= distance_checking_limit; distance++)
	{
		test_tile.x = good_start_tile.x - distance;
		if (is_good_for_walk_path(map, collision_ref, test_tile.x, test_tile.y))
		{
			left_end = test_tile;
		}
		else
		{
			break;
		}
	}

	test_tile = good_start_tile;
	for (i32 distance = 0; distance <= distance_checking_limit; distance++)
	{
		test_tile.x = good_start_tile.x + distance;
		if (is_good_for_walk_path(map, collision_ref, test_tile.x, test_tile.y))
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

tile_range find_horizontal_range_of_free_tiles(level map, level collision_ref, tile_position starting_tile, u32 length_limit)
{
	tile_position left_end = starting_tile;
	tile_position right_end = starting_tile;
	tile_position test_tile = starting_tile;
	for (i32 distance = 0; distance <= length_limit; distance++)
	{
		test_tile.x = starting_tile.x - distance;
		if (false == is_tile_colliding(map, collision_ref, test_tile.x, test_tile.y))
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
		if (false == is_tile_colliding(map, collision_ref, test_tile.x, test_tile.y))
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

tile_range find_vertical_range_of_free_tiles(level map, level collision_ref, tile_position starting_tile, u32 length_limit)
{
	tile_position upper_end = starting_tile;
	tile_position lower_end = starting_tile;
	tile_position test_tile = starting_tile;
	for (i32 distance = 0; distance <= length_limit; distance++)
	{
		test_tile.y = starting_tile.y - distance;
		if (false == is_tile_colliding(map, collision_ref, test_tile.x, test_tile.y))
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
		if (false == is_tile_colliding(map, collision_ref, test_tile.x, test_tile.y))
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

void fire_bullet(game_data* game, entity_type* bullet_type, world_position bullet_starting_position, 
	v2 bullet_offset, v2 velocity)
{
	if (game->bullets_count < game->bullets_max_count)
	{
		bullet* bul = &game->bullets[game->bullets_count];
		bul->type = bullet_type;
		bullet_starting_position.pos_in_chunk += bullet_offset;
		bul->position = renormalize_position(bullet_starting_position);
		bul->velocity = velocity;
		game->bullets_count++;
	}
}

void fire_bullet(game_data* game, entity* entity, b32 cooldown)
{
	assert(entity->type->fired_bullet_type);

	v2 direction = (entity->direction == direction::E
		? get_v2(1.0f, 0.0f) 
		: get_v2(-1.0f, 0.0f));
	v2 bullet_offset = (entity->direction == direction::E
		? entity->type->fired_bullet_offset 
		: get_v2(-entity->type->fired_bullet_offset.x, entity->type->fired_bullet_offset.y));

	fire_bullet(game, entity->type->fired_bullet_type, entity->position, bullet_offset,
		direction * entity->type->fired_bullet_type->constant_velocity);

	if (cooldown)
	{
		entity->attack_cooldown = entity->type->default_attack_cooldown;
	}
}

void remove_bullet(game_data* game, u32 bullet_index)
{
	assert(game->bullets_count > 0);
	assert(bullet_index < game->bullets_max_count);

	// compact array - działa też w przypadku bullet_index == bullets_count - 1
	bullet last_bullet = game->bullets[game->bullets_count - 1];
	game->bullets[bullet_index] = last_bullet;
	game->bullets_count--;
}

// nie ma znaczenia, czy sprawdzamy na osi x, czy y
b32 check_line_intersection(r32 start_coord, r32 movement_delta, r32 line_coord, r32* movement_perc)
{
	b32 result = false;

	r32 distance_to_line = line_coord - start_coord;
	if (movement_delta == 0)
	{
		result = false; // jesteśmy równolegli do ściany        
	}
	else if (start_coord == line_coord)
	{
		result = false; // stoimy dokładnie na ścianie
	}
	else
	{
		*movement_perc = distance_to_line / movement_delta;
		if (*movement_perc < 0.0f)
		{
			result = false; // ściana jest w drugą stronę
		}
		else if (*movement_perc > 1.0f)
		{
			result = false; // nie trafimy w tej klatce
		}
		else
		{
			result = true;
		}
	}
	return result;
}

// działa także, gdy zamienimy x z y
b32 check_segment_intersection(r32 movement_start_x, r32 movement_start_y,
	r32 movement_delta_x, r32 movement_delta_y,
	r32 line_x, r32 min_segment_y, r32 max_segment_y, r32* min_movement_perc)
{
	b32 result = false;
	r32 movement_perc = 0;
	if (check_line_intersection(movement_start_x, movement_delta_x, line_x, &movement_perc))
	{
		v2 movement_start = get_v2(movement_start_x, movement_start_y);
		v2 movement_delta = get_v2(movement_delta_x, movement_delta_y);
		v2 intersection_pos = movement_start + (movement_perc * movement_delta);
		// wiemy, że trafiliśmy w linię - sprawdzamy, czy mieścimy się w zakresie, który nas interesuje
		if (intersection_pos.y > min_segment_y && intersection_pos.y < max_segment_y)
		{
			result = true;
			if (*min_movement_perc > movement_perc)
			{
				*min_movement_perc = movement_perc;
			}
		}
	}
	return result;
}

entity* add_entity(game_data* game, world_position position, entity_type* type)
{
	assert(game->entities_count + 1 < game->entities_max_count);

	entity* new_entity = &game->entities[game->entities_count];
	game->entities_count++;
	new_entity->position = renormalize_position(position);
	new_entity->type = type;
	new_entity->health = type->max_health;
	return new_entity;
}

entity* add_entity(game_data* game, tile_position position, entity_type* type)
{
	entity* result = add_entity(game, get_world_position(position), type);
	return result;
}

void remove_entity(game_data* game, u32 entity_index)
{
	assert(game->entities_count > 1);
	assert(entity_index != 0);
	assert(entity_index < game->entities_max_count);

	// compact array - działa też w przypadku entity_index == entities_count - 1
	entity last_entity = game->entities[game->entities_count - 1];
	game->entities[entity_index] = last_entity;
	game->entities_count--;
}

entity* get_player(game_data* game)
{
	entity* result = &game->entities[0];
	return result;
}

b32 damage_player(game_data* game, i32 damage_amount)
{
	b32 damaged = false;
	if (game->player_invincibility_cooldown <= 0)
	{
		damaged = true;
		game->entities[0].health -= damage_amount;
		start_visual_effect(game, &game->entities[0], 0, false);
		printf("gracz dostaje %.0f obrazen, zostalo %.0f zdrowia\n", damage_amount, game->entities[0].health);
		if (game->entities[0].health < 0.0f)
		{
			// przegrywamy
			debug_breakpoint;
		}
		else
		{
			game->player_invincibility_cooldown = game->default_player_invincibility_cooldown;
		}
	}
	return damaged;
}

entity_collision_data get_entity_collision_data(chunk_position reference_chunk, entity* entity)
{
	entity_collision_data result = {};
	result.position = get_position_difference(entity->position, reference_chunk);
	result.collision_rect_dim = entity->type->collision_rect_dim;
	result.collision_rect_offset = entity->type->collision_rect_offset;
	return result;
}

entity_collision_data get_bullet_collision_data(chunk_position reference_chunk, bullet* bullet)
{
	entity_collision_data result = {};
	result.position = get_position_difference(bullet->position, reference_chunk);
	result.collision_rect_dim = bullet->type->collision_rect_dim;
	result.collision_rect_offset = bullet->type->collision_rect_offset;
	return result;
}

entity_collision_data get_tile_collision_data(chunk_position reference_chunk, tile_position tile_pos)
{
	entity_collision_data result = {};
	result.position = get_position_difference(tile_pos, reference_chunk);
	result.collision_rect_dim = get_v2(1.0f, 1.0f);
	return result;
}

collision check_minkowski_collision(
	entity_collision_data a,
	entity_collision_data b,
	v2 movement_delta, r32 min_movement_perc)
{
	collision result = {};
	result.possible_movement_perc = min_movement_perc;

	v2 relative_pos = (a.position + a.collision_rect_offset) - (b.position + b.collision_rect_offset);

	// środkiem zsumowanej figury jest (0,0,0)
	// pozycję playera traktujemy jako odległość od 0
	// 0 jest pozycją entity, z którym sprawdzamy kolizję

	v2 minkowski_dimensions = a.collision_rect_dim + b.collision_rect_dim;
	v2 min_corner = minkowski_dimensions * -0.5f;
	v2 max_corner = minkowski_dimensions * 0.5f;

	b32 west_wall = check_segment_intersection(
		relative_pos.x, relative_pos.y, movement_delta.x, movement_delta.y,
		min_corner.x, min_corner.y, max_corner.y, &result.possible_movement_perc);

	b32 east_wall = check_segment_intersection(
		relative_pos.x, relative_pos.y, movement_delta.x, movement_delta.y,
		max_corner.x, min_corner.y, max_corner.y, &result.possible_movement_perc);

	b32 north_wall = check_segment_intersection(
		relative_pos.y, relative_pos.x, movement_delta.y, movement_delta.x,
		max_corner.y, min_corner.x, max_corner.x, &result.possible_movement_perc);

	b32 south_wall = check_segment_intersection(
		relative_pos.y, relative_pos.x, movement_delta.y, movement_delta.x,
		min_corner.y, min_corner.x, max_corner.x, &result.possible_movement_perc);

	if (west_wall)
	{
		result.collided_wall = direction::W;
		result.collided_wall_normal = get_v2(-1, 0);
	}
	else if (east_wall)
	{
		result.collided_wall = direction::E;
		result.collided_wall_normal = get_v2(1, 0);
	}
	else if (north_wall)
	{
		result.collided_wall = direction::N;
		result.collided_wall_normal = get_v2(0, 1);
	}
	else if (south_wall)
	{
		result.collided_wall = direction::S;
		result.collided_wall_normal = get_v2(0, -1);
	}

	return result;
}

rect get_tiles_area_to_check_for_collision(world_position entity_position, v2 collision_rect_offset, v2 collision_rect_dim, world_position target_pos)
{
	entity_position.pos_in_chunk += collision_rect_offset;

	tile_position entity_tile = get_tile_position(entity_position);
	tile_position target_tile = get_tile_position(target_pos);

	i32 x_margin = (i32)SDL_ceil(collision_rect_dim.x);
	i32 y_margin = (i32)SDL_ceil(collision_rect_dim.y);

	rect result = {};
	result.min_corner.x = (r32)min((i32)entity_tile.x - x_margin, (i32)target_tile.x - x_margin);
	result.min_corner.y = (r32)min((i32)entity_tile.y - y_margin, (i32)target_tile.y - y_margin);
	result.max_corner.x = (r32)max((i32)entity_tile.x + x_margin, (i32)target_tile.x + x_margin);
	result.max_corner.y = (r32)max((i32)entity_tile.y + y_margin, (i32)target_tile.y + y_margin);
	return result;
}

rect get_tiles_area_to_check_for_collision(entity* entity, world_position target_pos)
{
	rect result = get_tiles_area_to_check_for_collision(
		entity->position, entity->type->collision_rect_offset, entity->type->collision_rect_dim, target_pos);
	return result;
}

rect get_tiles_area_to_check_for_collision(bullet* bullet, world_position target_pos)
{
	rect result = get_tiles_area_to_check_for_collision(
		bullet->position, bullet->type->collision_rect_offset, bullet->type->collision_rect_dim, target_pos);
	return result;
}

struct collision_with_effect
{
	collision data;
	entity* collided_entity;
	entity* collided_switch_entity;
};

collision_with_effect move(game_data* game, entity* moving_entity, world_position target_pos)
{
	collision_with_effect result = {};

	v2 movement_delta = get_position_difference(target_pos, moving_entity->position);
	chunk_position reference_chunk = get_tile_chunk_position(get_tile_position(moving_entity->position));

	if (false == is_zero(movement_delta))
	{
		r32 movement_apron = 0.001f;

		for (u32 iteration = 0; iteration < 4; iteration++)
		{
			if (is_zero(movement_delta))
			{
				break;
			}

			collision closest_collision = {};
			closest_collision.collided_wall = direction::NONE;
			closest_collision.possible_movement_perc = 1.0f;

			// collision with tiles
			{
				rect area_to_check = get_tiles_area_to_check_for_collision(moving_entity, target_pos);
				for (i32 tile_y_to_check = area_to_check.min_corner.y;
					tile_y_to_check <= area_to_check.max_corner.y;
					tile_y_to_check++)
				{
					for (i32 tile_x_to_check = area_to_check.min_corner.x;
						tile_x_to_check <= area_to_check.max_corner.x;
						tile_x_to_check++)
					{
						tile_position tile_to_check_pos = get_tile_position(tile_x_to_check, tile_y_to_check);
						u32 tile_value = get_tile_value(game->current_level, tile_x_to_check, tile_y_to_check);
						if (is_tile_colliding(game->collision_reference, tile_value))
						{
							collision new_collision = check_minkowski_collision(
								get_entity_collision_data(reference_chunk, moving_entity),
								get_tile_collision_data(reference_chunk, tile_to_check_pos),
								movement_delta, closest_collision.possible_movement_perc);

							if (new_collision.collided_wall != direction::NONE)
							{
								// paskudny hack rozwiązujący problem blokowania się na ścianie, jeśli kolidujemy z nią podczas spadania
								/*if (new_collision.collided_wall == direction::S)
								{
									u32 upper_tile_value = get_tile_value(game->current_level, tile_x_to_check, tile_y_to_check - 1);
									if (is_tile_colliding(game->collision_reference, upper_tile_value))
									{
										if ((moving_entity->position - tile_to_check_pos.x > 0)
										{
											new_collision.collided_wall_normal = get_v2(-1, 0);
										}
										else
										{
											new_collision.collided_wall_normal = get_v2(1, 0);
										}
									}
								}*/

								if (new_collision.possible_movement_perc < closest_collision.possible_movement_perc)
								{
									closest_collision = new_collision;
								}
							}
						}
					}
				}
			}

			collision closest_effect_entity_collision = {};
			closest_effect_entity_collision.collided_wall = direction::NONE;
			closest_effect_entity_collision.possible_movement_perc = 1.0f;
			entity* collided_effect_entity = NULL;

			// collision with entities
			{
				for (u32 entity_index = 0; entity_index < game->entities_count; entity_index++)
				{
					entity* entity_to_check = game->entities + entity_index;
					if (entity_to_check != moving_entity
						&& are_entity_flags_set(entity_to_check, entity_flags::COLLIDES))
					{
						collision new_collision = check_minkowski_collision(
							get_entity_collision_data(reference_chunk, moving_entity),
							get_entity_collision_data(reference_chunk, entity_to_check),
							movement_delta, closest_effect_entity_collision.possible_movement_perc);

						if (new_collision.collided_wall != direction::NONE)
						{
							if (are_entity_flags_set(entity_to_check, entity_flags::COLLIDES))
							{
								closest_collision = new_collision;

								if (are_entity_flags_set(moving_entity, entity_flags::PLAYER) 
									&& are_entity_flags_set(entity_to_check, entity_flags::SWITCH))
								{
									result.collided_switch_entity = entity_to_check;
								}
							}

							if (new_collision.possible_movement_perc < closest_effect_entity_collision.possible_movement_perc)
							{
								closest_effect_entity_collision = new_collision;

								if (are_entity_flags_set(moving_entity, entity_flags::PLAYER)
									&& are_entity_flags_set(entity_to_check, entity_flags::ENEMY))
								{
									collided_effect_entity = entity_to_check;
								}
							}
						}
					}
				}
			}

			// przesuwamy się o tyle, o ile możemy
			if ((closest_collision.possible_movement_perc - movement_apron) > 0.0f)
			{
				v2 possible_movement = movement_delta * (closest_collision.possible_movement_perc - movement_apron);
				moving_entity->position = add_to_position(moving_entity->position, possible_movement);
				// pozostałą deltę zmniejszamy o tyle, o ile się poruszyliśmy
				movement_delta -= possible_movement;
				//printf("przesuniecie o (%.04f, %.04f)\n", possible_movement.x, possible_movement.y);
			}
			
			result.data = closest_collision;

			// jeśli z entity mieliśmy kolizję wcześniej niż z tile
			if (closest_effect_entity_collision.possible_movement_perc <= closest_collision.possible_movement_perc)
			{
				if (collided_effect_entity)
				{
					result.data = closest_effect_entity_collision;
					result.collided_entity = collided_effect_entity;
				}
			}			

			if (false == is_zero(closest_collision.collided_wall_normal))
			{
				v2 wall_normal = closest_collision.collided_wall_normal;
				v2 movement_delta_orig = movement_delta;

				// i sprawdzamy, co zrobić z pozostałą deltą - czy możemy się poruszyć wzdłuż ściany lub odbić
				i32 how_many_times_subtract = 1; // 1 dla ślizgania się, 2 dla odbijania    
				v2 bounced = wall_normal * inner(wall_normal, moving_entity->velocity);
				moving_entity->velocity -= how_many_times_subtract * bounced;

				movement_delta -= how_many_times_subtract * (wall_normal * inner(movement_delta, wall_normal));

				if (movement_delta.x == movement_delta_orig.x && movement_delta.y == movement_delta_orig.y)
				{
					debug_breakpoint;
				}

				//printf("collision fr: %d, iter: %d, before: (%.04f, %.04f) after: (%.04f, %.04f) \n",
				//	sdl_game->debug_frame_counter, iteration, movement_delta_orig.x, movement_delta_orig.y, movement_delta.x, movement_delta.y);
			}
			else
			{
				// jeśli nie było kolizji, nie ma potrzeby kolejnych iteracji
				break;
			}
		}
	}

	return result;
}

b32 move_bullet(game_data* game, bullet* moving_bullet, u32 bullet_index, world_position target_pos)
{
	b32 was_collision = false;
	
	v2 movement_delta = get_position_difference(target_pos, moving_bullet->position);
	chunk_position reference_chunk = get_tile_chunk_position(get_tile_position(moving_bullet->position));

	if (false == is_zero(movement_delta))
	{
		r32 movement_apron = 0.001f;

		collision closest_collision = {};
		closest_collision.collided_wall = direction::NONE;
		closest_collision.possible_movement_perc = 1.0f;
		entity* collided_entity = NULL;

		// collision with tiles
		{
			rect area_to_check = get_tiles_area_to_check_for_collision(moving_bullet, target_pos);
			for (i32 tile_y_to_check = area_to_check.min_corner.y;
				tile_y_to_check <= area_to_check.max_corner.y;
				tile_y_to_check++)
			{
				for (i32 tile_x_to_check = area_to_check.min_corner.x;
					tile_x_to_check <= area_to_check.max_corner.x;
					tile_x_to_check++)
				{
					tile_position tile_to_check_pos = get_tile_position(tile_x_to_check, tile_y_to_check);
					u32 tile_value = get_tile_value(game->current_level, tile_x_to_check, tile_y_to_check);
					if (is_tile_colliding(game->collision_reference, tile_value))
					{
						collision new_collision = check_minkowski_collision(
							get_bullet_collision_data(reference_chunk, moving_bullet),
							get_tile_collision_data(reference_chunk, tile_to_check_pos),
							movement_delta, closest_collision.possible_movement_perc);

						if (new_collision.collided_wall != direction::NONE)
						{
							if (new_collision.possible_movement_perc < closest_collision.possible_movement_perc)
							{
								closest_collision = new_collision;
							}
						}
					}
				}
			}
		}

		// collision with entities
		{
			for (u32 entity_index = 0; entity_index < game->entities_count; entity_index++)
			{
				entity* entity_to_check = game->entities + entity_index;
				if (are_entity_flags_set(entity_to_check, entity_flags::COLLIDES))
				{
					collision new_collision = check_minkowski_collision(
						get_bullet_collision_data(reference_chunk, moving_bullet),
						get_entity_collision_data(reference_chunk, entity_to_check),
						movement_delta, closest_collision.possible_movement_perc);

					if (new_collision.collided_wall != direction::NONE)
					{
						if (are_flags_set(&moving_bullet->type->flags, entity_flags::DAMAGES_PLAYER))
						{
							if (entity_index == 0)
							{
								// mamy gracza							
								if (new_collision.possible_movement_perc < closest_collision.possible_movement_perc)
								{
									closest_collision = new_collision;
									collided_entity = entity_to_check;
								}
							}
						}
						else
						{
							if (entity_index != 0)
							{
								// mamy przeciwnika
								if (new_collision.possible_movement_perc < closest_collision.possible_movement_perc)
								{
									closest_collision = new_collision;
									collided_entity = entity_to_check;
								}
							}
						}
					}
				}
			}
		}

		was_collision = (closest_collision.collided_wall != direction::NONE);

		if ((closest_collision.possible_movement_perc - movement_apron) > 0.0f)
		{
			v2 possible_movement = movement_delta * (closest_collision.possible_movement_perc - movement_apron);
			moving_bullet->position = add_to_position(moving_bullet->position, possible_movement);
			movement_delta -= possible_movement;
		}

		if (collided_entity)
		{
			if (are_entity_flags_set(collided_entity, entity_flags::PLAYER))
			{
				damage_player(game, moving_bullet->type->damage_on_contact);
			}
			else
			{
				if (false == are_entity_flags_set(collided_entity, entity_flags::INDESTRUCTIBLE))
				{
					start_visual_effect(game, collided_entity, 0, false);
					collided_entity->health -= moving_bullet->type->damage_on_contact;
					printf("pocisk trafil w entity, %.2f obrazen, zostalo %.2f\n",
						moving_bullet->type->damage_on_contact, collided_entity->health);
				}
			}
		}

		if (was_collision)
		{
			remove_bullet(game, bullet_index);
		}
	}

	return was_collision;
}

b32 is_standing_on_ground(game_data* game, entity* entity_to_check)
{
	b32 result = false;
	r32 corner_distance_apron = 0.0f;
	r32 max_distance_to_tile = 0.05f;

	entity test_entity = *entity_to_check;
	world_position target_pos = add_to_position(test_entity.position, get_v2(0.0f, 0.1f));
	collision_with_effect collision = move(game, &test_entity, target_pos);
	if (collision.collided_entity || collision.data.collided_wall == direction::S)
	{
		result = true;
	}
	return result;
}

void render_debug_information(sdl_game_data* sdl_game, game_data* game)
{
	entity* player = get_player(game);

	b32 is_standing = is_standing_on_ground(game, player);

	char buffer[200];
	v4 text_color = get_v4(255, 255, 255, 0);
	/*int error = SDL_snprintf(buffer, 200, "Frame: %d Elapsed: %0.2f ms, Pos: (%0.2f,%0.2f) Acc: (%0.2f,%0.2f) Standing: %d ",
		sdl_game->debug_frame_counter, sdl_game->debug_elapsed_work_ms, player->position.x, player->position.y,
		player->acceleration.x, player->acceleration.y, is_standing);*/
	int error = SDL_snprintf(buffer, 200, "Chunk:(%d,%d),Pos:(%0.2f,%0.2f),Acc: (%0.2f,%0.2f) Standing: %d, Direction: %s",
		player->position.chunk_pos.x, player->position.chunk_pos.y, player->position.pos_in_chunk.x, player->position.pos_in_chunk.y,
		player->acceleration.x, player->acceleration.y, is_standing, (player->direction == direction::W ? "W" : "E"));
	render_text(sdl_game, buffer, 10, 200, text_color);
}

void write_to_input_buffer(input_buffer* buffer, game_input* new_input)
{
	buffer->buffer[buffer->current_index] = *new_input;
	buffer->current_index++;
	if (buffer->current_index == buffer->size)
	{
		buffer->current_index = 0;
	}
}

game_input* get_past_input(input_buffer* buffer, u32 how_many_frames_backwards)
{
	i32 input_index = buffer->current_index - 1 - how_many_frames_backwards;
	while (input_index < 0)
	{
		input_index = buffer->size + input_index;
	}
	game_input* result = &buffer->buffer[input_index];
	return result;
}

game_input* get_last_frame_input(input_buffer* buffer)
{
	game_input* result = get_past_input(buffer, 0);
	return result;
}

// żeby działało na dowolnych przyciskach, trzeba dodać nowy enum
b32 was_up_key_pressed_in_last_frames(input_buffer* buffer, u32 number_of_frames)
{
	b32 result = false;
	for (u32 frame = 1; frame <= number_of_frames; frame++)
	{
		game_input* input = get_past_input(buffer, frame);
		if (input->up.number_of_presses > 0)
		{
			result = true;
			break;
		}
	}
	return result;
}

void circular_buffer_test(memory_arena* arena)
{
	temporary_memory test_memory = begin_temporary_memory(arena);

	u32 test_input_count = 200;

	input_buffer* input_buf = push_struct(test_memory.arena, input_buffer);
	input_buf->size = 100;
	input_buf->buffer = push_array(test_memory.arena, input_buf->size, game_input);

	for (u32 input_index = 0; input_index < test_input_count; input_index++)
	{
		game_input* new_test_input = push_struct(test_memory.arena, game_input);
		new_test_input->up.number_of_presses = input_index;
		write_to_input_buffer(input_buf, new_test_input);
	}

	game_input* test_input = 0;
	test_input = get_past_input(input_buf, 0);
	assert(test_input->up.number_of_presses == test_input_count - 1);

	test_input = get_past_input(input_buf, input_buf->size);
	assert(test_input->up.number_of_presses == test_input_count - 1);

	test_input = get_past_input(input_buf, 1);
	assert(test_input->up.number_of_presses == test_input_count - 2);

	end_temporary_memory(test_memory);
}

void change_movement_mode(player_movement* movement, movement_mode mode)
{
	movement->previous_mode = movement->current_mode;
	movement->previous_mode_frame_duration = movement->frame_duration;
	movement->current_mode = mode;
	movement->frame_duration = 0;

	//debug
	switch (mode)
	{
		case movement_mode::WALK:
			printf("switch to WALK after %d frames\n", movement->previous_mode_frame_duration);
			break;
		case movement_mode::JUMP:
			printf("switch to JUMP after %d frames\n", movement->previous_mode_frame_duration);
			break;
		case movement_mode::RECOIL:
			printf("switch to RECOIL after %d frames\n", movement->previous_mode_frame_duration);
			break;
	}
}

world_position process_input(game_data* game, entity* player, r32 delta_time)
{
	game_input* input = get_last_frame_input(&game->input);

	b32 is_standing_at_frame_beginning = is_standing_on_ground(game, player);

	v2 gravity = get_v2(0, 1.0f);

	// zmiana statusu
	switch (game->player_movement.current_mode)
	{
		case movement_mode::WALK:
		{
			if (false == is_standing_at_frame_beginning)
			{
				change_movement_mode(&game->player_movement, movement_mode::JUMP);
			}
		}
		break;
		case movement_mode::JUMP:
		{
			if (is_standing_at_frame_beginning)
			{
				change_movement_mode(&game->player_movement, movement_mode::WALK);
			}
		}
		break;
		case movement_mode::RECOIL:
		{
			// czy odzyskujemy kontrolę?
			if (is_standing_at_frame_beginning)
			{
				if (game->player_movement.recoil_timer > 0.0f)
				{
					// nie
					game->player_movement.recoil_timer -= delta_time;
				}
				else
				{
					// tak
					change_movement_mode(&game->player_movement, movement_mode::WALK);
				}
			}
			else
			{
				// w tym wypadku po prostu lecimy dalej
			}
		}
		break;
	}

	game->player_movement.frame_duration++;

	if (player->attack_cooldown > 0)
	{
		player->attack_cooldown -= delta_time;
	}

	// przetwarzanie inputu
	player->acceleration = get_zero_v2();
	switch (game->player_movement.current_mode)
	{
		case movement_mode::WALK:
		{
			// ułatwienie dla gracza - jeśli gracz nacisnął skok w ostatnich klatkach skoku, wykonujemy skok i tak
			if (game->player_movement.frame_duration == 1)
			{
				if (is_standing_at_frame_beginning
					&& game->player_movement.previous_mode == movement_mode::JUMP)
				{
					if (was_up_key_pressed_in_last_frames(&game->input, 3))
					{
						player->acceleration += get_v2(0, -30);
						change_movement_mode(&game->player_movement, movement_mode::JUMP);
						printf("ulatwienie!\n");
						break;
					}
				}
			}

			if (input->is_left_mouse_key_held)
			{
				if (player->attack_cooldown <= 0)
				{
					fire_bullet(game, player, true);
				}
			}

			if (input->up.number_of_presses > 0)
			{
				if (is_standing_at_frame_beginning)
				{
					player->acceleration += get_v2(0, -30);
					change_movement_mode(&game->player_movement, movement_mode::JUMP);
					break;
				}
			}

			if (input->left.number_of_presses > 0)
			{
				player->acceleration += get_v2(-1, 0);
			}

			if (input->right.number_of_presses > 0)
			{
				player->acceleration += get_v2(1, 0);
			}

#if 0
			if (input->up.number_of_presses > 0)
			{
				player->acceleration += get_v2(0, -1);
			}

			if (input->down.number_of_presses > 0)
			{
				player->acceleration += get_v2(0, 1);
			}
#endif
		}
		break;
		case movement_mode::JUMP:
		{
			player->acceleration = gravity;

			if (input->is_left_mouse_key_held)
			{
				if (player->attack_cooldown <= 0)
				{
					fire_bullet(game, player, true);
				}
			}

			if (input->left.number_of_presses > 0)
			{
				player->acceleration += get_v2(-0.5f, 0);
			}

			if (input->right.number_of_presses > 0)
			{
				player->acceleration += get_v2(0.5f, 0);
			}
		}
		break;
		case movement_mode::RECOIL:
		{
			if (false == is_standing_at_frame_beginning)
			{
				player->acceleration = gravity;
			}

			if (game->player_movement.recoil_acceleration_timer > 0.0f)
			{
				game->player_movement.recoil_acceleration_timer -= delta_time;
				player->acceleration += game->player_movement.recoil_acceleration;
			}
		}
		break;
	}

	player->velocity = player->type->slowdown_multiplier *
		(player->velocity + (player->acceleration * delta_time));

	world_position target_pos = add_to_position(player->position,
		player->velocity * player->type->velocity_multiplier * delta_time);

	/*printf("target_pos: chunk: (%d,%d), pos: (%0.02f,%0.2f)\n", 
		target_pos.chunk_pos.x, target_pos.chunk_pos.y, target_pos.pos_in_chunk.x, target_pos.pos_in_chunk.y);*/

	return target_pos;
}

void animate_entity(player_movement* movement, entity* entity, r32 delta_time)
{
	if (entity->visual_effect)
	{
		if (entity->visual_effect->total_duration != 0.0f)
		{
			if (entity->visual_effect_duration < entity->visual_effect->total_duration)
			{
				entity->visual_effect_duration += delta_time;
			}
			else
			{
				if (are_flags_set((u32*)&entity->visual_effect->flags, (u32)sprite_effect_flags::REPEATS))
				{
					entity->visual_effect_duration = 0;
				}
				else
				{
					entity->visual_effect = NULL;
				}
			}
		}
	}

	if (entity->current_animation)
	{
		entity->animation_duration += delta_time;
		while (entity->animation_duration > entity->current_animation->total_duration)
		{
			entity->animation_duration -= entity->current_animation->total_duration;
		}

		entity->current_animation->current_frame_index =
			get_current_animation_frame_index(entity->current_animation, entity->animation_duration);
	}

	if (movement)
	{
		switch (movement->current_mode)
		{
			case movement_mode::WALK:
			{
				if (length(entity->velocity) > 0.05f)
				{
					if (entity->current_animation != entity->type->walk_animation)
					{
						entity->current_animation = entity->type->walk_animation;
						entity->animation_duration = 0.0f;
					}
				}
				else
				{
					entity->current_animation = NULL;
					entity->animation_duration = 0.0f;
				}
			}
			break;
			case movement_mode::JUMP:
			{
				entity->current_animation = NULL;
				entity->animation_duration = 0.0f;
			}
			break;
			case movement_mode::RECOIL:
			{
				entity->current_animation = NULL;
				entity->animation_duration = 0.0f;
			}
			break;
		}
	}
}

SDL_Rect get_render_rect(v2 position_relative_to_camera, v2 rect_size)
{
	SDL_Rect result = {};
	result.w = rect_size.x;
	result.h = rect_size.y;
	result.x = SCREEN_CENTER_IN_PIXELS.x + (position_relative_to_camera.x * TILE_SIDE_IN_PIXELS)
		- (rect_size.x / 2);
	result.y = SCREEN_CENTER_IN_PIXELS.y + (position_relative_to_camera.y * TILE_SIDE_IN_PIXELS)
		- (rect_size.y / 2);
	return result;
}

SDL_Rect get_tile_render_rect(v2 position_relative_to_camera)
{
	SDL_Rect result = {};
	result.w = TILE_SIDE_IN_PIXELS;
	result.h = TILE_SIDE_IN_PIXELS;
	result.x = SCREEN_CENTER_IN_PIXELS.x + (position_relative_to_camera.x * TILE_SIDE_IN_PIXELS) 
		- (TILE_SIDE_IN_PIXELS / 2);
	result.y = SCREEN_CENTER_IN_PIXELS.y + (position_relative_to_camera.y * TILE_SIDE_IN_PIXELS) 
		- (TILE_SIDE_IN_PIXELS / 2);
	return result;
}

void debug_render_tile(SDL_Renderer* renderer, tile_position tile_pos, v4 color, world_position camera_pos)
{
	v2 position = get_position_difference(tile_pos, camera_pos);
	SDL_Rect screen_rect = get_tile_render_rect(position);
	SDL_RenderFillRect(renderer, &screen_rect);
}

void render_debug_path_ends(sdl_game_data* sdl_game, entity* entity, world_position camera_pos)
{
	if (entity->has_walking_path)
	{
		SDL_SetRenderDrawColor(sdl_game->renderer, 0, 0, 255, 0);
		debug_render_tile(sdl_game->renderer, entity->path.start, { 255,255,0,255 }, camera_pos);
		debug_render_tile(sdl_game->renderer, entity->path.end, { 0,0,255,255 }, camera_pos);
	}
}

void render_entity_sprite(SDL_Renderer* renderer,
	world_position camera_position, world_position entity_position, direction entity_direction,
	sprite_effect* visual_effect, r32 visual_effect_duration, sprite sprite)
{	
	b32 tint_modified = false;
	v4 tint = get_zero_v4();

	if (visual_effect)
	{
		tint_modified = true;	
		tint = get_tint(visual_effect, visual_effect_duration);
	}	

	for (u32 part_index = 0; part_index < sprite.parts_count; part_index++)
	{
		sprite_part* part = &sprite.parts[part_index];
		v2 offset = part->offset_in_pixels;

		SDL_RendererFlip flip = SDL_FLIP_NONE;
		if (entity_direction != part->default_direction)
		{
			flip = SDL_FLIP_HORIZONTAL; // SDL_FLIP_VERTICAL
			offset = get_v2(-part->offset_in_pixels.x, part->offset_in_pixels.y);
		}

		v2 position = get_position_difference(entity_position, camera_position);
		SDL_Rect screen_rect = get_render_rect(position, get_v2(part->texture_rect.w, part->texture_rect.h));
		screen_rect.x += offset.x;
		screen_rect.y += offset.y;

		if (tint_modified)
		{
			assert(tint.r >= 0 && tint.r <= 1 && tint.g >= 0 && tint.g <= 1 && tint.b >= 0 && tint.b <= 1);

			v4 sdl_tint = tint * 255;
			if (are_flags_set(&visual_effect->flags, sprite_effect_flags::ADDITIVE_MODE))
			{
				// rysujemy dwa razy - raz normalnie, a raz dodajemy do wartości koloru
				SDL_RenderCopyEx(renderer, part->texture, &part->texture_rect, &screen_rect, 0, NULL, flip);

				SDL_SetTextureBlendMode(part->texture, SDL_BLENDMODE_ADD);
				SDL_SetTextureColorMod(part->texture, sdl_tint.r, sdl_tint.g, sdl_tint.b);
				SDL_RenderCopyEx(renderer, part->texture, &part->texture_rect, &screen_rect, 0, NULL, flip);
				SDL_SetTextureColorMod(part->texture, 255, 255, 255);
				SDL_SetTextureBlendMode(part->texture, SDL_BLENDMODE_BLEND);
			}
			else
			{
				SDL_SetTextureColorMod(part->texture, sdl_tint.r, sdl_tint.g, sdl_tint.b);
				SDL_RenderCopyEx(renderer, part->texture, &part->texture_rect, &screen_rect, 0, NULL, flip);
				SDL_SetTextureColorMod(part->texture, 255, 255, 255);
			}
		}
		else
		{
			SDL_RenderCopyEx(renderer, part->texture, &part->texture_rect, &screen_rect, 0, NULL, flip);
		}
	}
}

void render_entity_animation_frame(SDL_Renderer* renderer, 
	world_position camera_position, entity* entity)
{
	sprite* sprite_to_render = NULL;
	if (entity->current_animation && entity->current_animation->frames)
	{	
		sprite_to_render = &entity->current_animation->frames[entity->current_animation->current_frame_index].sprite;
	}
	
	if (sprite_to_render == NULL || sprite_to_render->parts == NULL)
	{
		sprite_to_render = &entity->type->idle_pose.sprite;
	}

	assert(sprite_to_render != NULL);

	render_entity_sprite(renderer, camera_position, entity->position, entity->direction,
		entity->visual_effect, entity->visual_effect_duration, *sprite_to_render);
}

u32 get_hash_from_color(v4 color)
{
	u32 result = ((u32)color.r * 13 + (u32)color.g * 7 + (u32)color.b * 3);
	return result;
}

void add_gate_to_dictionary(memory_arena* arena, gate_dictionary dict, entity* gate_entity)
{
	v4 color = gate_entity->type->color;
	u32 index = get_hash_from_color(color) % dict.entries_count;
	gate_dictionary_entry* entry = dict.entries + index;
	if (entry->entity == NULL)
	{
		entry->entity = gate_entity;
	}
	else
	{
		while (entry && entry->next)
		{
			entry = entry->next;
		}

		gate_dictionary_entry* new_entry = push_struct(arena, gate_dictionary_entry);
		new_entry->entity = gate_entity;
		entry->next = new_entry;
	}
}

void open_gates_with_given_color(gate_dictionary dict, v4 color)
{
	u32 index = get_hash_from_color(color) % dict.entries_count;
	gate_dictionary_entry* entry = &dict.entries[index];
	while (entry)
	{
		if (entry->entity != NULL)
		{
			assert(entry->entity->type->color == color);
			tile_position pos = get_tile_position(entry->entity->position);
			if (are_entity_flags_set(entry->entity, entity_flags::SWITCH))
			{
				printf("switch na pozycji (%d,%d)\n", pos.x, pos.y);
			}
			else
			{
				unset_flags(&entry->entity->type->flags, entity_flags::COLLIDES);
				//entry->entity->type->collision_rect_dim = get_zero_v2();
				entry->entity->type->idle_pose.sprite.parts_count = 0;
				printf("brama na pozycji (%d,%d)\n", pos.x, pos.y);
			}					
		}
		entry = entry->next;
	}

	// w ten sposób nie będziemy otwierać bram ponownie
	dict.entries[index].entity = NULL;
	dict.entries[index].next = NULL;
}

void add_gate_entity(game_data* game, memory_arena* arena, entity_to_spawn* new_entity_to_spawn, b32 is_switch)
{
	entity_type* new_type = push_struct(arena, entity_type);

	u32 max_size = 10;

	v2 collision_rect_dim;
	tile_range occupied_tiles;
	if (is_switch)
	{
		occupied_tiles = find_horizontal_range_of_free_tiles(
			game->current_level, game->collision_reference, new_entity_to_spawn->position, max_size);
		collision_rect_dim = get_length_from_tile_range(occupied_tiles);
	}
	else
	{
		occupied_tiles = find_vertical_range_of_free_tiles(
			game->current_level, game->collision_reference, new_entity_to_spawn->position, max_size);
		collision_rect_dim = get_length_from_tile_range(occupied_tiles);
	}

	new_type->collision_rect_dim = collision_rect_dim;
	new_type->color = new_entity_to_spawn->color;

	world_position new_position = add_to_position(
		get_world_position(occupied_tiles.start),
		get_position_difference(occupied_tiles.end, occupied_tiles.start) / 2);

	if (is_switch)
	{
		u32 tiles_count = occupied_tiles.end.x - occupied_tiles.start.x + 1;

		animation_frame frame = {};
		frame.sprite.parts_count = tiles_count * 2;
		frame.sprite.parts = push_array(arena, frame.sprite.parts_count, sprite_part);

		for (u32 distance = 0; distance < tiles_count; distance++)
		{
			sprite_part* part = &frame.sprite.parts[distance];
			if (distance == 0)
			{
				*part = game->switch_frame_left_sprite;
			}
			else if (distance == tiles_count - 1)
			{
				*part = game->switch_frame_right_sprite;
			}
			else
			{
				*part = game->switch_frame_middle_sprite;
			}
			part->offset_in_pixels = get_position_difference(
				get_tile_position(occupied_tiles.start.x + distance, occupied_tiles.start.y), new_position) 
				* TILE_SIDE_IN_PIXELS;
		}

		for (u32 distance = 0; distance < tiles_count; distance++)
		{
			sprite_part* part = &frame.sprite.parts[tiles_count + distance];
			if (distance == 0)
			{
				*part = game->switch_off_left_sprite;
			}
			else if (distance == tiles_count - 1)
			{
				*part = game->switch_off_right_sprite;
			}
			else
			{
				*part = game->switch_off_middle_sprite;
			}
			part->offset_in_pixels = get_position_difference(
				get_tile_position(occupied_tiles.start.x + distance, occupied_tiles.start.y), new_position) 
				* TILE_SIDE_IN_PIXELS;
		}

		new_type->idle_pose = frame;
	}
	else
	{
		u32 tiles_count = occupied_tiles.end.y - occupied_tiles.start.y + 1;

		animation_frame frame = {};
		frame.sprite.parts_count = tiles_count;
		frame.sprite.parts = push_array(arena, frame.sprite.parts_count, sprite_part);

		for (u32 distance = 0; distance < tiles_count; distance++)
		{
			sprite_part* part = &frame.sprite.parts[distance];
			*part = game->gate_sprite;

			part->offset_in_pixels = get_position_difference(
				get_tile_position(occupied_tiles.start.x, occupied_tiles.start.y + distance), new_position) 
				* TILE_SIDE_IN_PIXELS;
		}

		new_type->idle_pose = frame;
	}

	set_flags(&new_type->flags, entity_flags::COLLIDES);
	set_flags(&new_type->flags, entity_flags::INDESTRUCTIBLE);
	set_flags(&new_type->flags, (is_switch ? entity_flags::SWITCH : entity_flags::GATE));

	entity* new_entity = add_entity(game, new_position, new_type);
	add_gate_to_dictionary(arena, game->gates_dict, new_entity);
}

void initialize_entites(sdl_game_data* sdl_game, game_data* game)
{
	temporary_memory memory_for_entities_initialization = begin_temporary_memory(sdl_game->transient_arena);

	add_entity(game, game->current_level.starting_tile,
		get_entity_type_ptr(game->entity_types_dict, entity_type_enum::PLAYER));

	game->gates_dict.entries_count = 100;
	game->gates_dict.entries = push_array(sdl_game->arena, game->gates_dict.entries_count, gate_dictionary_entry);

	for (u32 entity_index = 0;
		entity_index < game->current_level.entities_to_spawn_count;
		entity_index++)
	{
		entity_to_spawn* new_entity = game->current_level.entities_to_spawn + entity_index;
		switch (new_entity->type)
		{
			case entity_type_enum::GATE:
			{
				add_gate_entity(game, sdl_game->arena, new_entity, false);
			}
			break;
			case entity_type_enum::SWITCH:
			{
				add_gate_entity(game, sdl_game->arena, new_entity, true);
			}
			break;
			case entity_type_enum::UNKNOWN:
			break;
			default:
			{
				add_entity(game, get_world_position(new_entity->position),
					get_entity_type_ptr(game->entity_types_dict, new_entity->type));
			}
			break;
		}
	}

	end_temporary_memory(memory_for_entities_initialization);
}

void update_and_render(sdl_game_data* sdl_game, game_data* game, r32 delta_time)
{
	if (false == game->current_level_initialized)
	{
		initialize_entites(sdl_game, game);
		game->current_level_initialized = true;
	}

	entity* player = get_player(game);

	entity* debug_entity_to_render_path = 0;

	// update player
	{
		if (game->player_invincibility_cooldown > 0.0f)
		{
			game->player_invincibility_cooldown -= delta_time;
			//printf("niezniszczalnosc jeszcze przez %.02f\n", game->player_invincibility_cooldown);
		}

		animate_entity(&game->player_movement, player, delta_time);

		world_position target_pos = process_input(game, player, delta_time);

		collision_with_effect collision = move(game, player, target_pos);
		if (collision.collided_entity)
		{
			damage_player(game, collision.collided_entity->type->damage_on_contact);

			v2 direction = get_unit_vector(
				get_position_difference(player->position, collision.collided_entity->position));
			
			r32 acceleration = collision.collided_entity->type->player_acceleration_on_collision;

			game->player_movement.recoil_timer = 2.0f;
			game->player_movement.recoil_acceleration_timer = 1.0f;
			game->player_movement.recoil_acceleration = (direction * acceleration);

			printf("odrzut! nowe przyspieszenie: (%.02f,%.02f)\n",
				game->player_movement.recoil_acceleration.x,
				game->player_movement.recoil_acceleration.y);

			change_movement_mode(&game->player_movement, movement_mode::RECOIL);
		}

		if (collision.collided_switch_entity)
		{
			v4 color = collision.collided_switch_entity->type->color;
			open_gates_with_given_color(game->gates_dict, color);
		}

		v2 player_direction_v2 = get_unit_vector(player->velocity);
		if (false == is_zero(player->velocity))
		{
			player->direction = player->velocity.x < 0 ? direction::W : direction::E;
		}
		else
		{
			// zostawiamy stary
		}
	}

	// update entities
	for (u32 entity_index = 1; entity_index < game->entities_count; entity_index++)
	{
		entity* entity = game->entities + entity_index;

		if (false == is_in_neighbouring_chunk(player->position.chunk_pos, entity->position))
		{
			continue;
		}

		animate_entity(NULL, entity, delta_time);

		if (entity->health < 0)
		{
			remove_entity(game, entity_index);
			entity_index--; // ze względu na działanie compact array
		}

		if (entity->attack_cooldown > 0)
		{
			entity->attack_cooldown -= delta_time;
		}

		if (are_entity_flags_set(entity, entity_flags::WALKS_HORIZONTALLY))
		{
			if (entity->has_walking_path)
			{
				tile_position current_goal;
				tile_position current_start;

				if (entity->goal_path_point == 0)
				{
					current_goal = entity->path.start;
					current_start = entity->path.end;
				}
				else if (entity->goal_path_point == 1)
				{
					current_goal = entity->path.end;
					current_start = entity->path.start;
				}
				else
				{
					// wracamy na początek
					entity->goal_path_point = 0;
					current_goal = entity->path.start;
					current_start = entity->path.end;
				}

				if (current_goal != current_start)
				{
					v2 distance = get_position_difference(current_goal, entity->position);
					r32 distance_length = length(distance);
					v2 distance_to_start = get_position_difference(current_start, entity->position);
					r32 distance_to_start_length = length(distance_to_start);

					v2 direction = get_zero_v2();
					if (distance_length != 0)
					{
						direction = get_unit_vector(distance);
					}

					r32 velocity = entity->type->velocity_multiplier;

					r32 slowdown_threshold = 2.0f;
					r32 fudge = 0.1f;
					if (distance_length < slowdown_threshold)
					{
						velocity *= ((distance_length + fudge) / slowdown_threshold);
					}
					else if (distance_to_start_length < slowdown_threshold)
					{
						velocity *= ((distance_to_start_length + fudge) / slowdown_threshold);
					}

					world_position new_position = add_to_position(entity->position, (direction * velocity * delta_time));
					entity->position = new_position;

					if (length(get_position_difference(current_goal, entity->position)) < 0.01f)
					{
						//entity->position = goal_pos;

						if (entity->goal_path_point == 0)
						{
							//printf("dotarliśmy do 0\n");
							entity->goal_path_point = 1;
						}
						else if (entity->goal_path_point == 1)
						{
							//printf("dotarliśmy do 1\n");
							entity->goal_path_point = 0;
						}
					}
				}
				debug_entity_to_render_path = entity;
			}
			else
			{
				tile_range new_path = find_walking_path_for_enemy(
					game->current_level, game->collision_reference, get_tile_position(entity->position));
				entity->path = new_path;
				entity->has_walking_path = true;

				// zabezpieczenie na wypadek nierównego ustawienia entity w edytorze 
				tile_position current_position = get_tile_position(entity->position);
				if (current_position.y != entity->path.start.y)
				{
					tile_position new_position = get_tile_position(current_position.x, entity->path.start.y);
					entity->position = get_world_position(new_position);
				}
			}
		}

		if (are_entity_flags_set(entity, entity_flags::ENEMY)
			&& entity->type->fired_bullet_type)
		{
			if (entity->attack_cooldown <= 0)
			{
				v2 player_relative_pos = get_position_difference(player->position, entity->position);
				r32 distance_to_player = length(player_relative_pos);
				if (distance_to_player < entity->type->player_detecting_distance)
				{
					v2 direction_to_player = get_unit_vector(player_relative_pos);
					entity->direction = direction_to_player.x < 0 ? direction::W : direction::E;
					fire_bullet(game, entity->type->fired_bullet_type, entity->position, get_zero_v2(),
						direction_to_player * entity->type->fired_bullet_type->constant_velocity);
					entity->attack_cooldown = entity->type->default_attack_cooldown;
				}
			}
		}
	}

	// update bullets
	for (u32 bullet_index = 0; bullet_index < game->bullets_count; bullet_index++)
	{
		bullet* bullet = game->bullets + bullet_index;

		if (is_in_neighbouring_chunk(player->position.chunk_pos, bullet->position))
		{
			if (bullet->type)
			{
				world_position bullet_target_pos = add_to_position(bullet->position, bullet->velocity * delta_time);
				b32 hit = move_bullet(game, bullet, bullet_index, bullet_target_pos);
				if (hit)
				{
					bullet_index--; // ze względu na compact array - został usunięty bullet, ale nowy został wstawiony na jego miejsce
				}
			}

			if (is_zero(bullet->velocity))
			{
				remove_bullet(game, bullet_index);
				bullet_index--;
			}
		}
		else
		{
			remove_bullet(game, bullet_index);
			bullet_index--;
		}		
	}

	// rendering
	{
		SDL_SetRenderDrawColor(sdl_game->renderer, 0, 255, 0, 0);
		SDL_RenderClear(sdl_game->renderer);
		
		chunk_position reference_chunk = player->position.chunk_pos;
		tile_position player_tile_pos = get_tile_position(player->position);
		v2 player_tile_offset_in_chunk = get_tile_offset_in_chunk(reference_chunk, player_tile_pos);
		v2 player_offset_in_chunk = get_position_difference(player->position, reference_chunk);
		v2 player_offset_in_tile = player_offset_in_chunk - player_tile_offset_in_chunk;
	
		i32 screen_half_width = SDL_ceil(HALF_SCREEN_WIDTH_IN_TILES) + 2;
		i32 screen_half_height = SDL_ceil(HALF_SCREEN_HEIGHT_IN_TILES) + 2;

		// draw tiles
		for (i32 y_coord_relative = -screen_half_height;
			y_coord_relative < screen_half_height;
			y_coord_relative++)
		{
			i32 y_coord_on_screen = y_coord_relative;;// +screen_half_height;
			i32 y_coord_in_world = player_tile_pos.y + y_coord_relative;

			for (i32 x_coord_relative = -screen_half_width;
				x_coord_relative < screen_half_width;
				x_coord_relative++)
			{
				i32 x_coord_on_screen = x_coord_relative;//+ screen_half_width;
				i32 x_coord_in_world = player_tile_pos.x + x_coord_relative;

				u32 tile_value = get_tile_value(game->current_level, x_coord_in_world, y_coord_in_world);
				SDL_Rect tile_bitmap = get_tile_rect(tile_value);

				v2 position = get_v2(x_coord_on_screen, y_coord_on_screen) - player_offset_in_tile;
				SDL_Rect screen_rect = get_tile_render_rect(position);
				SDL_RenderCopy(sdl_game->renderer, sdl_game->tileset_texture, &tile_bitmap, &screen_rect);

#if 0
				if (is_tile_colliding(game->collision_reference, tile_value))
				{
					tile_position tile_pos = get_tile_position(x_coord_in_world, y_coord_in_world);
					entity_collision_data tile_collision = get_tile_collision_data(player->position.chunk_pos, tile_pos);
					v2 relative_position = get_position_difference(tile_pos, player->position);
					v2 center = relative_position + tile_collision.collision_rect_offset;
					v2 size = tile_collision.collision_rect_dim;
					rect collision_rect = get_rect_from_center(
						SCREEN_CENTER_IN_PIXELS + (center * TILE_SIDE_IN_PIXELS),
						(size * TILE_SIDE_IN_PIXELS));
					render_rect(sdl_game, collision_rect);
				}
#endif
			}
		}

		if (debug_entity_to_render_path)
		{
			render_debug_path_ends(sdl_game, debug_entity_to_render_path, player->position);
		}

		// draw entities
		for (u32 entity_index = 0; entity_index < game->entities_count; entity_index++)
		{
			entity* entity = game->entities + entity_index;
			if (is_in_neighbouring_chunk(player->position.chunk_pos, entity->position))
			{
				render_entity_animation_frame(sdl_game->renderer, player->position, entity);
			}
		}

		// draw bullets
		for (u32 bullet_index = 0; bullet_index < game->bullets_count; bullet_index++)
		{
			bullet* bullet = game->bullets + bullet_index;
			if (is_in_neighbouring_chunk(player->position.chunk_pos, bullet->position))
			{
				render_entity_sprite(sdl_game->renderer,
					player->position, bullet->position, direction::NONE,
					NULL, 0, bullet->type->idle_pose.sprite);
			}
		}

		// draw collision debug info
		{
#if 1
			for (u32 entity_index = 0; entity_index < game->entities_count; entity_index++)
			{
				entity* entity = game->entities + entity_index;
				if (is_in_neighbouring_chunk(player->position.chunk_pos, entity->position))
				{
					if (false == is_zero(entity->type->color))
					{
						debug_breakpoint;
					}

					// istotne - offset sprite'a nie ma tu znaczenia
					v2 relative_position = get_position_difference(entity->position, player->position);
					v2 center = relative_position + entity->type->collision_rect_offset;
					v2 size = entity->type->collision_rect_dim;
					rect collision_rect = get_rect_from_center(
						SCREEN_CENTER_IN_PIXELS + (center * TILE_SIDE_IN_PIXELS), 
						size * TILE_SIDE_IN_PIXELS);
					render_rect(sdl_game, collision_rect);

					SDL_SetRenderDrawColor(sdl_game->renderer, 255, 0, 0, 0);
					v2 entity_position = SCREEN_CENTER_IN_PIXELS + relative_position * TILE_SIDE_IN_PIXELS;
					SDL_RenderDrawPoint(sdl_game->renderer, entity_position.x, entity_position.y);
				}
			}
	
#endif
		}
		
		render_debug_information(sdl_game, game);

		render_hitpoint_bar(sdl_game, player);

		// testowy textbox
#if 0
		{
			rect textbox_area = get_rect_from_center(
				SCREEN_CENTER_IN_PIXELS + get_v2(0, 80),
				get_v2(240, 60));
			SDL_SetRenderDrawColor(sdl_game->renderer, 0, 0, 0, 0);
			SDL_Rect sdl_textbox_rect = get_sdl_rect(textbox_area);
			SDL_RenderFillRect(sdl_game->renderer, &sdl_textbox_rect);
			font font = {};
			font.pixel_height = 8;
			font.pixel_width = 8;
			write(sdl_game->arena, sdl_game, font, textbox_area, sdl_game->test_str);
		}
#endif

		SDL_RenderPresent(sdl_game->renderer);
	}
}

r32 get_elapsed_miliseconds(u32 start_counter, u32 end_counter)
{
	r32 result = ((end_counter - start_counter) * 1000) / (r64)SDL_GetPerformanceFrequency();
	return result;
}

int main(int argc, char* args[])
{
	sdl_game_data sdl_game = init_sdl();
	if (sdl_game.initialized)
	{
		bool run = true;

		u32 memory_for_permanent_arena_size = megabytes_to_bytes(50);
		memory_arena permantent_arena = {};
		void* memory_for_permanent_arena = SDL_malloc(memory_for_permanent_arena_size);
		initialize_memory_arena(&permantent_arena, memory_for_permanent_arena_size, (byte*)memory_for_permanent_arena);
		
		memory_arena transient_arena = {};
		u32 memory_for_transient_arena_size = megabytes_to_bytes(50);
		void* memory_for_transient_arena = SDL_malloc(memory_for_transient_arena_size);
		initialize_memory_arena(&transient_arena, memory_for_transient_arena_size, (byte*)memory_for_transient_arena);

		sdl_game.arena = &permantent_arena;
		sdl_game.transient_arena = &transient_arena;

		//circular_buffer_test(&arena);

		//const char* test_c_str = "calkiem dlugi napis ktory sam sie zawija i w ogole 2137";
		//sdl_game.test_str = c_string_to_string_ref(&permantent_arena, test_c_str);
		
		game_data* game = push_struct(&permantent_arena, game_data);
		game->input.size = 60 * 2; // 2 sekundy
		game->input.buffer = push_array(&permantent_arena, game->input.size, game_input);

		load_game_data(&sdl_game, game, &permantent_arena, &transient_arena);

		u32 frame_counter = 0;

		r32 target_hz = 30;
		r32 target_elapsed_ms = 1000 / target_hz;
		r32 elapsed_work_ms = 0;
		r64 delta_time = 1 / target_hz;

		while (run)
		{
			frame_counter++;

			u32 start_work_counter = SDL_GetPerformanceCounter();
			{
				SDL_Event e = {};
				game_input input = {};
				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						run = false;
					}

					if (e.type == SDL_MOUSEBUTTONDOWN)
					{
						input.fire.number_of_presses++;
					}
				}

				const Uint8* state = SDL_GetKeyboardState(NULL);
				if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W]) input.up.number_of_presses++;
				if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) input.down.number_of_presses++;
				if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) input.left.number_of_presses++;
				if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) input.right.number_of_presses++;

				int mouse_x = -1;
				int mouse_y = -1;
				Uint32 mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
				if (mouse_buttons & SDL_BUTTON_LMASK)
				{
					input.is_left_mouse_key_held = true;
				}

				write_to_input_buffer(&game->input, &input);

				update_and_render(&sdl_game, game, delta_time);
			}
			u32 end_work_counter = SDL_GetPerformanceCounter();

			elapsed_work_ms = get_elapsed_miliseconds(start_work_counter, end_work_counter);
			if (elapsed_work_ms < target_elapsed_ms)
			{
				r32 how_long_to_sleep_ms = target_elapsed_ms - elapsed_work_ms;
				if (how_long_to_sleep_ms > 1)
				{
					SDL_Delay(how_long_to_sleep_ms);
				}

				r32 total_elapsed_ms = get_elapsed_miliseconds(start_work_counter, SDL_GetPerformanceCounter());
				while (target_elapsed_ms > total_elapsed_ms)
				{
					total_elapsed_ms = get_elapsed_miliseconds(start_work_counter, SDL_GetPerformanceCounter());
				}
			}

			sdl_game.debug_elapsed_work_ms = elapsed_work_ms;
			sdl_game.debug_frame_counter = frame_counter + 1;

			debug_breakpoint;
		}
	}
	else
	{
		invalid_code_path;
	}

	check_arena(sdl_game.arena);
	check_arena(sdl_game.transient_arena);

	TTF_CloseFont(sdl_game.font);
	SDL_DestroyTexture(sdl_game.tileset_texture);

	SDL_DestroyRenderer(sdl_game.renderer);
	SDL_DestroyWindow(sdl_game.window);

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();

	return 0;
}

