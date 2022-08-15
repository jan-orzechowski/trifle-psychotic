#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <stdio.h>
#include <string>

//Screen dimension constants
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define assert(statement) {if (!statement) { int x = 1; x = x / 0; }}
#define invalid_code_path { int x = 1; x = x / 0; }

struct sdl_game_data
{
	bool initialized;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* tileset_texture;
};

void print_sdl_error()
{
	const char* error = SDL_GetError();
	printf("SDL Error: %s\n", error);
	invalid_code_path;
}

void print_sdl_image_error()
{
	const char* error = IMG_GetError();
	printf("SDL_image Error: %s\n", IMG_GetError());
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

struct key_press
{
	int number_of_presses;
};

struct game_input
{
	key_press up;
	key_press down;
	key_press left;
	key_press right;
};

int main(int argc, char* args[])
{
	sdl_game_data sdl_game = init_sdl();
	if (sdl_game.initialized)
	{
		bool run = true;

		SDL_Event e;

		while (run)
		{
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
							printf("UP\n");
							break;
						case SDLK_DOWN:
						case SDLK_s:
							input.down.number_of_presses++;
							printf("DOWN\n");
							break;
						case SDLK_LEFT:
						case SDLK_a:
							input.left.number_of_presses++;
							printf("LEFT\n");
							break;
						case SDLK_RIGHT:
						case SDLK_d:
							input.right.number_of_presses++;
							printf("RIGHT\n");
							break;
						default:
							break;
					}
				}		
			}

			SDL_Texture* texture_to_draw = sdl_game.tileset_texture;

			SDL_RenderClear(sdl_game.renderer);
			SDL_RenderCopy(sdl_game.renderer, texture_to_draw, NULL, NULL);
			SDL_RenderPresent(sdl_game.renderer);
		}
	}
	else
	{			
		invalid_code_path;
	}

	SDL_DestroyTexture(sdl_game.tileset_texture);

	SDL_DestroyRenderer(sdl_game.renderer);
	SDL_DestroyWindow(sdl_game.window);

	IMG_Quit();
	SDL_Quit();

	return 0;
}