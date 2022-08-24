#include "main.h"
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
			sdl_game.renderer = get_renderer(sdl_game.window);
			if (sdl_game.renderer)
			{
				SDL_SetRenderDrawColor(sdl_game.renderer, 0xFF, 0xFF, 0xFF, 0xFF);

				int img_flags = IMG_INIT_PNG;
				if (IMG_Init(img_flags) & img_flags)
				{
					SDL_Surface* loaded_surface = IMG_Load("gfx/surt_tileset.png");
					if (loaded_surface)
					{
						SDL_Texture* tileset = SDL_CreateTextureFromSurface(sdl_game.renderer, loaded_surface);
						if (tileset)
						{
							sdl_game.tileset_texture = tileset;
						}
						else
						{
							print_sdl_error();
							success = false;
						}
						SDL_FreeSurface(loaded_surface);
					}
					else
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
					TTF_Font* font = TTF_OpenFont("gfx/kenney_pixel.ttf", 20);
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

void render_text(sdl_game_data sdl_game, std::string textureText, int x, int y, SDL_Color color)
{
	SDL_Surface* text_surface = TTF_RenderText_Solid(sdl_game.font, textureText.c_str(), color);
	if (text_surface)
	{
		SDL_Texture* font_texture = SDL_CreateTextureFromSurface(sdl_game.renderer, text_surface);
		if (font_texture)
		{
			SDL_Rect dest = {};
			dest.w = text_surface->w;
			dest.h = text_surface->h;
			dest.x = x;
			dest.y = y;

			SDL_RenderCopy(sdl_game.renderer, font_texture, NULL, &dest);
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

u32 get_tile_value(level map, u32 x_coord, u32 y_coord)
{
	u32 result = 0;
	if (x_coord < map.width && y_coord < map.height)
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
	u32 first_gid = 3137; // tymczasowe, potrzebna jest obsługa GID w tmx
	collision_tile_value -= first_gid;
	collision_tile_value++;

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

		// collision with tiles
		{
			v2 tile_collision_rect_dim = get_v2(1.0f, 1.0f);
			
			tile_position player_tile = get_tile_position(*player_pos);
			tile_position target_tile = get_tile_position(target_pos);

			i32 min_tile_x_to_check = min(player_tile.x - 1, target_tile.x - 1);
			i32 min_tile_y_to_check = min(player_tile.y - 1, target_tile.y - 1);
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
						v2 relative_player_pos = *player_pos - tile_to_check_pos;

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
					v2 relative_player_pos = *player_pos - entity->position;

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
				}
			}
		}

		if (was_intersection)
		{
			printf("kolizja\n");
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

		SDL_Event e = {};

		game_data* game = push_struct(&arena, game_data);

		std::string collision_file_path = "data/collision_map.tmx";
		read_file_result collision_file = read_file(collision_file_path);
		game->collision_reference = read_level_from_tmx_file(&arena, collision_file, "collision");

		std::string map_file_path = "data/trifle_map_01.tmx";
		read_file_result map_file = read_file(map_file_path);
		game->current_level = read_level_from_tmx_file(&arena, map_file, "map");

		game->player_pos = {0, 0};
		game->player_speed = 0.4f;
		game->player_collision_rect_dim = get_v2(1.0f, 1.0f);

		game->entity_types_count = 5;
		game->entity_types = push_array(&arena, game->entity_types_count, entity_type);
			
		game->entities_count = 0;
		game->entities_max_count = 1000;
		game->entities = push_array(&arena, game->entities_max_count, entity);

		entity_type* default_entity_type = &game->entity_types[0];
		default_entity_type->collision_rect_dim = get_v2(1.0f, 1.0f);
		default_entity_type->graphics = get_tile_rect(837);
		default_entity_type->collides = true;

		add_entity(game, get_v2(2.0f, 2.0f), default_entity_type);
		add_entity(game, get_v2(4.0f, 4.0f), default_entity_type);

		v2 target_pos = {};
		
		while (run)
		{
			game_input input = {};

			target_pos = game->player_pos;

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
							target_pos.y = game->player_pos.y - game->player_speed;
							//printf("UP\n");
							break;
						case SDLK_DOWN:
						case SDLK_s:
							input.down.number_of_presses++;
							target_pos.y = game->player_pos.y + game->player_speed;
							//printf("DOWN\n");
							break;
						case SDLK_LEFT:
						case SDLK_a:
							input.left.number_of_presses++;
							target_pos.x = game->player_pos.x - game->player_speed;
							//printf("LEFT\n");
							break;
						case SDLK_RIGHT:
						case SDLK_d:
							input.right.number_of_presses++;							
							target_pos.x = game->player_pos.x + game->player_speed;
							//printf("RIGHT\n");
							break;
						default:
							break;
					}
				}		
			}

			move(game, &game->player_pos, target_pos);

			SDL_Texture* texture_to_draw = sdl_game.tileset_texture;
			SDL_RenderClear(sdl_game.renderer);

			for (u32 y_coord = 0; y_coord < 20; y_coord++)
			{
				for (u32 x_coord = 0; x_coord < 20; x_coord++)
				{
					u32 tile_in_map_index = x_coord + (game->current_level.width * y_coord);
					u32 tile_value = game->current_level.tiles[tile_in_map_index];
					SDL_Rect tile_bitmap = get_tile_rect(tile_value);

					SDL_Rect screen_rect = get_tile_render_rect(get_v2(x_coord, y_coord));
					SDL_RenderCopy(sdl_game.renderer, texture_to_draw, &tile_bitmap, &screen_rect);
				}
			}

			for (u32 entity_index = 0; entity_index < game->entities_count; entity_index++)
			{
				entity* entity = game->entities + entity_index;
				SDL_Rect entity_bitmap = entity->type->graphics;
				SDL_Rect entity_rect = get_tile_render_rect(entity->position);
				SDL_RenderCopy(sdl_game.renderer, texture_to_draw, &entity_bitmap, &entity_rect);
			}

			SDL_Rect player_bitmap = get_tile_rect(1);
			SDL_Rect player_rect = get_tile_render_rect(game->player_pos);
			SDL_RenderCopy(sdl_game.renderer, texture_to_draw, &player_bitmap, &player_rect);
	
			{
				char buffer[100];
				SDL_Color text_color = { 255, 255, 255, 0 };
				int error = SDL_snprintf(buffer, 100, "Player pos: (%0.2f,%0.2f)", game->player_pos.x, game->player_pos.y);
				render_text(sdl_game, buffer, 0, 290, text_color);
			}

			SDL_RenderPresent(sdl_game.renderer);
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

