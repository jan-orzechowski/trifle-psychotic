#include "main.h"
#include "game_data.h"

r32 get_elapsed_miliseconds(u32 start_counter, u32 end_counter)
{
	r32 result = ((end_counter - start_counter) * 1000) / (r64)SDL_GetPerformanceFrequency();
	return result;
}

SDL_Color get_sdl_color(v4 color)
{
	SDL_Color result = { (Uint8)color.r, (Uint8)color.g, (Uint8)color.b, (Uint8)color.a };
	return result;
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
					load_image(sdl_game.renderer, &sdl_game.tileset_texture, "gfx/tileset.png", &success);
					load_image(sdl_game.renderer, &sdl_game.player_texture, "gfx/player.png", &success);
					load_image(sdl_game.renderer, &sdl_game.bullets_texture, "gfx/bullets.png", &success);
					load_image(sdl_game.renderer, &sdl_game.gates_texture, "gfx/gates.png", &success);
					load_image(sdl_game.renderer, &sdl_game.misc_texture, "gfx/misc.png", &success);
					load_image(sdl_game.renderer, &sdl_game.ui_texture, "gfx/interface.png", &success);
					load_image(sdl_game.renderer, &sdl_game.font_texture, "gfx/font.png", &success);
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

int main(int argc, char* args[])
{
	sdl_game_data sdl_game = init_sdl();
	if (sdl_game.initialized)
	{
		bool run = true;

		u32 memory_for_permanent_arena_size = megabytes_to_bytes(50);
		memory_arena permanent_arena = {};
		void* memory_for_permanent_arena = SDL_malloc(memory_for_permanent_arena_size);
		initialize_memory_arena(&permanent_arena, memory_for_permanent_arena_size, (byte*)memory_for_permanent_arena);

		memory_arena transient_arena = {};
		u32 memory_for_transient_arena_size = megabytes_to_bytes(50);
		void* memory_for_transient_arena = SDL_malloc(memory_for_transient_arena_size);
		initialize_memory_arena(&transient_arena, memory_for_transient_arena_size, (byte*)memory_for_transient_arena);

		sdl_game.arena = &permanent_arena;
		sdl_game.transient_arena = &transient_arena;

		//circular_buffer_test(&arena);

		const char* test_c_str = "calkiem dlugi napis ktory sam sie zawija i w ogole 2137";
		sdl_game.test_str = copy_c_string_to_memory_arena(&permanent_arena, test_c_str);

		static_game_data* static_data = push_struct(&permanent_arena, static_game_data);

		load_static_game_data(&sdl_game, static_data, &permanent_arena, &transient_arena);
		input_buffer input_buffer = initialize_input_buffer(&permanent_arena);

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
				game_input new_input = {};
				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						run = false;
					}

					if (e.type == SDL_MOUSEBUTTONDOWN)
					{
						new_input.fire.number_of_presses++;
					}
				}

				const Uint8* state = SDL_GetKeyboardState(NULL);
				if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W]) new_input.up.number_of_presses++;
				if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) new_input.down.number_of_presses++;
				if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) new_input.left.number_of_presses++;
				if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) new_input.right.number_of_presses++;

				int mouse_x = -1;
				int mouse_y = -1;
				Uint32 mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
				if (mouse_buttons & SDL_BUTTON_LMASK)
				{
					new_input.is_left_mouse_key_held = true;
				}

				write_to_input_buffer(&input_buffer, &new_input);

				main_game_loop(&sdl_game, static_data, &input_buffer, delta_time);
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
		}

		end_temporary_memory(sdl_game.game_temporary_memory);
	}
	else
	{
		invalid_code_path;
	}

	check_arena(sdl_game.arena);
	check_arena(sdl_game.transient_arena);

	SDL_DestroyTexture(sdl_game.tileset_texture);

	SDL_DestroyRenderer(sdl_game.renderer);
	SDL_DestroyWindow(sdl_game.window);

	IMG_Quit();
	SDL_Quit();

	return 0;
}