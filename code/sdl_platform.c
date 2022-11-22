#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "main.h"
#include "game_data.h"
#include "rendering.h"
#include "input.h"
#include "progress.h"
#include "level_initialization.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#define TARGET_HZ 30.0f

typedef struct sdl_data
{
    b32 initialized;
    b32 fullscreen;
    i32 screen_width;
    i32 screen_height;

    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Texture* tileset_texture;
    SDL_Texture* ui_font_texture;
    SDL_Texture* title_font_texture;
    SDL_Texture* charset_texture;
    SDL_Texture* explosion_texture;
    SDL_Texture* background_desert_texture;
    SDL_Texture* background_ice_desert_texture;
    SDL_Texture* background_clouds_texture;
    SDL_Texture* background_red_planet_sky_texture;
    SDL_Texture* background_red_planet_desert_texture;
    SDL_Texture* background_planet_orbit_texture;
    SDL_Texture* background_title_screen_texture;

    string_builder path_buffer;

    string_ref preferences_file_path;
    Mix_Music* music;
} sdl_data;

sdl_data GLOBAL_SDL_DATA;

void render_list_to_output(render_list* render_list);

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
    SDL_Rect result = {0};
    result.x = (int)round(rect.min_corner.x);
    result.y = (int)round(rect.min_corner.y);
    result.w = (int)round(rect.max_corner.x - rect.min_corner.x);
    result.h = (int)round(rect.max_corner.y - rect.min_corner.y);
    return result;
}

void print_sdl_error(void)
{
#ifdef TRIFLE_DEBUG
    const char* error = SDL_GetError();
    printf("SDL error: %s\n", error);
    invalid_code_path;
#endif
}

void print_sdl_image_error(void)
{
#ifdef TRIFLE_DEBUG
    const char* error = IMG_GetError();
    printf("SDL_image error: %s\n", error);
    invalid_code_path;
#endif
}

