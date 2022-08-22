#include "main.h"
#include "tmx_parsing.h"
#include "jorutils.h"
#include "jormath.h"

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
			// SDL_RENDERER_ACCELERATED powoduje zawieszenie się komputera - trzeba zbadać sprawę
			sdl_game.renderer = SDL_CreateRenderer(sdl_game.window, -1, SDL_RENDERER_SOFTWARE);
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

#define TILESET_WIDTH 64
#define TILE_X_SIZE 16
#define TILE_Y_SIZE 16

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

		u32 x = column * TILE_X_SIZE;
		u32 y = row * TILE_Y_SIZE;

		tile_rect.x = x;
		tile_rect.y = y;
		tile_rect.w = TILE_X_SIZE;
		tile_rect.h = TILE_Y_SIZE;
	}
	
	return tile_rect;
}

struct tile_pos
{
	u32 x;
	u32 y;
};

tile_pos get_tile_pos(u32 x_coord, u32 y_coord)
{
	tile_pos result = {};
	result.x = x_coord;
	result.y = y_coord;
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

b32 move(level map, level collision_ref, v2 player_pos, v2 target_pos)
{
	b32 result = true;

	u32 x_coord = (u32)target_pos.x;
	u32 y_coord = (u32)target_pos.y;

	debug_breakpoint;

	u32 target_tile_value = get_tile_value(map, x_coord, y_coord);
	if (is_tile_colliding(collision_ref, target_tile_value))
	{
		result = false;
	}

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

		std::string collision_file_path = "data/collision_map.tmx";
		read_file_result collision_file = read_file(collision_file_path);
		level collision_ref = read_level_from_tmx_file(&arena, collision_file, "collision");

		std::string map_file_path = "data/trifle_map_01.tmx";
		read_file_result map_file = read_file(map_file_path);
		level map = read_level_from_tmx_file(&arena, map_file, "map");

		v2 player_pos = {5, 5};
		r32 player_speed = 0.10f;

		v2 target_pos = player_pos;
		
		while (run)
		{
			game_input input = {};

			target_pos = player_pos;

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
							target_pos.y = player_pos.y - player_speed;
							//printf("UP\n");
							break;
						case SDLK_DOWN:
						case SDLK_s:
							input.down.number_of_presses++;
							target_pos.y = player_pos.y + player_speed;
							//printf("DOWN\n");
							break;
						case SDLK_LEFT:
						case SDLK_a:
							input.left.number_of_presses++;
							target_pos.x = player_pos.x - player_speed;
							//printf("LEFT\n");
							break;
						case SDLK_RIGHT:
						case SDLK_d:
							input.right.number_of_presses++;							
							target_pos.x = player_pos.x + player_speed;
							//printf("RIGHT\n");
							break;
						default:
							break;
					}
				}		
			}

			if (move(map, collision_ref, player_pos, target_pos))
			{
				player_pos = target_pos;
			}

			SDL_Texture* texture_to_draw = sdl_game.tileset_texture;
			SDL_RenderClear(sdl_game.renderer);

			for (u32 y_coord = 0; y_coord < 20; y_coord++)
			{
				for (u32 x_coord = 0; x_coord < 20; x_coord++)
				{
					u32 tile_in_map_index = x_coord + (map.width * y_coord);
					u32 tile_value = map.tiles[tile_in_map_index];
					SDL_Rect tile_bitmap = get_tile_rect(tile_value);

					SDL_Rect screen_rect = {};
					screen_rect.h = 16;
					screen_rect.w = 16;
					screen_rect.x = x_coord * 16;
					screen_rect.y = y_coord * 16;

					SDL_RenderCopy(sdl_game.renderer, texture_to_draw, &tile_bitmap, &screen_rect);
				}
			}

			SDL_Rect tile_bitmap = get_tile_rect(1);
			SDL_Rect player_rect = {};
			player_rect.h = 16;
			player_rect.w = 16;
			player_rect.x = (player_pos.x * 16) - 8;
			player_rect.y = (player_pos.y * 16) - 16;
			SDL_RenderCopy(sdl_game.renderer, texture_to_draw, &tile_bitmap, &player_rect);
			
			{
				char buffer[100];
				SDL_Color text_color = { 0, 0, 0, 0 };
				int error = SDL_snprintf(buffer, 100, "Player pos: (%0.2f,%0.2f)", player_pos.x, player_pos.y);
				render_text(sdl_game, buffer, 0, 350, text_color);
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

