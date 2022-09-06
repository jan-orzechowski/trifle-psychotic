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

					sdl_game.bullets_texture = load_image(sdl_game.renderer, "gfx/bullets.png");
					if (sdl_game.bullets_texture == NULL)
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

sprite_effect_stage* add_sprite_effect_stage(sprite_effect* effect, 
	r32 amplitude, r32 phase_shift, r32 vertical_shift, r32 period, r32 stage_duration)
{
	assert(stage_duration != 0);

	sprite_effect_stage* new_stage = &effect->stages[effect->stages_count++];
	effect->total_duration += stage_duration;

	new_stage->amplitude = amplitude;
	new_stage->phase_shift = phase_shift;
	new_stage->period = period;
	new_stage->stage_duration = stage_duration;
	
	return new_stage;
}

sprite_effect_stage* add_constant_tint_sprite_effect_stage(sprite_effect* effect, 
	r32 tint_perc, r32 stage_duration)
{
	sprite_effect_stage* new_stage = add_sprite_effect_stage(effect, tint_perc, 0.0f, 0.0f, 0.0f, stage_duration);
	return new_stage;
}

r32 get_stage_tint(sprite_effect_stage* stage, r32 total_time)
{
	r32 result = 1.0f;
	if (total_time < stage->stage_duration)
	{
		if (stage->period == 0)
		{
			// stała wartość 
			result = stage->amplitude;
		}
		else
		{
			// czy tutaj powinniśmy podzielić period przez 2PI?
			result = (stage->amplitude *
				SDL_sinf((total_time / stage->period) + stage->phase_shift))
				+ stage->vertical_shift;

			debug_breakpoint;
		}

		if (result < 0)
		{
			if (stage->ignore_negatives)
			{
				result = 0;
			}
			else 
			{
				result = -result;
			}
		}		
	}

	return result;
}

SDL_Color get_tint(sprite_effect* effect, r32 time)
{
	SDL_Color result = effect->color;
	if (time >= 0.0f && time <= effect->total_duration)
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
				break;
			}
			else
			{
				time -= stage->stage_duration;
			}
		}

		if (found)
		{
			result.r = effect->color.r * tint_value;
			result.g = effect->color.g * tint_value;
			result.b = effect->color.b * tint_value;
			result.a = effect->color.a * tint_value;
		}
	}

	return result;
}

void start_visual_effect(game_data* game, entity* entity, u32 sprite_effect_index, b32 override_current)
{
	assert(sprite_effect_index < game->visual_effects_count);
	if (entity->visual_effect == NULL || override_current)
	{
		sprite_effect* effect = &game->visual_effects[sprite_effect_index];
		entity->visual_effect = effect;
		entity->visual_effect_duration = 0;
	}
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

sprite* get_sprite_from_animation(animation* animation, r32* elapsed_time)
{
	sprite* result = NULL;

	while (*elapsed_time > animation->total_duration)
	{
		*elapsed_time -= animation->total_duration;
	}

	r32 time_within_frame = *elapsed_time;
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
			result = &frame->sprite;
			break;
		}
	}

	return result;
}

sprite_part get_sprite_part(SDL_Texture* texture, SDL_Rect texture_rect, v2 offset = get_zero_v2(), 
	direction default_direction = direction::E)
{
	sprite_part result = {};
	result.texture = texture;
	result.texture_rect = texture_rect;
	result.offset = offset;
	result.default_direction = default_direction;
	return result;
}

void fill_animation_frame(animation* animation, u32 frame_index, u32 part_index, sprite_part part, r32 duration)
{
	animation->frames[frame_index].sprite.parts[part_index] = part;	
	animation->frames[frame_index].duration = duration;	
	animation->total_duration += duration;
}