void print_sdl_mixer_error(void)
{
#ifdef TRIFLE_DEBUG
    const char* error = Mix_GetError();
    printf("SDL_mixer error: %s\n", error);
    invalid_code_path;
#endif
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

read_file_result read_file(const char* path)
{
    read_file_result result = {0};

    SDL_RWops* file = SDL_RWFromFile(path, "r");
    if (file)
    {
        int64_t file_size = SDL_RWsize(file);
        if (file_size != -1 // error
            && file_size < 1024 * 1024 * 5) // safeguard
        {
            result.size = file_size;
            result.contents = calloc(file_size + 1, sizeof(byte));

#ifdef TRIFLE_DEBUG
            printf("read file result: address: %d, size: %d\n", (int)result.contents, (int)result.size);
#endif

            if (result.contents != 0)
            {
                for (int byte_index = 0;
                    byte_index < file_size;
                    ++byte_index)
                {
                    SDL_RWread(file, (void*)((char*)result.contents + byte_index), sizeof(char), 1);
                }
                *((char*)result.contents + file_size) = 0;
            }    
        }
        else
        {
            print_sdl_error();
        }

        SDL_RWclose(file);
    }

    return result;
}

void save_file(const char* path, write_to_file contents)
{
    SDL_RWops* file = SDL_RWFromFile(path, "w+b");
    if (file != NULL)
    {				
        int bytes_written = SDL_RWwrite(file, contents.buffer, sizeof(char), contents.length);
        if (bytes_written < contents.length)
        {
            print_sdl_error();
        }

        SDL_RWclose(file);
    }
}

void store_preferences_file_path(sdl_data* sdl, memory_arena* permanent_arena)
{
    char* preferences_folder_path = SDL_GetPrefPath("Trifle Psychotic", "Data");
    if (preferences_folder_path != NULL)
    {
        empty_string_builder(&sdl->path_buffer);
        push_c_string_to_builder(&sdl->path_buffer, preferences_folder_path);
        push_c_string_to_builder(&sdl->path_buffer, "completed_levels.txt");
        safe_push_null_terminator_to_builder(&sdl->path_buffer);
        string_ref path = get_string_from_string_builder(&sdl->path_buffer);
        sdl->preferences_file_path = copy_string(permanent_arena, path);

        SDL_free(preferences_folder_path);
    }
}

read_file_result load_prefs(void)
{
    read_file_result result = {0};
    if (GLOBAL_SDL_DATA.preferences_file_path.ptr != NULL)
    {		
        result = read_file(GLOBAL_SDL_DATA.preferences_file_path.ptr);
    }
    return result;
}

void save_prefs(write_to_file contents)
{
    if (GLOBAL_SDL_DATA.preferences_file_path.ptr != NULL)
    {
        save_file(GLOBAL_SDL_DATA.preferences_file_path.ptr, contents);
    }	
}

void start_playing_music(string_ref audio_file_name)
{
    if (audio_file_name.string_size > 0)
    {
        sdl_data* sdl = &GLOBAL_SDL_DATA;
        if (sdl->music != NULL)
        {
            Mix_HaltMusic();
            Mix_FreeMusic(sdl->music);
            sdl->music = NULL;
        }

        empty_string_builder(&sdl->path_buffer);
        push_c_string_to_builder(&sdl->path_buffer, "audio/");
        push_string_to_builder(&sdl->path_buffer, audio_file_name);
        if (false == (ends_with(audio_file_name, ".ogg")
            || ends_with(audio_file_name, ".mp3")
            || ends_with(audio_file_name, ".wav")))
        {
            push_c_string_to_builder(&sdl->path_buffer, ".ogg");
        }
        safe_push_null_terminator_to_builder(&sdl->path_buffer);

        sdl->music = Mix_LoadMUS(sdl->path_buffer.ptr);
        if (sdl->music == NULL)
        {
            print_sdl_mixer_error();
        }
        else
        {
            Mix_FadeInMusic(GLOBAL_SDL_DATA.music, -1, 4000); // -1 means loop infinitely
        }
    }
}

void stop_playing_music(int fade_out_ms)
{
    if (Mix_PlayingMusic() != 0)
    {
        Mix_FadeOutMusic(fade_out_ms);
    }
}

sdl_data init_sdl(void)
{
    sdl_data sdl_game = {0};
    b32 success = true;

    // SDL_INIT_EVERYTHING causes error in Emscripten
    int init = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); 
    if (init == 0) // success
    {
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
            if (sdl_game.renderer)
            {
                SDL_RenderSetScale(sdl_game.renderer, SCALING_FACTOR, SCALING_FACTOR);

                SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
                SDL_RenderSetLogicalSize(sdl_game.renderer,
                    SCREEN_WIDTH / SCALING_FACTOR,
                    SCREEN_HEIGHT / SCALING_FACTOR);

#ifndef __EMSCRIPTEN__
                SDL_SetWindowFullscreen(sdl_game.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                sdl_game.fullscreen = true;

                SDL_GetRendererOutputSize(sdl_game.renderer,
                    &sdl_game.screen_width, &sdl_game.screen_height);
#endif

                SDL_SetRenderDrawColor(sdl_game.renderer, 0xFF, 0xFF, 0xFF, 0xFF);

                int img_flags = IMG_INIT_PNG;
                if (IMG_Init(img_flags) & img_flags)
                {
                    load_image(sdl_game.renderer, &sdl_game.tileset_texture, "gfx/tileset.png", &success);
                    load_image(sdl_game.renderer, &sdl_game.ui_font_texture, "gfx/ui_font.png", &success);
                    load_image(sdl_game.renderer, &sdl_game.title_font_texture, "gfx/title_font.png", &success);
                    load_image(sdl_game.renderer, &sdl_game.charset_texture, "gfx/charset.png", &success);
                    load_image(sdl_game.renderer, &sdl_game.explosion_texture, "gfx/explosions.png", &success);
                    load_image(sdl_game.renderer, &sdl_game.background_desert_texture, "gfx/background_desert.png", &success);
                    load_image(sdl_game.renderer, &sdl_game.background_ice_desert_texture, "gfx/background_ice_desert.png", &success);
                    load_image(sdl_game.renderer, &sdl_game.background_clouds_texture, "gfx/background_clouds.png", &success);
                    load_image(sdl_game.renderer, &sdl_game.background_red_planet_sky_texture, "gfx/background_red_planet_sky.png", &success);
                    load_image(sdl_game.renderer, &sdl_game.background_red_planet_desert_texture, "gfx/background_red_planet_desert.png", &success);
                    load_image(sdl_game.renderer, &sdl_game.background_planet_orbit_texture, "gfx/background_planet_orbit.png", &success);
                    load_image(sdl_game.renderer, &sdl_game.background_title_screen_texture, "gfx/background_title_screen.png", &success);
                }
                else
                {
                    print_sdl_image_error();
                    success = false;
                }

                if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
                {
                    print_sdl_mixer_error();
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
        return (sdl_data){0};
    }
}

game_input get_input_from_sdl_events(void)
{
    game_input new_input = {0};

    SDL_Event e = {0};
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            new_input.quit = true;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_UP]    || state[SDL_SCANCODE_W]) new_input.up.number_of_presses++;
    if (state[SDL_SCANCODE_DOWN]  || state[SDL_SCANCODE_S]) new_input.down.number_of_presses++;
    if (state[SDL_SCANCODE_LEFT]  || state[SDL_SCANCODE_A]) new_input.left.number_of_presses++;
    if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) new_input.right.number_of_presses++;
    if (state[SDL_SCANCODE_ESCAPE])                         new_input.escape.number_of_presses++;

    new_input.mouse_x = -1;
    new_input.mouse_y = -1;
    Uint32 mouse_buttons = SDL_GetMouseState(&new_input.mouse_x, &new_input.mouse_y);
    if (mouse_buttons & SDL_BUTTON_LMASK)
    {
        new_input.is_fire_button_held = true;
    }

    if (state[SDL_SCANCODE_SPACE])
    {
        new_input.is_fire_button_held = true;
    }

    return new_input;
}

