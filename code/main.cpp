#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <stdio.h>
#include <string>

//Screen dimension constants
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define assert(statement) {if (!(statement)) { int x = 1; x = x / 0; }}
#define invalid_code_path { int x = 1; x = x / 0; }

struct sdl_game_data
{
	bool initialized;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* tileset_texture;
	TTF_Font* font;
};

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

struct read_file_result
{
	void* contents;
	int size;
};

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

enum tmx_node_type // już wszystkie
{
	TMX_NODE_OTHER,
	TMX_NODE_MAP,
	TMX_NODE_TILESET,
	TMX_NODE_LAYER,
	TMX_NODE_DATA,
	TMX_NODE_OBJECTGROUP,
	TMX_NODE_OBJECT,
};

enum tmx_attr_type // do uzupełnienia
{
	TMX_ATTR_TYPE_UNKNOWN,
	TMX_ATTR_ID,
	TMX_ATTR_NAME,
	TMX_ATTR_CLASS,
	TMX_ATTR_X,
	TMX_ATTR_Y,
	TMX_ATTR_WIDTH,
	TMX_ATTR_HEIGHT,
	TMX_ATTR_TILEWIDTH,
	TMX_ATTR_TILEHEIGHT,
	TMX_ATTR_ORIENTATION,
	TMX_ATTR_RENDERORDER,
	TMX_ATTR_SOURCE,
	TMX_ATTR_ENCODING,
};

struct tmx_attribute
{
	tmx_attr_type type;
	char* value;
};

struct tmx_node
{
	tmx_node_type type;
	tmx_attribute* attributes;
	int attributes_count;
	char* inner_text;
};

#define ATTR_MAX_COUNT 10
#define LEXER_BUFFER_SIZE 256

tmx_node_type parse_node_type(char* lexer_buffer)
{
	tmx_node_type type = tmx_node_type::TMX_NODE_OTHER;
	if (SDL_strncmp(lexer_buffer, "map", LEXER_BUFFER_SIZE) == 0)
	{
		type = tmx_node_type::TMX_NODE_MAP;
	}
	else if (SDL_strncmp(lexer_buffer, "tileset", LEXER_BUFFER_SIZE) == 0)
	{
		type = tmx_node_type::TMX_NODE_TILESET;
	}
	else if (SDL_strncmp(lexer_buffer, "layer", LEXER_BUFFER_SIZE) == 0)
	{
		type = tmx_node_type::TMX_NODE_LAYER;
	}
	else if (SDL_strncmp(lexer_buffer, "data", LEXER_BUFFER_SIZE) == 0)
	{
		type = tmx_node_type::TMX_NODE_DATA;
	}
	else if (SDL_strncmp(lexer_buffer, "objectgroup", LEXER_BUFFER_SIZE) == 0)
	{
		type = tmx_node_type::TMX_NODE_OBJECTGROUP;
	}
	else if (SDL_strncmp(lexer_buffer, "object", LEXER_BUFFER_SIZE) == 0)
	{
		type = tmx_node_type::TMX_NODE_OBJECT;
	}
	return type;
}