animation* get_player_walk_animation(sdl_game_data* sdl_game, memory_arena* arena)
{
	animation* new_animation = push_struct(arena, animation);
	new_animation->frames_count = 4;
	new_animation->frames = push_array(arena, new_animation->frames_count, animation_frame);

	SDL_Rect legs_rect = {};
	legs_rect.x = 0;
	legs_rect.y = 24;
	legs_rect.w = 24;
	legs_rect.h = 24;

	SDL_Rect head_rect = {};
	head_rect.x = 0;
	head_rect.y = 0;
	head_rect.w = 24;
	head_rect.h = 24;

	v2 legs_offset = get_v2(0.0f, -4.0f);
	v2 head_offset = get_v2(5.0f, -20.0f);
	
	r32 frame_duration = 0.2f;
	SDL_Texture* texture = sdl_game->player_texture;

	new_animation->frames[0].sprite.parts = push_array(arena, 2, sprite_part);
	new_animation->frames[1].sprite.parts = push_array(arena, 2, sprite_part);
	new_animation->frames[2].sprite.parts = push_array(arena, 2, sprite_part);
	new_animation->frames[3].sprite.parts = push_array(arena, 2, sprite_part);

	new_animation->frames[0].sprite.parts_count = 2;
	new_animation->frames[1].sprite.parts_count = 2;
	new_animation->frames[2].sprite.parts_count = 2;
	new_animation->frames[3].sprite.parts_count = 2;

	legs_rect.x = 24;
	fill_animation_frame(new_animation, 0, 0, get_sprite_part(texture, legs_rect, legs_offset), frame_duration);
	fill_animation_frame(new_animation, 0, 1, get_sprite_part(texture, head_rect, head_offset), frame_duration);
	head_offset.x += 1;
	fill_animation_frame(new_animation, 1, 0, get_sprite_part(texture, legs_rect, legs_offset), frame_duration);
	fill_animation_frame(new_animation, 1, 1, get_sprite_part(texture, head_rect, head_offset), frame_duration);
	legs_rect.x = 48;
	fill_animation_frame(new_animation, 2, 0, get_sprite_part(texture, legs_rect, legs_offset), frame_duration);
	fill_animation_frame(new_animation, 2, 1, get_sprite_part(texture, head_rect, head_offset), frame_duration);
	head_offset.x -= 1;
	//legs_rect.x = 0;
	fill_animation_frame(new_animation, 3, 0, get_sprite_part(texture, legs_rect, legs_offset), frame_duration);
	fill_animation_frame(new_animation, 3, 1, get_sprite_part(texture, head_rect, head_offset), frame_duration);
	//head_offset.x -= 1;
	
	return new_animation;
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

u32 get_tile_value(level map, tile_position tile)
{
	u32 result = get_tile_value(map, tile.x, tile.y);
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

sprite get_tile_graphics(sdl_game_data* sdl_game, memory_arena* arena, u32 tile_value)
{
	sprite result = {};

	SDL_Rect texture_rect = get_tile_rect(tile_value);

	result.parts_count = 1;
	result.parts = push_array(arena, result.parts_count, sprite_part);
	result.parts[0].texture = sdl_game->tileset_texture;
	result.parts[0].texture_rect = texture_rect;
	result.parts[0].offset = get_zero_v2();

	return result;
}

sprite get_player_idle_pose(sdl_game_data* sdl_game, memory_arena* arena)
{
	sprite result = {};

	SDL_Rect player_head_rect = {};
	player_head_rect.x = 0;
	player_head_rect.y = 0;
	player_head_rect.w = 24;
	player_head_rect.h = 24;

	SDL_Rect player_legs_rect = {};
	player_legs_rect.x = 0;
	player_legs_rect.y = 24;
	player_legs_rect.w = 24;
	player_legs_rect.h = 24;

	v2 legs_offset = get_v2(0.0f, -4.0f);
	v2 head_offset = get_v2(5.0f, -20.0f);

	result.parts_count = 2;
	result.parts = push_array(arena, result.parts_count, sprite_part);
	result.parts[0].texture = sdl_game->player_texture;
	result.parts[0].texture_rect = player_head_rect;
	result.parts[0].offset = head_offset;
	result.parts[0].default_direction = direction::E;

	result.parts[1].texture = sdl_game->player_texture;
	result.parts[1].texture_rect = player_legs_rect;
	result.parts[1].offset = legs_offset;
	result.parts[1].default_direction = direction::E;

	return result;
}

sprite get_bullet_graphics(sdl_game_data* sdl_game, memory_arena* arena, u32 x, u32 y)
{
	sprite result = {};

	SDL_Rect texture_rect = {};
	texture_rect.w = 10;
	texture_rect.h = 10;
	texture_rect.x = y * 10;
	texture_rect.y = x * 10;

	result.parts_count = 1;
	result.parts = push_array(arena, result.parts_count, sprite_part);
	result.parts[0].texture = sdl_game->bullets_texture;
	result.parts[0].texture_rect = texture_rect;
	result.parts[0].offset = get_zero_v2();

	return result;
}

void add_bullet(game_data* game, entity_type* type, v2 position, v2 velocity)
{
	if (game->bullets_count < game->bullets_max_count)
	{
		bullet* bul = &game->bullets[game->bullets_count];
		bul->type = type;
		bul->position = position;
		bul->velocity = velocity;
		game->bullets_count++;		
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

v2 get_standing_collision_rect_offset(v2 collision_rect_dim)
{
	// zakładamy że wszystkie obiekty mają pozycję na środku pola, czyli 0.5f nad górną krawędzią pola pod nimi
	v2 offset = get_zero_v2();
	offset.y = -((collision_rect_dim.y / 2) - 0.5f);
	return offset;
}

b32 is_standing_on_ground(level map, level collision_ref, entity* entity_to_check)
{
	v2 position = entity_to_check->position;
	v2 collision_rect_dim = entity_to_check->type->collision_rect_dim;

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
	new_entity->health = type->max_health;
	return new_entity;
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

b32 are_entity_flags_set(entity* entity, u32 flag_values)
{
	b32 result = are_flags_set((u32*)&entity->type->flags, flag_values);
	return result;
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
		printf("gracz dostaje %d obrazen, zostalo %d zdrowia\n", damage_amount, game->entities[0].health);
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

entity_collision_data get_entity_collision_data(entity* entity)
{
	entity_collision_data result = {};
	result.position = entity->position;
	result.collision_rect_dim = entity->type->collision_rect_dim;
	result.collision_rect_offset = entity->type->collision_rect_offset;
	return result;
}

entity_collision_data get_bullet_collision_data(bullet* bullet)
{
	entity_collision_data result = {};
	result.position = bullet->position;
	result.collision_rect_dim = bullet->type->collision_rect_dim;
	result.collision_rect_offset = bullet->type->collision_rect_offset;
	return result;
}

entity_collision_data get_tile_collision_data(u32 tile_x, u32 tile_y)
{
	entity_collision_data result = {};
	result.position = get_tile_v2_position(get_tile_position(tile_x, tile_y));
	result.collision_rect_dim = get_v2(1.0f, 1.0f);
	result.collision_rect_offset = get_zero_v2();
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

	//result.was_intersection = (was_intersection || west || east || north || south);

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

rect get_tiles_area_to_check_for_collision(v2 entity_position, v2 collision_rect_offset, v2 collision_rect_dim, v2 target_pos)
{
	tile_position entity_tile = get_tile_position(entity_position + collision_rect_offset);
	tile_position target_tile = get_tile_position(target_pos);

	i32 x_margin = (i32)SDL_ceil(collision_rect_dim.x);
	i32 y_margin = (i32)SDL_ceil(collision_rect_dim.y);

	rect result = {};
	result.min_corner.x = min((i32)entity_tile.x - x_margin, (i32)target_tile.x - x_margin);
	result.min_corner.y = min((i32)entity_tile.y - y_margin, (i32)target_tile.y - y_margin);
	result.max_corner.x = max((i32)entity_tile.x + x_margin, (i32)target_tile.x + x_margin);
	result.max_corner.y = max((i32)entity_tile.y + y_margin, (i32)target_tile.y + y_margin);
	return result;
}

rect get_tiles_area_to_check_for_collision(entity* entity, v2 target_pos)
{
	rect result = get_tiles_area_to_check_for_collision(
		entity->position, entity->type->collision_rect_offset, entity->type->collision_rect_dim, target_pos);
	return result;
}

rect get_tiles_area_to_check_for_collision(bullet* bullet, v2 target_pos)
{
	rect result = get_tiles_area_to_check_for_collision(
		bullet->position, bullet->type->collision_rect_offset, bullet->type->collision_rect_dim, target_pos);
	return result;
}

struct collision_with_effect
{
	collision data;
	entity* collided_entity;
};

collision_with_effect move(sdl_game_data* sdl_game, game_data* game, entity* moving_entity, v2 target_pos)
{	
	v2 movement_delta = target_pos - moving_entity->position;
	collision_with_effect result = {};

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
						u32 tile_value = get_tile_value(game->current_level, tile_x_to_check, tile_y_to_check);
						if (is_tile_colliding(game->collision_reference, tile_value))
						{							
							collision new_collision = check_minkowski_collision(
								get_entity_collision_data(moving_entity),
								get_tile_collision_data(tile_x_to_check, tile_y_to_check),
								movement_delta, closest_collision.possible_movement_perc);
							
							if (new_collision.collided_wall != direction::NONE)
							{
								// paskudny hack rozwiązujący problem blokowania się na ścianie, jeśli kolidujemy z nią podczas spadania
								if (new_collision.collided_wall == direction::S)
								{
									u32 upper_tile_value = get_tile_value(game->current_level, tile_x_to_check, tile_y_to_check - 1);
									if (is_tile_colliding(game->collision_reference, upper_tile_value))
									{
										if ((moving_entity->position - get_tile_v2_position(tile_x_to_check, tile_y_to_check)).x > 0)
										{
											new_collision.collided_wall_normal = get_v2(-1, 0);
										}
										else
										{
											new_collision.collided_wall_normal = get_v2(1, 0);
										}
									}
								}

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
							get_entity_collision_data(moving_entity), 
							get_entity_collision_data(entity_to_check), 
							movement_delta, closest_effect_entity_collision.possible_movement_perc);

						if (new_collision.collided_wall != direction::NONE)
						{
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
				moving_entity->position += possible_movement;
				// pozostałą deltę zmniejszamy o tyle, o ile się poruszyliśmy
				movement_delta -= possible_movement;
				//printf("przesuniecie o (%.04f, %.04f)\n", possible_movement.x, possible_movement.y);
			}

			// jeśli z entity mieliśmy kolizję wcześniej niż z tile
			if (closest_effect_entity_collision.possible_movement_perc < closest_collision.possible_movement_perc)
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

b32 move_bullet(game_data* game, bullet* moving_bullet, u32 bullet_index, v2 target_pos)
{
	v2 movement_delta = target_pos - moving_bullet->position;
	b32 was_collision = false;
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
					u32 tile_value = get_tile_value(game->current_level, tile_x_to_check, tile_y_to_check);
					if (is_tile_colliding(game->collision_reference, tile_value))
					{
						collision new_collision = check_minkowski_collision(
							get_bullet_collision_data(moving_bullet),
							get_tile_collision_data(tile_x_to_check, tile_y_to_check),
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
						get_bullet_collision_data(moving_bullet),
						get_entity_collision_data(entity_to_check),
						movement_delta, closest_collision.possible_movement_perc);

					if (new_collision.collided_wall != direction::NONE)
					{		
						if (are_flags_set((u32*)&moving_bullet->type->flags, entity_flags::DAMAGES_PLAYER))
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
			moving_bullet->position += possible_movement;
			movement_delta -= possible_movement;
		}

		if (collided_entity)
		{
			// trzeba wymyślić lepszy sposób na sprawdzenie, czy to gracz
			if (collided_entity == &game->entities[0])
			{
				damage_player(game, moving_bullet->type->damage_on_contact);
			}
			else
			{
				start_visual_effect(game, collided_entity, 0, false);
				collided_entity->health -= moving_bullet->type->damage_on_contact;
				printf("pocisk trafil w entity, %i obrazen, zostalo %i\n", moving_bullet->type->damage_on_contact, collided_entity->health);
			}
		}

		if (was_collision)
		{
			//printf("pocisk usuniety\n");
			remove_bullet(game, bullet_index);
		}
	}

	return was_collision;
}

void render_debug_information(sdl_game_data* sdl_game, game_data* game)
{
	entity* player = get_player(game);
	
	b32 is_standing = is_standing_on_ground(game->current_level, game->collision_reference, player);

	char buffer[200];
	SDL_Color text_color = { 255, 255, 255, 0 };
	/*int error = SDL_snprintf(buffer, 200, "Frame: %d Elapsed: %0.2f ms, Pos: (%0.2f,%0.2f) Acc: (%0.2f,%0.2f) Standing: %d ",
		sdl_game->debug_frame_counter, sdl_game->debug_elapsed_work_ms, player->position.x, player->position.y,
		player->acceleration.x, player->acceleration.y, is_standing);*/
	int error = SDL_snprintf(buffer, 200, "Pos: (%0.2f,%0.2f) Acc: (%0.2f,%0.2f) Standing: %d, Direction: %s",
		player->position.x, player->position.y,
		player->acceleration.x, player->acceleration.y, is_standing, (player->direction == direction::W? "W" : "E"));
	render_text(sdl_game, buffer, 10, 100, text_color);
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
	assert(test_input->up.number_of_presses == test_input_count - 1)

	test_input = get_past_input(input_buf, 1);
	assert(test_input->up.number_of_presses == test_input_count - 2)

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

v2 process_input(game_data* game, entity* player, r32 delta_time)
{	
	game_input* input = get_last_frame_input(&game->input);

	b32 is_standing_at_frame_beginning = is_standing_on_ground(
		game->current_level, game->collision_reference, player);

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

	v2 bullet_direction = (player->direction == direction::W ? get_v2(-1.0f, 0.0f) : get_v2(1.0f, 0.0f));
	v2 bullet_offset = (player->direction == direction::W ? get_v2(-0.75f, -0.85f) : get_v2(0.75f, -0.85f));

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

			if (input->fire.number_of_presses > 0)
			{
				if (player->attack_cooldown <= 0)
				{					
					add_bullet(game, player->type->fired_bullet_type, player->position + bullet_offset,
						bullet_direction * player->type->fired_bullet_type->constant_velocity);
					printf("strzal\n");
					player->attack_cooldown = player->type->default_attack_cooldown;
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
		}
		break;
		case movement_mode::JUMP:
		{
			player->acceleration = gravity;

			if (input->fire.number_of_presses > 0)
			{
				if (player->attack_cooldown <= 0)
				{
					add_bullet(game, player->type->fired_bullet_type, player->position + bullet_offset,
						bullet_direction * player->type->fired_bullet_type->constant_velocity);
					printf("strzal w skoku!\n");;
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
				player->acceleration +=game->player_movement.recoil_acceleration;
			}			
		}
		break;
	}

	player->velocity = player->type->slowdown_multiplier *
		(player->velocity + (player->acceleration * delta_time));

	v2 target_pos = player->position +
		(player->velocity * player->type->velocity_multiplier * delta_time);

	return target_pos;
}

void animate(player_movement* movement, entity* entity, r32 delta_time)
{
	if (entity->visual_effect)
	{
		if (entity->visual_effect_duration < entity->visual_effect->total_duration)
		{
			entity->visual_effect_duration += delta_time;
		}
		else
		{
			entity->visual_effect = NULL;
		}
	}

	if (entity->current_animation)
	{
		entity->animation_duration += delta_time;
		while (entity->animation_duration > entity->current_animation->total_duration)
		{
			entity->animation_duration -= entity->current_animation->total_duration;
		}
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

void render_entity_sprite(SDL_Renderer* renderer, v2 screen_center,
	v2 player_position, v2 entity_position, direction entity_direction,
	sprite_effect* visual_effect, r32 visual_effect_duration, sprite sprite)
{
	b32 tint_modified = false;
	SDL_Color tint = { 255,255,255,255 };

	if (visual_effect)
	{
		tint_modified = true;
		tint = get_tint(visual_effect, visual_effect_duration);
	}

	for (u32 part_index = 0; part_index < sprite.parts_count; part_index++)
	{
		sprite_part* part = &sprite.parts[part_index];

		if (tint_modified)
		{
			SDL_SetTextureColorMod(part->texture, tint.r, tint.g, tint.b);
		}

		v2 flipped_offset = part->offset;
		SDL_RendererFlip flip = SDL_FLIP_NONE;
		if (entity_direction != part->default_direction)
		{
			flip = SDL_FLIP_HORIZONTAL; // SDL_FLIP_VERTICAL
			flipped_offset = get_v2(-part->offset.x, part->offset.y);
		}

		v2 position = screen_center + (entity_position - player_position);
		SDL_Rect screen_rect = {};
		screen_rect.w = part->texture_rect.w;
		screen_rect.h = part->texture_rect.h;
		screen_rect.x = (position.x * TILE_SIDE_IN_PIXELS) - (part->texture_rect.w / 2) + flipped_offset.x;
		screen_rect.y = (position.y * TILE_SIDE_IN_PIXELS) - (part->texture_rect.h / 2) + flipped_offset.y;
		
		SDL_RenderCopyEx(renderer, part->texture, &part->texture_rect, &screen_rect, 0, NULL, flip);

		if (tint_modified)
		{
			// przywracamy domyślne ustawienia
			SDL_SetTextureColorMod(part->texture, 255, 255, 255);
		}
	}
}

void render_entity_animation_frame(SDL_Renderer* renderer, v2 screen_center, v2 player_position, entity* entity)
{
	sprite* sprite_to_render = NULL;
	if (entity->current_animation)
	{
		sprite_to_render = get_sprite_from_animation(entity->current_animation, &entity->animation_duration);
		if (sprite_to_render == NULL || sprite_to_render->parts == NULL)
		{
			sprite_to_render = &entity->type->idle_pose;
		}
	}
	else
	{
		sprite_to_render = &entity->type->idle_pose;
	}

	render_entity_sprite(renderer, screen_center,
		player_position, entity->position, entity->direction,
		entity->visual_effect, entity->visual_effect_duration, *sprite_to_render);
}

void update_and_render(sdl_game_data* sdl_game, game_data* game, r32 delta_time)
{	
	entity* player = get_player(game);

	// update player
	{
		if (game->player_invincibility_cooldown > 0.0f)
		{
			game->player_invincibility_cooldown -= delta_time;
			//printf("niezniszczalnosc jeszcze przez %.02f\n", game->player_invincibility_cooldown);
		}

		animate(&game->player_movement, player, delta_time);

		v2 target_pos = process_input(game, player, delta_time);

		collision_with_effect collision = move(sdl_game, game, player, target_pos);
		if (collision.collided_entity)
		{
			damage_player(game, collision.collided_entity->type->damage_on_contact);

			v2 direction = player->position - collision.collided_entity->position;
			r32 acceleration = collision.collided_entity->type->player_acceleration_on_collision;

			game->player_movement.recoil_timer = 2.0f;
			game->player_movement.recoil_acceleration_timer = 1.0f;
			game->player_movement.recoil_acceleration = (direction * acceleration);

			printf("odrzut! nowe przyspieszenie: (%.02f,%.02f)\n",
				game->player_movement.recoil_acceleration.x,
				game->player_movement.recoil_acceleration.y);

			change_movement_mode(&game->player_movement, movement_mode::RECOIL);			
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

		animate(NULL, entity, delta_time);

		if (entity->health <= 0)
		{
			remove_entity(game, entity_index);
			entity_index--; // ze względu na działanie compact array
		}

		if (entity->attack_cooldown > 0)
		{
			entity->attack_cooldown -= delta_time;
		}

		if (are_entity_flags_set(entity, entity_flags::ENEMY)
			&& entity->type->fired_bullet_type)
		{
			if (entity->attack_cooldown <= 0)
			{
				v2 player_relative_pos = player->position - entity->position;
				r32 distance_to_player = length(player_relative_pos);
				if (distance_to_player < 5.0f)
				{
					v2 direction_to_player = get_unit_vector(player_relative_pos);

					entity->direction = direction_to_player.x < 0 ? direction::W : direction::E;

					add_bullet(game, entity->type->fired_bullet_type, entity->position/* + get_v2(1.0f, 1.0f)*/,
						player->type->fired_bullet_type->constant_velocity * direction_to_player);
					//printf("wrog strzela!\n");

					entity->attack_cooldown = entity->type->default_attack_cooldown;
				}
			}
		}
	}

	// update bullets
	for (u32 bullet_index = 0; bullet_index < game->bullets_count; bullet_index++)
	{
		bullet* bullet = game->bullets + bullet_index;
		if (bullet->type)
		{
			v2 bullet_target_pos = bullet->position + (bullet->velocity * delta_time);
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

	// rendering
	{
		SDL_SetRenderDrawColor(sdl_game->renderer, 0, 255, 0, 0);
		SDL_RenderClear(sdl_game->renderer);

		tile_position player_tile_pos = get_tile_position(player->position);
		v2 player_offset_in_tile = get_v2(
			player->position.x - (r32)player_tile_pos.x,
			player->position.y - (r32)player_tile_pos.y);

		i32 screen_half_width = SDL_ceil(HALF_SCREEN_WIDTH_IN_TILES) + 1;
		i32 screen_half_height = SDL_ceil(HALF_SCREEN_HEIGHT_IN_TILES) + 1;

		// draw tiles
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

				v2 position = get_v2(x_coord_on_screen, y_coord_on_screen) - player_offset_in_tile;
				SDL_Rect screen_rect = {};
				screen_rect.w = TILE_SIDE_IN_PIXELS;
				screen_rect.h = TILE_SIDE_IN_PIXELS;
				screen_rect.x = (position.x * TILE_SIDE_IN_PIXELS) - (TILE_SIDE_IN_PIXELS / 2);
				screen_rect.y = (position.y * TILE_SIDE_IN_PIXELS) - (TILE_SIDE_IN_PIXELS / 2);
					
				SDL_RenderCopy(sdl_game->renderer, sdl_game->tileset_texture, &tile_bitmap, &screen_rect);
			}
		}

		v2 screen_center = get_v2(HALF_SCREEN_WIDTH_IN_TILES + 1, HALF_SCREEN_HEIGHT_IN_TILES + 1);

		// draw entities
		for (u32 entity_index = 0; entity_index < game->entities_count; entity_index++)
		{
			entity* entity = game->entities + entity_index;
		
			render_entity_animation_frame(sdl_game->renderer, screen_center, player->position, entity);
		}

		// draw bullets
		for (u32 bullet_index = 0; bullet_index < game->bullets_count; bullet_index++)
		{
			bullet* bullet = game->bullets + bullet_index;
			render_entity_sprite(sdl_game->renderer, screen_center,
				player->position, bullet->position, direction::NONE,
				NULL, 0, bullet->type->idle_pose);
		}

		// draw debug info
		{
#if 1
			v2 screen_half_size = get_v2(screen_half_width, screen_half_height) * TILE_SIDE_IN_PIXELS;
			rect current_player_collision_rect = get_rect_from_center(
				screen_half_size + (player->type->collision_rect_offset * TILE_SIDE_IN_PIXELS),
				player->type->collision_rect_dim * TILE_SIDE_IN_PIXELS);
			render_rect(sdl_game, current_player_collision_rect);

			render_debug_information(sdl_game, game);
#endif
		}

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

		memory_arena arena = {};
		u32 memory_for_permanent_arena_size = megabytes_to_bytes(100);
		void* memory_for_permanent_arena = SDL_malloc(memory_for_permanent_arena_size);
		initialize_memory_arena(&arena, memory_for_permanent_arena_size, (byte*)memory_for_permanent_arena);

		circular_buffer_test(&arena);

		game_data* game = push_struct(&arena, game_data);
		game->input.size = 60 * 2; // 2 sekundy
		game->input.buffer = push_array(&arena, game->input.size, game_input);

		std::string collision_file_path = "data/collision_map.tmx";
		read_file_result collision_file = read_file(collision_file_path);
		game->collision_reference = read_level_from_tmx_file(&arena, collision_file, "collision");

		std::string map_file_path = "data/map_01.tmx";
		read_file_result map_file = read_file(map_file_path);
		game->current_level = read_level_from_tmx_file(&arena, map_file, "map");

		game->entity_types_count = 5;
		game->entity_types = push_array(&arena, game->entity_types_count, entity_type);		
		game->entities_count = 0;
		game->entities_max_count = 1000;
		game->entities = push_array(&arena, game->entities_max_count, entity);

		game->default_player_invincibility_cooldown = 2.0f;
		game->player_invincibility_cooldown = 0.0f;

		entity_type* player_entity_type = &game->entity_types[0];
		player_entity_type->idle_pose = get_player_idle_pose(&sdl_game, &arena);
		player_entity_type->flags = (entity_flags)(entity_flags::COLLIDES | entity_flags::PLAYER);
		player_entity_type->max_health = 100;
		player_entity_type->velocity_multiplier = 40.0f;
		player_entity_type->slowdown_multiplier = 0.80f;
		player_entity_type->default_attack_cooldown = 0.1f;
		player_entity_type->walk_animation = get_player_walk_animation(&sdl_game, &arena);
		player_entity_type->collision_rect_dim = get_v2(0.35f, 1.6f);
		player_entity_type->collision_rect_offset = 
			get_standing_collision_rect_offset(player_entity_type->collision_rect_dim);

		entity* player = add_entity(game, get_v2(0, 0), player_entity_type);

		game->player_movement.current_mode = movement_mode::WALK;
		
		entity_type* default_entity_type = &game->entity_types[1];
		default_entity_type->idle_pose = get_tile_graphics(&sdl_game, &arena, 837);
		default_entity_type->flags = (entity_flags)(entity_flags::COLLIDES | entity_flags::ENEMY);
		default_entity_type->max_health = 10;
		default_entity_type->damage_on_contact = 10;
		default_entity_type->default_attack_cooldown = 0.5f;
		default_entity_type->player_acceleration_on_collision = 5.0f;
		default_entity_type->collision_rect_dim = get_v2(1.0f, 1.0f);
		default_entity_type->collision_rect_offset = 
			get_standing_collision_rect_offset(default_entity_type->collision_rect_dim);

		add_entity(game, get_v2(16.0f, 6.0f), default_entity_type);
		add_entity(game, get_v2(18.0f, 6.0f), default_entity_type);
		add_entity(game, get_v2(20.0f, 6.0f), default_entity_type);

		game->bullet_types_count = 5;
		game->bullet_types = push_array(&arena, game->bullet_types_count, entity_type);
		game->bullets_count = 0;
		game->bullets_max_count = 5000;
		game->bullets = push_array(&arena, game->bullets_max_count, bullet);

		entity_type* player_bullet_type = &game->bullet_types[0];
		player_bullet_type->damage_on_contact = 5;
		player_bullet_type->constant_velocity = 12.0f;
		player_bullet_type->idle_pose = get_bullet_graphics(&sdl_game, &arena, 1, 1);

		entity_type* enemy_bullet_type = &game->bullet_types[1];
		enemy_bullet_type->damage_on_contact = 5;
		enemy_bullet_type->flags = entity_flags::DAMAGES_PLAYER;
		enemy_bullet_type->constant_velocity = 12.0f;
		enemy_bullet_type->idle_pose = get_bullet_graphics(&sdl_game, &arena, 1, 1);
			
		player_entity_type->fired_bullet_type = player_bullet_type;
		default_entity_type->fired_bullet_type = enemy_bullet_type;

		game->visual_effects_count = 5;
		game->visual_effects = push_array(&arena, game->visual_effects_count, sprite_effect);

		sprite_effect* damage_tint_effect = &game->visual_effects[0];
		damage_tint_effect->stages_count = 1;
		damage_tint_effect->stages = push_array(&arena, damage_tint_effect->stages_count, sprite_effect_stage);
		damage_tint_effect->color = { 255, 0, 0, 0 };

		add_sprite_effect_stage(damage_tint_effect, 1.0f, 0.0f, 0.0f, 5.0f, 5.0f);
		add_constant_tint_sprite_effect_stage(damage_tint_effect, 0.5f, 5.0f);

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