#ifdef __EMSCRIPTEN__

EM_JS(bool, is_browser_fullscreen, (),
{
    let fullscreen = document.fullScreen || document.mozFullScreen || document.webkitIsFullScreen;
    return fullscreen;
});

EM_JS(bool, is_browser_firefox, (),
{
    let is_firefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;
    return is_firefox;
});

void emscripten_main_game_loop(void* passed_data)
{
    game_input new_input = get_input_from_sdl_events();
    write_to_input_buffer(&(((game_state*)passed_data)->input_buffer), &new_input);

    bool is_sdl_set_to_fullscreen = SDL_GetWindowFlags(GLOBAL_SDL_DATA.window) & SDL_WINDOW_FULLSCREEN_DESKTOP;
    if (is_browser_fullscreen())
    {
        if (false == is_sdl_set_to_fullscreen)
        {
            SDL_SetWindowFullscreen(GLOBAL_SDL_DATA.window, SDL_WINDOW_FULLSCREEN);
        }
        GLOBAL_SDL_DATA.fullscreen = true;	
    }
    else
    {
        if (is_sdl_set_to_fullscreen)
        {
            SDL_SetWindowFullscreen(GLOBAL_SDL_DATA.window, 0);
        }
        GLOBAL_SDL_DATA.fullscreen = false;
    }

    r64 delta_time = 1 / TARGET_HZ;
    main_game_loop((game_state*)passed_data, delta_time);
}

#endif

game_state* initialize_game_state(void)
{
    u32 memory_for_permanent_arena_size = megabytes_to_bytes(5);
    void* memory_for_permanent_arena = SDL_malloc(memory_for_permanent_arena_size);
    memory_arena* permanent_arena = initialize_memory_arena(memory_for_permanent_arena_size, (byte*)memory_for_permanent_arena);

    u32 memory_for_transient_arena_size = megabytes_to_bytes(2);
    void* memory_for_transient_arena = SDL_malloc(memory_for_transient_arena_size);
    memory_arena* transient_arena = initialize_memory_arena(memory_for_transient_arena_size, (byte*)memory_for_transient_arena);

    game_state* game = push_struct(permanent_arena, game_state);

    game->arena = permanent_arena;
    game->transient_arena = transient_arena;

    game->platform.read_file = &read_file;
    game->platform.save_file = &save_file;
    game->platform.load_prefs = &load_prefs;
    game->platform.save_prefs = &save_prefs;
    game->platform.start_playing_music = &start_playing_music;
    game->platform.stop_playing_music = &stop_playing_music;
    game->platform.render_list_to_output = &render_list_to_output;
    
    game->static_data = push_struct(permanent_arena, static_game_data);;
    load_static_game_data(&game->platform, game->static_data, permanent_arena, transient_arena);

    game->render.max_push_buffer_size = megabytes_to_bytes(1);
    game->render.push_buffer_base = (u8*)push_size(permanent_arena, game->render.max_push_buffer_size);

    game->level_state = push_struct(permanent_arena, level_state);
    game->level_name_buffer = (char*)push_size(permanent_arena, MAX_LEVEL_NAME_LENGTH);

    initialize_memory_for_checkpoint(game, permanent_arena);

    game->input_buffer = initialize_input_buffer(permanent_arena);

    game->current_scene = SCENE_MAIN_MENU;

    return game;
}