tmx_attr_type parse_attr_type(char* lexer_buffer, tmx_node_type node_type)
{
	tmx_attr_type type = tmx_attr_type::TMX_ATTR_TYPE_UNKNOWN;
	switch (node_type)
	{	
		case tmx_node_type::TMX_NODE_MAP: 
		{		
			if (SDL_strncmp(lexer_buffer, "width", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_WIDTH;
			}
			else if (SDL_strncmp(lexer_buffer, "height", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_HEIGHT;
			}
			else if (SDL_strncmp(lexer_buffer, "tilewidth", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_TILEWIDTH;
			}
			else if (SDL_strncmp(lexer_buffer, "tileheight", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_TILEHEIGHT;
			}
			else if (SDL_strncmp(lexer_buffer, "renderorder", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_RENDERORDER;
			}
			else if (SDL_strncmp(lexer_buffer, "orientation", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_ORIENTATION;
			}
		} break;
		case tmx_node_type::TMX_NODE_TILESET:
		{
			if (SDL_strncmp(lexer_buffer, "source", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_SOURCE;
			}
		} break;
		case tmx_node_type::TMX_NODE_LAYER:
		{
			if (SDL_strncmp(lexer_buffer, "id", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_ID;
			}
			else if (SDL_strncmp(lexer_buffer, "name", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_NAME;
			}
			else if (SDL_strncmp(lexer_buffer, "width", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_WIDTH;
			}
			else if (SDL_strncmp(lexer_buffer, "height", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_HEIGHT;
			}
		} break;
		case tmx_node_type::TMX_NODE_DATA:
		{
			if (SDL_strncmp(lexer_buffer, "encoding", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_ENCODING;
			}
		} break;
		case tmx_node_type::TMX_NODE_OBJECTGROUP:
		{
			if (SDL_strncmp(lexer_buffer, "id", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_ID;
			}
			else if (SDL_strncmp(lexer_buffer, "name", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_NAME;
			}
		} break;
		case  tmx_node_type::TMX_NODE_OBJECT:
		{
			if (SDL_strncmp(lexer_buffer, "id", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_ID;
			}
			else if (SDL_strncmp(lexer_buffer, "name", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_NAME;
			}
			else if (SDL_strncmp(lexer_buffer, "class", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_CLASS;
			}
			else if (SDL_strncmp(lexer_buffer, "x", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_X;
			}
			else if (SDL_strncmp(lexer_buffer, "y", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_Y;
			}
			else if (SDL_strncmp(lexer_buffer, "width", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_WIDTH;
			}
			else if (SDL_strncmp(lexer_buffer, "height", LEXER_BUFFER_SIZE) == 0)
			{
				type = tmx_attr_type::TMX_ATTR_HEIGHT;
			}
		} break;
	}
	
	return type;
}

void parse_tilemap(read_file_result file)
{
	assert(file.contents && file.size);

	char* src_buf = (char*)file.contents;
	char* lex_buf = new char[LEXER_BUFFER_SIZE];
	int index = 0;
	int lex_index = 0;

	tmx_node* node_array = new tmx_node[1024];
	int node_count = 0;

	tmx_node* current_node = 0;

	// pomijamy nagłówek XML
	if (src_buf[index] == '<' && src_buf[index + 1] == '?')
	{
		while (src_buf[index] != '>')
		{
			index++;
		}
		index++;
	}

	while (src_buf[index] != '\0') // null terminating string
	{
		// pomijamy nowe linie
		if (src_buf[index] == '\r' && src_buf[index + 1] == '\n')
		{
			index += 2;
		}

		// po nowej linii mogą wystąpić spacje
		while (src_buf[index] == ' ')
		{
			index++;
		}

		if (src_buf[index] == '<')
		{
			// pomijanie, jeśli < zaczynał zamykający tag
			if (src_buf[index + 1] == '/')
			{	
				while (src_buf[index] != '>')
				{
					index++;
				}
				index++; // idziemy za '>'
				continue;
			}

			// czytanie taga
			index++;
			while (src_buf[index] != ' ' && src_buf[index] != '>')
			{
				lex_buf[lex_index++] = src_buf[index++];
			}
			lex_buf[lex_index] = '\0';
			tmx_node_type type = parse_node_type(lex_buf);

			// dodawanie nowego node
			node_array[node_count] = {};
			current_node = &node_array[node_count];
			node_count++;
			current_node->type = type;

			// czytanie atrybutów
			while (src_buf[index] != '>')
			{
				// teraz idziemy do przodu, aż napotkamy jakiś atrybut
				while (src_buf[index] == ' ')
				{
					index++;
				}
				lex_index = 0;

				while (src_buf[index] != ' ' && src_buf[index] != '=')
				{
					lex_buf[lex_index++] = src_buf[index++];
				}

				// ignorujemy spacje na końcu
				while (lex_buf[lex_index - 1] == ' ')
				{
					lex_index--;
				}
				lex_buf[lex_index] = '\0';

				if (current_node->attributes == NULL)
				{
					current_node->attributes = (tmx_attribute*)SDL_malloc(sizeof(tmx_attribute) * ATTR_MAX_COUNT);
				}

				tmx_attr_type new_attribute_type = parse_attr_type(lex_buf, current_node->type);			
				void* new_attribute_value = NULL;

				while (src_buf[index] == ' ' || src_buf[index] == '"')
				{
					index++;
				}

				if (src_buf[index] == '=')
				{
					index++;
					while (src_buf[index] != '"')
					{
						index++;
					}
					index++; // idziemy za "

					lex_index = 0;
					while (src_buf[index] != '"')
					{
						lex_buf[lex_index++] = src_buf[index++];
					}
					lex_buf[lex_index] = '\0';
					index++; // idziemy za "

					// tymczasowe
					new_attribute_value = SDL_strdup(lex_buf);
					lex_index = 0;
				}

				// zapisujemy tylko znane nodes
				if (new_attribute_type != tmx_attr_type::TMX_ATTR_TYPE_UNKNOWN 
					&& new_attribute_value != NULL)
				{
					tmx_attribute* current_attr = &current_node->attributes[current_node->attributes_count];
					current_attr->type = new_attribute_type;
					current_attr->value = (char*)new_attribute_value;
					current_node->attributes_count++;
					assert(current_node->attributes_count < ATTR_MAX_COUNT);
				}
			}

			if (src_buf[index] == '>')
			{
				index++;
			}
		}
		else
		{
			// tutaj odbywa się parsowanie zawartości node
			if (current_node->type == tmx_node_type::TMX_NODE_DATA)
			{
				// tutaj możemy mieć bardzo duży rozmiar - potrzebny jest większy bufor
				char* data_lex_buf = new char[1024*1024];
				int data_lex_index = 0;

				// pomijamy to, co poza buforem
				while (src_buf[index] != '<' && data_lex_index < 1024 * 1024)
				{
					data_lex_buf[data_lex_index++] = src_buf[index++];
				}
				data_lex_buf[data_lex_index] = '\0';
				data_lex_buf[(1024 * 1024) - 1] = '\0'; // zabezpieczenie

				current_node->inner_text = SDL_strdup(data_lex_buf);

				delete data_lex_buf;
			}
			else
			{
				while (src_buf[index] != '<')
				{
					index++;
				}
			}			
		}
	}

	int x = 1;

	delete lex_buf;
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

		//std::string map_path = "data/trifle_map_01.tmx";
		std::string map_path = "data/test.xml";
		read_file_result map = read_file(map_path);

		parse_tilemap(map);

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
			
			{
				SDL_Color text_color = { 255, 255, 255, 0 };
				std::string text_test = "najwyrazniej ten piekny font nie obsluguje polskich znakow";
				render_text(sdl_game, text_test, 0, 50, text_color);
			}

			SDL_RenderPresent(sdl_game.renderer);
		}

		delete map.contents;
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