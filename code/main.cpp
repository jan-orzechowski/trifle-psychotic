﻿#include "main.h"
#include "tmx_parsing.h"

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

SDL_Renderer* get_renderer(SDL_Window* window)
{
	SDL_Renderer* renderer = NULL;
	for (int driver_index = 0; driver_index < SDL_GetNumRenderDrivers(); ++driver_index)
	{
		SDL_RendererInfo rendererInfo = {};
		SDL_GetRenderDriverInfo(driver_index, &rendererInfo);
		// direct3d11 i direct3d powoduje freeze
		if (rendererInfo.name != std::string("direct3d11"))
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

SDL_Texture* load_image(SDL_Renderer* renderer, const char* path)
{
	SDL_Texture* result = NULL;
	SDL_Surface* loaded_surface = IMG_Load(path);
	if (loaded_surface)
	{
		result  = SDL_CreateTextureFromSurface(renderer, loaded_surface);		
		SDL_FreeSurface(loaded_surface);
	}
	return result;
}

sdl_game_data init_sdl()
{
	sdl_game_data sdl_game = {};
	bool success = true;

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
					sdl_game.tileset_texture = load_image(sdl_game.renderer, "gfx/tileset.png");
					if (sdl_game.tileset_texture == NULL)
					{
						print_sdl_image_error();
						success = false;
					}

					sdl_game.player_texture = load_image(sdl_game.renderer, "gfx/player.png");
					if (sdl_game.player_texture == NULL)
					{
						print_sdl_image_error();
						success = false;
					}
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

void render_text(sdl_game_data* sdl_game, std::string textureText, int x, int y, SDL_Color color)
{
	SDL_Surface* text_surface = TTF_RenderText_Solid(sdl_game->font, textureText.c_str(), color);
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

read_file_result read_file(std::string path)
{
	read_file_result result = {};
	
	SDL_RWops* file = SDL_RWFromFile(path.c_str(), "r");
	if (file)		
	{
		int64_t file_size = SDL_RWsize(file);
		if (file_size != -1 // błąd
			&& file_size < 1024*1024*5) // zabezpieczenie
		{
			result.size = file_size;
			result.contents = new char[file_size + 1]; // dodatkowy bajt na końcu przyda się przy parsowaniu
			for (int byte_index = 0; 
				byte_index < file_size; 
				++byte_index)
			{
				SDL_RWread(file, (void*)((char*)result.contents + byte_index), sizeof(char), 1);
			}
			*((char*)result.contents + file_size) = 0;
		}
		else
		{
			print_sdl_error();
		}

		SDL_RWclose(file);
	}
	else
	{
		print_sdl_error();
	}

	return result;
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

tile_position get_tile_position(u32 tile_x, u32 tile_y)
{
	tile_position result = {};
	result.x = tile_x;
	result.y = tile_y;
	return result;
}

tile_position get_tile_position(v2 world_position)
{
	tile_position result = {};
	// każde pole ma środek w pełnych współrzędnych, np. (1, 1) i ma szerokość boku 1
	// tak więc zamieniamy np. (0.6, 0.6) -> (1, 1), (1.6, 1.6) -> (2, 2)
	result.x = (u32)(world_position.x + 0.5f);
	result.y = (u32)(world_position.y + 0.5f);
	return result;
}

v2 get_tile_v2_position(u32 tile_x, u32 tile_y)
{
	v2 result = get_v2(tile_x, tile_y);
	return result;
}

v2 get_tile_v2_position(tile_position tile)
{
	v2 result = get_tile_v2_position(tile.x, tile.y);
	return result;
}

u32 get_tile_value(level map, i32 x_coord, i32 y_coord)
{
	u32 result = 0;
	if (x_coord >= 0
		&& y_coord >= 0
		&& x_coord < map.width 
		&& y_coord < map.height)
	{
		u32 tile_index = x_coord + (map.width * y_coord);
		result = map.tiles[tile_index];
	}
	return result;	
}

b32 is_tile_colliding(level collision_ref_level, u32 tile_value)
{
	u32 x_coord = (tile_value - 1) % collision_ref_level.width;
	u32 y_coord = (tile_value - 1) / collision_ref_level.width;

	debug_breakpoint;

	b32 collides = false;
	u32 collision_tile_value = get_tile_value(collision_ref_level, x_coord, y_coord);
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

v2 get_standing_collision_rect_offset(v2 collision_rect_dim)
{
	// zakładamy że wszystkie obiekty mają pozycję na środku pola, czyli 0.5f nad górną krawędzią pola pod nimi
	v2 offset = get_zero_v2();
	offset.y = -(collision_rect_dim.y / 2);
	return offset;
}

b32 is_standing_on_ground(level map, level collision_ref, v2 position, v2 collision_rect_dim)
{
	b32 result = false;
	r32 corner_distance_apron = 0.1f;
	r32 max_distance_to_tile = 0.05f;

	v2 left_corner_position = get_v2(
		position.x - collision_rect_dim.x / 2, 
		position.y + 0.5f - corner_distance_apron);
	v2 right_corner_position = get_v2(
		position.x + collision_rect_dim.x / 2,
		position.y + 0.5f - corner_distance_apron);

	tile_position left_tile_pos = get_tile_position(left_corner_position);
	tile_position right_tile_pos = get_tile_position(right_corner_position);

	u32 tile_under_left_corner = get_tile_value(map, left_tile_pos.x, left_tile_pos.y + 1);
	u32 tile_under_right_corner = get_tile_value(map, right_tile_pos.x, right_tile_pos.y + 1);
	
	if (is_tile_colliding(collision_ref, tile_under_left_corner))
	{
		r32 distance_to_tile = get_tile_v2_position(left_tile_pos.x, left_tile_pos.y + 1).y
			- left_corner_position.y - corner_distance_apron - 0.5f;
		if (distance_to_tile < max_distance_to_tile)
		{
			result = true;
		}
	}

	if (is_tile_colliding(collision_ref, tile_under_right_corner))
	{
		r32 distance_to_tile = get_tile_v2_position(right_tile_pos.x, right_tile_pos.y + 1).y
			- right_corner_position.y - corner_distance_apron - 0.5f;
		if (distance_to_tile < max_distance_to_tile)
		{
			result = true;
		}
	}

	return result;
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
b32 check_segment_intersection (r32 movement_start_x, r32 movement_start_y, 
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

entity* add_entity(game_data* game, v2 position, entity_type* type)
{
	assert(game->entities_count + 1 < game->entities_max_count);

	entity* new_entity = &game->entities[game->entities_count];
	game->entities_count++;
	new_entity->position = position;
	new_entity->type = type;
	return new_entity;
}

void move(game_data* game, v2* player_pos, v2 target_pos)
{	
	v2 player_delta = target_pos - *player_pos;
	if (false == is_zero(player_delta))
	{
		r32 movement_apron = 0.0001f;
		r32 min_movement_perc = 1.0f;
		b32 was_intersection = false;
		v2 collided_wall_normal = {};

		v2 goal_position = *player_pos + player_delta;

		// collision with tiles
		{
			v2 tile_collision_rect_dim = get_v2(1.0f, 1.0f);
			
			tile_position player_tile = get_tile_position(*player_pos);
			tile_position target_tile = get_tile_position(target_pos);

			// -2 w y ze względu na wysokość gracza i offset - gracz wystaje do góry na dwa pola
			// przydałoby się to obliczać na podstawie wielkości gracza, bez magicznych stałych
			i32 min_tile_x_to_check = min(player_tile.x - 1, target_tile.x - 1);
			i32 min_tile_y_to_check = min(player_tile.y - 2, target_tile.y - 2);
			i32 max_tile_x_to_check = max(player_tile.x + 1, target_tile.x + 1);
			i32 max_tile_y_to_check = max(player_tile.y + 1, target_tile.y + 1);

			for (i32 tile_y_to_check = min_tile_y_to_check;
				tile_y_to_check <= max_tile_y_to_check;
				tile_y_to_check++)
			{
				for (i32 tile_x_to_check = min_tile_x_to_check;
					tile_x_to_check <= max_tile_x_to_check;
					tile_x_to_check++)
				{
					u32 tile_value = get_tile_value(game->current_level, tile_x_to_check, tile_y_to_check);
					v2 tile_to_check_pos = get_tile_v2_position(get_tile_position(tile_x_to_check, tile_y_to_check));
					if (is_tile_colliding(game->collision_reference, tile_value))
					{
						v2 relative_player_pos = (*player_pos + game->player_collision_rect_offset)
							- tile_to_check_pos;

						// środkiem zsumowanej figury jest (0,0,0)
						// pozycję playera traktujemy jako odległość od 0
						// 0 jest pozycją entity, z którym sprawdzamy kolizję

						v2 minkowski_dimensions = game->player_collision_rect_dim + tile_collision_rect_dim;
						v2 min_corner = minkowski_dimensions * -0.5f;
						v2 max_corner = minkowski_dimensions * 0.5f;

						b32 west = check_segment_intersection(
							relative_player_pos.x, relative_player_pos.y, player_delta.x, player_delta.y,
							min_corner.x, min_corner.y, max_corner.y, &min_movement_perc); // ściana od zachodu

						b32 east = check_segment_intersection(
							relative_player_pos.x, relative_player_pos.y, player_delta.x, player_delta.y,
							max_corner.x, min_corner.y, max_corner.y, &min_movement_perc); // ściana od wschodu

						b32 north = check_segment_intersection(
							relative_player_pos.y, relative_player_pos.x, player_delta.y, player_delta.x,
							max_corner.y, min_corner.x, max_corner.x, &min_movement_perc); // ściana od północy

						b32 south = check_segment_intersection(
							relative_player_pos.y, relative_player_pos.x, player_delta.y, player_delta.x,
							min_corner.y, min_corner.x, max_corner.x, &min_movement_perc); // ściana od południa

						was_intersection = (was_intersection || west || east || north || south);

						if (west)
						{
							collided_wall_normal = get_v2(-1, 0);
						}
						else if (east)
						{
							collided_wall_normal = get_v2(1, 0);
						}
						else if (north)
						{
							collided_wall_normal = get_v2(0, 1);
						}
						else if (south)
						{
							collided_wall_normal = get_v2(0, -1);
						}
					}
				}
			}
		}

		// collision with entities
		{			
			for (u32 entity_index = 0; entity_index < game->entities_count; entity_index++)
			{
				entity* entity = game->entities + entity_index;		
				if (entity->type->collides)
				{
					v2 relative_player_pos = (*player_pos + game->player_collision_rect_offset) 
						- (entity->position + entity->type->collision_rect_offset);

					v2 minkowski_dimensions = game->player_collision_rect_dim + entity->type->collision_rect_dim;
					v2 min_corner = minkowski_dimensions * -0.5f;
					v2 max_corner = minkowski_dimensions * 0.5f;

					b32 west = check_segment_intersection(
						relative_player_pos.x, relative_player_pos.y, player_delta.x, player_delta.y,
						min_corner.x, min_corner.y, max_corner.y, &min_movement_perc); // ściana od zachodu

					b32 east = check_segment_intersection(
						relative_player_pos.x, relative_player_pos.y, player_delta.x, player_delta.y,
						max_corner.x, min_corner.y, max_corner.y, &min_movement_perc); // ściana od wschodu

					b32 north = check_segment_intersection(
						relative_player_pos.y, relative_player_pos.x, player_delta.y, player_delta.x,
						max_corner.y, min_corner.x, max_corner.x, &min_movement_perc); // ściana od północy

					b32 south = check_segment_intersection(
						relative_player_pos.y, relative_player_pos.x, player_delta.y, player_delta.x,
						min_corner.y, min_corner.x, max_corner.x, &min_movement_perc); // ściana od południa

					was_intersection = (was_intersection || west || east || north || south);

					if (west)
					{
						collided_wall_normal = get_v2(-1, 0);
					}
					else if (east)
					{
						collided_wall_normal = get_v2(1, 0);
					}
					else if (north)
					{
						collided_wall_normal = get_v2(0, 1);
					}
					else if (south)
					{
						collided_wall_normal = get_v2(0, -1);
					}					
				}
			}
		}

		if (was_intersection)
		{
			i32 how_many_times_subtract = 1; // 1 dla ślizgania się, 2 dla odbijania    

			v2 bounced = collided_wall_normal * inner(collided_wall_normal, game->player_velocity);
			game->player_velocity -= how_many_times_subtract * bounced;

			// zmniejszamy deltę o tyle, o ile udało nam się poruszyć oraz o fragment delty, który wchodzi w ścianę
			player_delta = goal_position - game->player_pos;
			player_delta -= how_many_times_subtract * (collided_wall_normal * inner(player_delta, collided_wall_normal));

			debug_breakpoint;
		}

		if ((min_movement_perc - movement_apron) > 0.0f)
		{
			v2 possible_movement = player_delta * (min_movement_perc - movement_apron);
			*player_pos += possible_movement;
		}
	}
}

// kształt każdego obiektu w grze ma środek w pozycji w świecie tego obiektu
SDL_Rect get_render_rect(v2 position, rect entity_rect)
{
	v2 entity_rect_dim = get_rect_dimensions(entity_rect);
	SDL_Rect result = {};
	result.w = entity_rect_dim.x;
	result.h = entity_rect_dim.y;	
	result.x = (position.x * entity_rect_dim.x) - (entity_rect_dim.x / 2);
	result.y = (position.y * entity_rect_dim.y) - (entity_rect_dim.y / 2);
	return result;
}

// każde pole ma środek w miejscu o pełnych współrzędnych, np. (1, 1)
SDL_Rect get_tile_render_rect(v2 position)
{
	SDL_Rect result = {};
	result.w = TILE_SIDE_IN_PIXELS;
	result.h = TILE_SIDE_IN_PIXELS;
	result.x = (position.x * TILE_SIDE_IN_PIXELS) - (TILE_SIDE_IN_PIXELS / 2);
	result.y = (position.y * TILE_SIDE_IN_PIXELS) - (TILE_SIDE_IN_PIXELS / 2);
	return result;
}

void render_debug_information(sdl_game_data* sdl_game, game_data* game)
{
	b32 is_standing = is_standing_on_ground(game->current_level, game->collision_reference, game->player_pos, game->player_collision_rect_dim);

	char buffer[200];
	SDL_Color text_color = { 255, 255, 255, 0 };
	/*int error = SDL_snprintf(buffer, 200, "Frame: %d Elapsed: %0.2f ms, Pos: (%0.2f,%0.2f) Acc: (%0.2f,%0.2f) Standing: %d ",
		sdl_game->debug_frame_counter, sdl_game->debug_elapsed_work_ms, game->player_pos.x, game->player_pos.y,
		game->player_acceleration.x, game->player_acceleration.y, is_standing);*/
	int error = SDL_snprintf(buffer, 200, "Pos: (%0.2f,%0.2f) Acc: (%0.2f,%0.2f) Standing: %d ",
		game->player_pos.x, game->player_pos.y,
		game->player_acceleration.x, game->player_acceleration.y, is_standing);
	render_text(sdl_game, buffer, 10, 100, text_color);
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

void update_and_render(sdl_game_data* sdl_game, game_data* game, game_input input, r32 delta_time)
{
	//v2 gravity = get_v2(0, 2.0f);

	game->player_acceleration = get_zero_v2();
	if (input.up.number_of_presses > 0)
	{
		game->player_acceleration += get_v2(0, -1);
	}
	else if (input.down.number_of_presses > 0)
	{
		game->player_acceleration += get_v2(0, 1);
	}

	if (input.left.number_of_presses > 0)
	{
		game->player_acceleration += get_v2(-1, 0);
	}
	else if (input.right.number_of_presses > 0)
	{
		game->player_acceleration += get_v2(1, 0);
	}

	/*if (false == is_zero(game->player_acceleration) 
		&& length(game->player_velocity) < 0.01f)
	{
		game->player_velocity = 5.0f * game->player_acceleration * delta_time;
		printf("przyspieszenie\n");
	}*/
	//else
	//{
		game->player_velocity = game->player_slowdown_multiplier *
			(game->player_velocity + (game->player_acceleration * delta_time));
	//}
	
	v2 target_pos = game->player_pos + 
		(game->player_velocity * game->player_velocity_multiplier * delta_time);

	move(game, &game->player_pos, target_pos);

	SDL_SetRenderDrawColor(sdl_game->renderer, 0, 255, 0, 0);
	SDL_RenderClear(sdl_game->renderer);

	tile_position player_tile_pos = get_tile_position(game->player_pos);	
	v2 player_offset_in_tile = get_v2(
		game->player_pos.x - (r32)player_tile_pos.x, 
		game->player_pos.y - (r32)player_tile_pos.y);

	i32 center_screen_x = player_tile_pos.x;
	i32 center_screen_y = player_tile_pos.y;
	i32 screen_half_width = SDL_ceil(HALF_SCREEN_WIDTH_IN_TILES) + 1;
	i32 screen_half_height = SDL_ceil(HALF_SCREEN_HEIGHT_IN_TILES) + 1;

	for (i32 y_coord_relative = -screen_half_height;
		y_coord_relative < screen_half_height;
		y_coord_relative++)
	{
		i32 y_coord_on_screen = y_coord_relative + screen_half_height;
		i32 y_coord_in_world = player_tile_pos.y + y_coord_relative;
		for (i32 x_coord_relative = -screen_half_width;
			x_coord_relative < screen_half_width;
			x_coord_relative++)
		{
			i32 x_coord_on_screen = x_coord_relative + screen_half_width;
			i32 x_coord_in_world = player_tile_pos.x + x_coord_relative;

			u32 tile_value = get_tile_value(game->current_level, x_coord_in_world, y_coord_in_world);
			SDL_Rect tile_bitmap = get_tile_rect(tile_value);

			SDL_Rect screen_rect = get_tile_render_rect(
				get_v2(x_coord_on_screen, y_coord_on_screen) - player_offset_in_tile);
			SDL_RenderCopy(sdl_game->renderer, sdl_game->tileset_texture, &tile_bitmap, &screen_rect);
		}
	}

	v2 center_screen = get_v2(HALF_SCREEN_WIDTH_IN_TILES + 1, HALF_SCREEN_HEIGHT_IN_TILES + 1);
	for (u32 entity_index = 0; entity_index < game->entities_count; entity_index++)
	{
		entity* entity = game->entities + entity_index;
		SDL_Rect entity_bitmap = entity->type->graphics;
		v2 relative_position = center_screen + (entity->position - game->player_pos);
		SDL_Rect screen_rect = get_tile_render_rect(relative_position);
		SDL_RenderCopy(sdl_game->renderer, sdl_game->tileset_texture, &entity_bitmap, &screen_rect);
	}

	SDL_Rect player_head_bitmap = {};
	player_head_bitmap.x = 0;
	player_head_bitmap.y = 0;
	player_head_bitmap.w = 24;
	player_head_bitmap.h = 24;

	SDL_Rect player_legs_bitmap = {};
	player_legs_bitmap.x = 0;
	player_legs_bitmap.y = 24;
	player_legs_bitmap.w = 24;
	player_legs_bitmap.h = 24;

	SDL_Rect player_legs_render_rect = {};
	player_legs_render_rect.w = 24;
	player_legs_render_rect.h = 24;
	player_legs_render_rect.x = (screen_half_width * TILE_SIDE_IN_PIXELS) - 24/2;
	player_legs_render_rect.y = (screen_half_height * TILE_SIDE_IN_PIXELS) - 24/2 - 4;

	v2 screen_half_size = get_v2(screen_half_width, screen_half_height) * TILE_SIDE_IN_PIXELS;

	SDL_Rect player_head_render_rect = player_legs_render_rect;
	player_head_render_rect.x += 4;
	player_head_render_rect.y += -16;

	SDL_RenderCopy(sdl_game->renderer, sdl_game->player_texture, &player_legs_bitmap, &player_legs_render_rect);
	SDL_RenderCopy(sdl_game->renderer, sdl_game->player_texture, &player_head_bitmap, &player_head_render_rect);

	rect current_player_collision_rect = get_rect_from_center(
		screen_half_size + (game->player_collision_rect_offset * TILE_SIDE_IN_PIXELS), 
		game->player_collision_rect_dim * TILE_SIDE_IN_PIXELS);
	render_rect(sdl_game, current_player_collision_rect);

	render_debug_information(sdl_game, game);

	SDL_RenderPresent(sdl_game->renderer);
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

		memory_arena arena = {};
		u32 memory_for_permanent_arena_size = megabytes_to_bytes(100);
		void* memory_for_permanent_arena = SDL_malloc(memory_for_permanent_arena_size);
		initialize_memory_arena(&arena, memory_for_permanent_arena_size, (byte*)memory_for_permanent_arena);

		game_data* game = push_struct(&arena, game_data);

		std::string collision_file_path = "data/collision_map.tmx";
		read_file_result collision_file = read_file(collision_file_path);
		game->collision_reference = read_level_from_tmx_file(&arena, collision_file, "collision");

		std::string map_file_path = "data/map_01.tmx";
		read_file_result map_file = read_file(map_file_path);
		game->current_level = read_level_from_tmx_file(&arena, map_file, "map");

		game->player_pos = {0, 0};
		game->player_velocity_multiplier = 40.0f;
		game->player_slowdown_multiplier = 0.80f;
		game->player_collision_rect_dim = get_v2(0.5f, 2.0f); //get_v2(0.5f, 2.0f);
		game->player_collision_rect_offset = 
			get_standing_collision_rect_offset(game->player_collision_rect_dim);

		game->entity_types_count = 5;
		game->entity_types = push_array(&arena, game->entity_types_count, entity_type);
			
		game->entities_count = 0;
		game->entities_max_count = 1000;
		game->entities = push_array(&arena, game->entities_max_count, entity);

		entity_type* default_entity_type = &game->entity_types[0];
		default_entity_type->graphics = get_tile_rect(837);
		default_entity_type->collides = true;
		default_entity_type->collision_rect_dim = get_v2(1.0f, 1.0f);
		game->player_collision_rect_offset =
			get_standing_collision_rect_offset(default_entity_type->collision_rect_dim);

		add_entity(game, get_v2(2.0f, 2.0f), default_entity_type);
		add_entity(game, get_v2(4.0f, 4.0f), default_entity_type);

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
					else if (e.type == SDL_KEYDOWN)
					{
						switch (e.key.keysym.sym)
						{
							case SDLK_UP:
							case SDLK_w:
								input.up.number_of_presses++;
								break;
							case SDLK_DOWN:
							case SDLK_s:
								input.down.number_of_presses++;
								break;
							case SDLK_LEFT:
							case SDLK_a:
								input.left.number_of_presses++;
								break;
							case SDLK_RIGHT:
							case SDLK_d:
								input.right.number_of_presses++;
								break;
							default:
								break;
						}
					}
				}

				update_and_render(&sdl_game, game, input, delta_time);
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

		delete map_file.contents;
	}
	else
	{			
		invalid_code_path;
	}

	TTF_CloseFont(sdl_game.font);
	SDL_DestroyTexture(sdl_game.tileset_texture);

	SDL_DestroyRenderer(sdl_game.renderer);
	SDL_DestroyWindow(sdl_game.window);

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	
	return 0;
}