string_ref parse_starting_level_from_command_line(memory_arena* arena, char* args[], int args_count)
{
    string_ref result = {0};
    if (args != NULL)
    {
        // we omit the first option - it's a path to the exe
        for (i32 option_index = 1; option_index < args_count; option_index++)
        {
            char* option = args[option_index];
            if (*option != 0)
            {
                result = copy_c_string(arena, option);
                break;
            }
        }
    }
    return result;
}

int main(int args_count, char* args[])
{
    GLOBAL_SDL_DATA = init_sdl();
    sdl_data* sdl = &GLOBAL_SDL_DATA;
    if (sdl->initialized)
    {
        bool run = true;

        game_state* game = initialize_game_state();

        int max_path_length = 4096; // max path length on Linux, Windows, and MacOS
        sdl->path_buffer = get_string_builder(game->transient_arena, max_path_length);
        store_preferences_file_path(sdl, game->arena);
        
#ifndef __EMSCRIPTEN__
        game->cmd_level_to_load = parse_starting_level_from_command_line(game->arena, args, args_count);
        if (game->cmd_level_to_load.string_size > 0)
        {
            game->current_scene = SCENE_GAME;
            game->exit_level_closes_game = true;
            game->skip_introductions = true;
            game->skip_deaths_prompts = true;
        }
#endif

#if TRIFLE_DEBUG
        // we erase the progress
        save_completed_levels(&game->platform, game->static_data, game->transient_arena);
#endif

#ifdef __EMSCRIPTEN__
        game->show_exit_game_option = false;

        r32 target_hz = TARGET_HZ;
        if (is_browser_firefox())
        {
            // hack improving speed in Firefox - without this game looks very slowed down
            target_hz *= 1.5f;
        }

        emscripten_set_main_loop_arg(emscripten_main_game_loop, (void*)game, target_hz, true);
#else		
        game->show_exit_game_option = true;

        r32 target_elapsed_ms = 1000 / TARGET_HZ;
        r32 elapsed_work_ms = 0;
        r64 delta_time = 1 / TARGET_HZ;

        while (run)
        {
            u32 start_work_counter = SDL_GetPerformanceCounter();

            {
                game_input new_input = get_input_from_sdl_events();

                if (new_input.quit)
                {
                    run = false;
                }

                write_to_input_buffer(&game->input_buffer, &new_input);

                main_game_loop(game, delta_time);

                if (game->exit_game)
                {
                    run = false;
                }
            }

            u32 end_work_counter = SDL_GetPerformanceCounter();

            elapsed_work_ms = get_elapsed_miliseconds(start_work_counter, end_work_counter);
            if (elapsed_work_ms < target_elapsed_ms)
            {
                r32 how_long_to_sleep_ms = target_elapsed_ms - elapsed_work_ms;
                if (how_long_to_sleep_ms > 1)
                {
                    SDL_Delay((Uint32)how_long_to_sleep_ms);
                }

                r32 total_elapsed_ms = get_elapsed_miliseconds(start_work_counter, SDL_GetPerformanceCounter());
                while (target_elapsed_ms > total_elapsed_ms)
                {
                    total_elapsed_ms = get_elapsed_miliseconds(start_work_counter, SDL_GetPerformanceCounter());
                }
            }
        }
#endif

        if (game->game_level_memory.arena)
        {
            end_temporary_memory(game->game_level_memory, false);
        }

        check_arena(game->arena);
        check_arena(game->transient_arena);
    }
    else
    {
        invalid_code_path;
    }

    SDL_DestroyTexture(sdl->background_clouds_texture);
    SDL_DestroyTexture(sdl->background_desert_texture);
    SDL_DestroyTexture(sdl->background_ice_desert_texture);
    SDL_DestroyTexture(sdl->background_planet_orbit_texture);
    SDL_DestroyTexture(sdl->background_red_planet_desert_texture);
    SDL_DestroyTexture(sdl->background_red_planet_sky_texture);
    SDL_DestroyTexture(sdl->background_title_screen_texture);
    SDL_DestroyTexture(sdl->tileset_texture);
    SDL_DestroyTexture(sdl->charset_texture);
    SDL_DestroyTexture(sdl->ui_font_texture);
    SDL_DestroyTexture(sdl->title_font_texture);
    SDL_DestroyTexture(sdl->explosion_texture);
    
    Mix_HaltMusic();
    Mix_FreeMusic(sdl->music);

    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);

    Mix_Quit();
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

    SDL_RenderDrawLine(sdl->renderer, // down
        rectangle.min_corner.x, rectangle.min_corner.y, rectangle.max_corner.x, rectangle.min_corner.y);
    SDL_RenderDrawLine(sdl->renderer, // left
        rectangle.min_corner.x, rectangle.min_corner.y, rectangle.min_corner.x, rectangle.max_corner.y);
    SDL_RenderDrawLine(sdl->renderer, // right
        rectangle.max_corner.x, rectangle.min_corner.y, rectangle.max_corner.x, rectangle.max_corner.y);
    SDL_RenderDrawLine(sdl->renderer, // up
        rectangle.min_corner.x, rectangle.max_corner.y, rectangle.max_corner.x, rectangle.max_corner.y);
}

