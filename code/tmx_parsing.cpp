#include <SDL.h>
#include "tmx_parsing.h"
#include "jorutils.h"

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

	while (src_buf[index] != '\0' // null terminating string
		&& index < file.size) // zabezpieczenie
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

		// po enterze i po spacjach może zakończyć się plik
		if (src_buf[index] == '\0')
		{
			break;
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
				if (src_buf[index] == '/' && src_buf[index + 1] == '>')
				{
					index += 2;
					break;
				}

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
				char* data_lex_buf = new char[1024 * 1024];
				int data_lex_index = 0;

				// pomijamy to, co poza buforem
				while (src_buf[index] != '<' && data_lex_index < 1024 * 1024)
				{
					data_lex_buf[data_lex_index++] = src_buf[index++];
				}
				data_lex_buf[data_lex_index] = '\0';
				data_lex_buf[(1024 * 1024) - 1] = '\0'; // zabezpieczenie

				current_node->inner_text = SDL_strdup(data_lex_buf);

				delete[] data_lex_buf;
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

	delete[] lex_buf;
}