#include "sdl_platform.h"
#include "main.h"
#include "game_data.h"
#include "rendering.h"
#include "input.h"

sdl_data GLOBAL_SDL_DATA;

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
	result.x = (int)round(rect.min_corner.x);
	result.y = (int)round(rect.min_corner.y);
	result.w = (int)round((rect.max_corner.x - rect.min_corner.x));
	result.h = (int)round((rect.max_corner.y - rect.min_corner.y));
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

read_file_result read_file(std::string path)
{
	read_file_result result = {};

	SDL_RWops* file = SDL_RWFromFile(path.c_str(), "r");
	if (file)
	{
		int64_t file_size = SDL_RWsize(file);
		if (file_size != -1 // błąd
			&& file_size < 1024 * 1024 * 5) // zabezpieczenie
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

sdl_data init_sdl()
{
	sdl_data sdl_game = {};
	b32 success = true;

	int init = SDL_Init(SDL_INIT_EVERYTHING);
	if (init == 0) // wg dokumentacji oznacza to sukces
	{
		if (false == SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			print_sdl_error();
		}

		if (false == SDL_ShowCursor(SDL_DISABLE))
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
					load_image(sdl_game.renderer, &sdl_game.font_texture, "gfx/font1.png", &success);
					load_image(sdl_game.renderer, &sdl_game.charset_texture, "gfx/charset.png", &success);
					load_image(sdl_game.renderer, &sdl_game.explosion_texture, "gfx/explosions.png", &success);
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
	sdl_data sdl = init_sdl();
	if (sdl.initialized)
	{
		GLOBAL_SDL_DATA = sdl;

		bool run = true;

		u32 memory_for_permanent_arena_size = megabytes_to_bytes(50);
		memory_arena permanent_arena = {};
		void* memory_for_permanent_arena = SDL_malloc(memory_for_permanent_arena_size);
		initialize_memory_arena(&permanent_arena, memory_for_permanent_arena_size, (byte*)memory_for_permanent_arena);

		memory_arena transient_arena = {};
		u32 memory_for_transient_arena_size = megabytes_to_bytes(50);
		void* memory_for_transient_arena = SDL_malloc(memory_for_transient_arena_size);
		initialize_memory_arena(&transient_arena, memory_for_transient_arena_size, (byte*)memory_for_transient_arena);

		game_state game = {};

		game.arena = &permanent_arena;
		game.transient_arena = &transient_arena;		

		game.render.max_push_buffer_size = megabytes_to_bytes(20);
		game.render.push_buffer_base = (u8*)push_size(&permanent_arena, game.render.max_push_buffer_size);

		game.level_state = push_struct(&permanent_arena, level_state);

		game.level_name_buffer = (char*)push_size(&permanent_arena, MAX_LEVEL_NAME_LENGTH);

		static_game_data* static_data = push_struct(&permanent_arena, static_game_data);

		load_static_game_data(static_data, &permanent_arena, &transient_arena);
		game.input_buffer = initialize_input_buffer(&permanent_arena);

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

				new_input.mouse_x = -1;
				new_input.mouse_y = -1;
				Uint32 mouse_buttons = SDL_GetMouseState(&new_input.mouse_x, &new_input.mouse_y);
				if (mouse_buttons & SDL_BUTTON_LMASK)
				{
					new_input.is_left_mouse_key_held = true;
				}

				write_to_input_buffer(&game.input_buffer, &new_input);

				main_game_loop(&game, static_data, delta_time);
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

			sdl.debug_elapsed_work_ms = elapsed_work_ms;
			sdl.debug_frame_counter = frame_counter + 1;
		}

		end_temporary_memory(game.game_level_memory, false);

		check_arena(game.arena);
		check_arena(game.transient_arena);
	}
	else
	{
		invalid_code_path;
	}
	
	SDL_DestroyTexture(sdl.charset_texture);
	SDL_DestroyTexture(sdl.tileset_texture);
	SDL_DestroyTexture(sdl.font_texture);
	SDL_DestroyTexture(sdl.explosion_texture);

	SDL_DestroyRenderer(sdl.renderer);
	SDL_DestroyWindow(sdl.window);

	IMG_Quit();
	SDL_Quit();

	return 0;
}

void render_rect(sdl_data* sdl, rect rectangle)
{
	rectangle.min_corner.x = round(rectangle.min_corner.x);
	rectangle.min_corner.y = round(rectangle.min_corner.y);
	rectangle.max_corner.x = round(rectangle.max_corner.x);
	rectangle.max_corner.y = round(rectangle.max_corner.y);

	SDL_RenderDrawLine(sdl->renderer, // dół
		rectangle.min_corner.x, rectangle.min_corner.y, rectangle.max_corner.x, rectangle.min_corner.y);
	SDL_RenderDrawLine(sdl->renderer, // lewa
		rectangle.min_corner.x, rectangle.min_corner.y, rectangle.min_corner.x, rectangle.max_corner.y);
	SDL_RenderDrawLine(sdl->renderer, // prawa
		rectangle.max_corner.x, rectangle.min_corner.y, rectangle.max_corner.x, rectangle.max_corner.y);
	SDL_RenderDrawLine(sdl->renderer, // góra
		rectangle.min_corner.x, rectangle.max_corner.y, rectangle.max_corner.x, rectangle.max_corner.y);
}

internal SDL_Texture* get_texture(sdl_data sdl, textures type)
{
	SDL_Texture* result = NULL;
	switch (type)
	{
		case textures::NONE: { result = NULL; }; break;
		case textures::TILESET: { result = sdl.tileset_texture;  }; break;
		case textures::FONT: { result = sdl.font_texture; }; break;
		case textures::CHARSET: { result = sdl.charset_texture; }; break;
		case textures::EXPLOSION: { result = sdl.explosion_texture; }; break;
		invalid_default_case;
	}
	return result;
}

void render_group_to_output(render_group* render_group)
{
	SDL_SetRenderDrawColor(GLOBAL_SDL_DATA.renderer, 0, 0, 0, 0);
	SDL_RenderClear(GLOBAL_SDL_DATA.renderer);
	SDL_SetRenderDrawColor(GLOBAL_SDL_DATA.renderer, 255, 255, 255, 0);

	assert(GLOBAL_SDL_DATA.initialized);
	for (u32 base_address = 0;
		base_address < render_group->push_buffer_size;
		)
	{
		render_group_entry_header* header = (render_group_entry_header*)(render_group->push_buffer_base + base_address);

		void* data = (u8*)header + sizeof(render_group_entry_header);
		base_address += sizeof(render_group_entry_header);

		switch (header->type)
		{
			case render_group_entry_type::BITMAP:
			{
				render_group_entry_bitmap* entry = (render_group_entry_bitmap*)data;

				SDL_Rect src = get_sdl_rect(entry->source_rect);
				SDL_Rect dst = get_sdl_rect(entry->destination_rect);
				SDL_RenderCopy(GLOBAL_SDL_DATA.renderer, get_texture(GLOBAL_SDL_DATA, entry->texture),
					&src, &dst);

				base_address += sizeof(render_group_entry_bitmap);
			}
			break;
			case render_group_entry_type::BITMAP_WITH_EFFECTS:
			{
				render_group_entry_bitmap_with_effects* entry = (render_group_entry_bitmap_with_effects*)data;

				SDL_Texture* texture = get_texture(GLOBAL_SDL_DATA, entry->texture);

				if (entry->render_in_additive_mode)
				{
					SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD);			
				}

				if (false == is_zero(entry->tint_color))
				{
					v4 sdl_tint = entry->tint_color * 255;
					SDL_SetTextureColorMod(texture, sdl_tint.r, sdl_tint.g, sdl_tint.b);
				}

				SDL_Rect src = get_sdl_rect(entry->source_rect);
				SDL_Rect dst = get_sdl_rect(entry->destination_rect);
				SDL_RendererFlip flip = (entry->flip_horizontally ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
				SDL_RenderCopyEx(GLOBAL_SDL_DATA.renderer, texture, &src, &dst, 0, NULL, flip);
				if (entry->render_in_additive_mode)
				{
					// drugi raz - zastosowanie pojedynczo nie daje zauważalnych efektów
					SDL_RenderCopyEx(GLOBAL_SDL_DATA.renderer, texture, &src, &dst, 0, NULL, flip);
				}

				if (false == is_zero(entry->tint_color))
				{
					SDL_SetTextureColorMod(texture, 255, 255, 255);
				}

				if (entry->render_in_additive_mode)
				{
					SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
				}

				base_address += sizeof(render_group_entry_bitmap_with_effects);
			}
			break;
			case render_group_entry_type::DEBUG_RECTANGLE:
			{
				render_group_entry_debug_rectangle* entry = (render_group_entry_debug_rectangle*)data;
			
				if (false == is_zero(entry->color))
				{
					v4 sdl_tint = entry->color * 255;
					SDL_SetRenderDrawColor(GLOBAL_SDL_DATA.renderer, sdl_tint.r, sdl_tint.g, sdl_tint.b, sdl_tint.a);
				}

				if (entry->render_outline_only)
				{
					render_rect(&GLOBAL_SDL_DATA, entry->destination_rect);
				}
				else
				{
					SDL_Rect dst = get_sdl_rect(entry->destination_rect);
					SDL_RenderFillRect(GLOBAL_SDL_DATA.renderer, &dst);
				}

				if (false == is_zero(entry->color))
				{				
					SDL_SetRenderDrawColor(GLOBAL_SDL_DATA.renderer, 255, 255, 255, 0);
				}

				base_address += sizeof(render_group_entry_debug_rectangle);
			}
			break;
			case render_group_entry_type::CLEAR:
			{
				render_group_entry_clear* entry = (render_group_entry_clear*)data;

				SDL_RenderClear(GLOBAL_SDL_DATA.renderer);

				base_address += sizeof(render_group_entry_clear);
			}
			break;
			case render_group_entry_type::FADE:
			{
				render_group_entry_fade* entry = (render_group_entry_fade*)data;

				v4 sdl_color = entry->color * 255;
				sdl_color.a = entry->percentage * 255;

				SDL_Rect fullscreen = { 0,0, SCREEN_WIDTH, SCREEN_HEIGHT };
				SDL_SetRenderDrawBlendMode(GLOBAL_SDL_DATA.renderer, SDL_BlendMode::SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(GLOBAL_SDL_DATA.renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a);
				SDL_RenderFillRect(GLOBAL_SDL_DATA.renderer, &fullscreen);
				SDL_SetRenderDrawColor(GLOBAL_SDL_DATA.renderer, 255, 255, 255, 0);
				SDL_SetRenderDrawBlendMode(GLOBAL_SDL_DATA.renderer, SDL_BlendMode::SDL_BLENDMODE_NONE);

				base_address += sizeof(render_group_entry_fade);
			}
			break;

			invalid_default_case;
		}
	}

	SDL_RenderPresent(GLOBAL_SDL_DATA.renderer);

	// zapisujemy od nowa - nie trzeba zerować
	render_group->push_buffer_size = 0;
}