internal SDL_Texture* get_texture(sdl_data sdl, textures type)
{
    SDL_Texture* result = NULL;
    switch (type)
    {
        case TEXTURE_NONE: { result = NULL; }; break;
        case TEXTURE_TILESET: { result = sdl.tileset_texture; }; break;
        case TEXTURE_FONT: { result = sdl.ui_font_texture; }; break;
        case TEXTURE_TITLE_FONT: { result = sdl.title_font_texture; }; break;
        case TEXTURE_CHARSET: { result = sdl.charset_texture; }; break;
        case TEXTURE_EXPLOSION: { result = sdl.explosion_texture; }; break;
        case TEXTURE_BACKGROUND_DESERT: { result = sdl.background_desert_texture; }; break;
        case TEXTURE_BACKGROUND_ICE_DESERT: { result = sdl.background_ice_desert_texture; }; break;
        case TEXTURE_BACKGROUND_CLOUDS: { result = sdl.background_clouds_texture; }; break;
        case TEXTURE_BACKGROUND_RED_PLANET_SKY: { result = sdl.background_red_planet_sky_texture; }; break;
        case TEXTURE_BACKGROUND_RED_PLANET_DESERT: { result = sdl.background_red_planet_desert_texture; }; break;
        case TEXTURE_BACKGROUND_PLANET_ORBIT: { result = sdl.background_planet_orbit_texture; }; break;
        case TEXTURE_BACKGROUND_TITLE_SCREEN: { result = sdl.background_title_screen_texture; }; break;
        invalid_default_case;
    }
    return result;
}

void render_list_to_output(render_list* render)
{
    SDL_SetRenderDrawColor(GLOBAL_SDL_DATA.renderer, 0, 0, 0, 0);
    SDL_RenderClear(GLOBAL_SDL_DATA.renderer);
    SDL_SetRenderDrawColor(GLOBAL_SDL_DATA.renderer, 255, 255, 255, 0);

    assert(GLOBAL_SDL_DATA.initialized);
    for (u32 base_address = 0;
        base_address < render->push_buffer_size;
        )
    {
        render_list_entry_header* header = (render_list_entry_header*)(render->push_buffer_base + base_address);

        void* data = (u8*)header + sizeof(render_list_entry_header);
        base_address += sizeof(render_list_entry_header);

        switch (header->type)
        {
            case RENDER_LIST_ENTRY_BITMAP:
            {
                render_list_entry_bitmap* entry = (render_list_entry_bitmap*)data;

                SDL_Texture* texture = get_texture(GLOBAL_SDL_DATA, entry->texture);
                SDL_Rect src = get_sdl_rect(entry->source_rect);
                SDL_Rect dst = get_sdl_rect(entry->destination_rect);
                SDL_RenderCopy(GLOBAL_SDL_DATA.renderer, texture, &src, &dst);

                base_address += sizeof(render_list_entry_bitmap);
            }
            break;
            case RENDER_LIST_ENTRY_BITMAP_WITH_EFFECTS:
            {
                render_list_entry_bitmap_with_effects* entry = (render_list_entry_bitmap_with_effects*)data;

                SDL_Texture* texture = get_texture(GLOBAL_SDL_DATA, entry->texture);
                SDL_Rect src = get_sdl_rect(entry->source_rect);
                SDL_Rect dst = get_sdl_rect(entry->destination_rect);
                SDL_RendererFlip flip = (entry->flip_horizontally ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
                v4 sdl_tint = multiply_v4(entry->tint_color, 255.0f);

                // this fixes weird behavior of SDL in the fullscreen mode - flipped bitmaps changed position depending on resolution
                if (flip == SDL_FLIP_HORIZONTAL && GLOBAL_SDL_DATA.fullscreen)
                {
                    // I have no idea why this makes a difference for SDL
                    if (GLOBAL_SDL_DATA.screen_width == 1920 && GLOBAL_SDL_DATA.screen_height == 1080)
                    {
                        dst.x -= 185;
                    }
                    else if (GLOBAL_SDL_DATA.screen_width == 1920 && GLOBAL_SDL_DATA.screen_height == 1200)
                    {
                        dst.x -= 126;
                    }
                    else if (GLOBAL_SDL_DATA.screen_width == 1536 && GLOBAL_SDL_DATA.screen_height == 960)
                    {
                        dst.x -= 95;
                    }
                    else if (GLOBAL_SDL_DATA.screen_width == 1536)
                    {
                        dst.x -= 136;
                    }
                }

                if (entry->render_in_additive_mode)
                {
                    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD);
                    SDL_SetTextureColorMod(texture, sdl_tint.r, sdl_tint.g, sdl_tint.b);

                    SDL_RenderCopyEx(GLOBAL_SDL_DATA.renderer, texture, &src, &dst, 0, NULL, flip);

                    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
                    SDL_SetTextureColorMod(texture, 255, 255, 255);
                }
                else
                {
                    if (false == is_zero_v4(entry->tint_color))
                    {
                        SDL_SetTextureColorMod(texture, sdl_tint.r, sdl_tint.g, sdl_tint.b);

                        SDL_RenderCopyEx(GLOBAL_SDL_DATA.renderer, texture, &src, &dst, 0, NULL, flip);

                        SDL_SetTextureColorMod(texture, 255, 255, 255);
                    }
                    else
                    {
                        SDL_RenderCopyEx(GLOBAL_SDL_DATA.renderer, texture, &src, &dst, 0, NULL, flip);
                    }
                }

                base_address += sizeof(render_list_entry_bitmap_with_effects);
            }
            break;
            case RENDER_LIST_ENTRY_RECTANGLE:
            {
                render_list_entry_rectangle* entry = (render_list_entry_rectangle*)data;

                if (false == is_zero_v4(entry->color))
                {
                    v4 sdl_tint = multiply_v4(entry->color, 255.0f);
                    if (entry->color.a != 1.0f)
                    {
                        SDL_SetRenderDrawBlendMode(GLOBAL_SDL_DATA.renderer, SDL_BLENDMODE_BLEND);
                    }
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

                if (false == is_zero_v4(entry->color))
                {
                    SDL_SetRenderDrawColor(GLOBAL_SDL_DATA.renderer, 255, 255, 255, 0);
                    SDL_SetRenderDrawBlendMode(GLOBAL_SDL_DATA.renderer, SDL_BLENDMODE_NONE);
                }

                base_address += sizeof(render_list_entry_rectangle);
            }
            break;
            case RENDER_LIST_ENTRY_CLEAR:
            {
                render_list_entry_clear* entry = (render_list_entry_clear*)data;

                if (false == is_zero_v4(entry->color))
                {
                    v4 sdl_tint = multiply_v4(entry->color, 255.0f);
                    SDL_SetRenderDrawColor(GLOBAL_SDL_DATA.renderer, sdl_tint.r, sdl_tint.g, sdl_tint.b, sdl_tint.a);
                }

                SDL_RenderClear(GLOBAL_SDL_DATA.renderer);

                if (false == is_zero_v4(entry->color))
                {
                    SDL_SetRenderDrawColor(GLOBAL_SDL_DATA.renderer, 255, 255, 255, 0);
                }

                base_address += sizeof(render_list_entry_clear);
            }
            break;
            case RENDER_LIST_ENTRY_FADE:
            {
                render_list_entry_fade* entry = (render_list_entry_fade*)data;

                v4 sdl_color = multiply_v4(entry->color, 255.0f);
                sdl_color.a = entry->percentage * 255;

                SDL_Rect fullscreen = { 0,0, SCREEN_WIDTH, SCREEN_HEIGHT };
                SDL_SetRenderDrawBlendMode(GLOBAL_SDL_DATA.renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(GLOBAL_SDL_DATA.renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a);
                SDL_RenderFillRect(GLOBAL_SDL_DATA.renderer, &fullscreen);
                SDL_SetRenderDrawColor(GLOBAL_SDL_DATA.renderer, 255, 255, 255, 0);
                SDL_SetRenderDrawBlendMode(GLOBAL_SDL_DATA.renderer, SDL_BLENDMODE_NONE);

                base_address += sizeof(render_list_entry_fade);
            }
            break;

            invalid_default_case;
        }
    }

    SDL_RenderPresent(GLOBAL_SDL_DATA.renderer);

    // we will just overwrite it - there is no need to zero memory
    render->push_buffer_size = 0;
}