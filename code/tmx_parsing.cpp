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

struct string_ref
{
	int string_size;
	char* ptr;
};

enum xml_token_type
{
	XML_TOKEN_LEFT_CHEVRON, // <
	XML_TOKEN_CLOSING_LEFT_CHEVRON, // </
	XML_TOKEN_RIGHT_CHEVRON, // >
	XML_TOKEN_SELF_CLOSE_RIGHT_CHEVRON, // />
	XML_TOKEN_LEFT_PROLOG_CHEVRON, // <?
	XML_TOKEN_RIGHT_PROLOG_CHEVRON, // ?>
	XML_TOKEN_TAG,
	XML_TOKEN_ATTRIBUTE_NAME,
	XML_TOKEN_ATTRIBUTE_VALUE,
	XML_TOKEN_INNER_TEXT,
	XML_END_OF_FILE
};

struct xml_token
{
	xml_token_type type;
	string_ref value;
	xml_token* next;
};

struct scanner
{
	char* text;
	u32 text_length;

	//char* temp_buf;
	//u32 temp_buf_index;
	//u32 temp_buf_length;

	u32 current_char_index;
	//u32 current_lexeme_start_char;

	xml_token* first_token;
	xml_token* last_token;
	u32 token_count;
	memory_arena* arena;
};

inline b32 is_at_end(scanner* scan)
{
	b32 result = (scan->current_char_index >= scan->text_length);

	if (result)
	{
		debug_breakpoint;
	}

	return result;
}

//inline void copy_current_char_to_temp_buffer(scanner* scan)
//{
//	if (scan->temp_buf_index < scan->temp_buf_length)
//	{
//		*(scan->temp_buf + scan->temp_buf_index) = scan->current_char;
//		scan->temp_buf_index++;
//	}
//}
//
//inline void append_null_terminator_to_temp_buffer(scanner* scan)
//{
//	if (scan->temp_buf_index < scan->temp_buf_length)
//	{
//		*(scan->temp_buf + scan->temp_buf_index) = '\0';
//		scan->temp_buf_index++;
//	}
//	else
//	{
//		invalid_code_path;
//	}
//}

//inline void reset_temp_buffer(scanner* scan)
//{
//	scan->temp_buf_index = 0;
//}

xml_token* add_token(scanner* scan, xml_token_type type)
{
	xml_token* new_token = push_struct(scan->arena, xml_token);
	new_token->type = type;

	if (scan->last_token)
	{
		scan->last_token->next = new_token;
		scan->last_token = scan->last_token->next;
	}
	else
	{
		scan->first_token = new_token;
		scan->last_token = new_token;
	}
	scan->token_count++;

	return new_token;
}

xml_token* add_token(scanner* scan, xml_token_type type, string_ref value)
{
	xml_token* new_token = add_token(scan, type);
	if (value.ptr)
	{
		new_token->value = value;
	}
	return new_token;
}

string_ref copy_string_to_memory_arena(memory_arena* arena, const char* str, int str_size)
{
	string_ref result = {};
	result.string_size = str_size;
	result.ptr = (char*)push_size(arena, str_size + 1);

	for (int index = 0; 
		index < str_size; 
		index++)
	{
		*(result.ptr + index) = *(str + index);
	}
	// pamięć jest domyślnie wyzerowana, ale na wszelki wypadek dodajemy null terminator:
	// dzięki temu jesteśmy bezpieczni nawet wtedy, gdy orginalny str nie miał null terminatora
	// lub gdy został podany błędny str_size, i nie skopiowaliśmy całości
	*(result.ptr + str_size) = '\0';

	return result;
}

inline char get_current_char(scanner* scan)
{
	if (false == is_at_end(scan))
	{
		char result = *(scan->text + scan->current_char_index);
		return result;
	}
	else
	{
		return '\0';
	}	
}

inline void advance(scanner* scan)
{
	if (false == is_at_end(scan))
	{
		scan->current_char_index++;
	}
}

inline char peek(scanner* scan)
{
	if (false == is_at_end(scan))
		// czy to jest potrzebne? może po prostu powinniśmy zaalokować jeden bajt więcej na ostatni peek?
	{
		char result = *(scan->text + scan->current_char_index + 1);
		return result;
	}
	else
	{
		return '\0';
	}
}

inline b32 is_whitespace(char c)
{
	b32 result = (c == ' ' || c == '\r' || c == '\t' || c == '\n');
	return result;
}

void omit_whitespace(scanner* scan)
{
	while (is_whitespace(peek(scan)))
	{
		scan->current_char_index++;
	}
}

b32 is_charater_allowed_for_text(char c, bool allow_whitespace)
{
	b32 result = (c != '\0'
		&& c != '"'
		&& c != '<'
		&& c != '='
		&& c != '>'
		&& (is_whitespace(c) == false || allow_whitespace));
	return result;
}

string_ref scan_text(scanner* scan, bool allow_whitespace = true)
{
	string_ref ref_to_source = {};

	u32 lexeme_start_char_index = scan->current_char_index;
	char next_c = peek(scan);
	while (is_charater_allowed_for_text(next_c, allow_whitespace))
	{
		advance(scan);
		next_c = peek(scan);
	}
	u32 lexeme_end_char_index = scan->current_char_index;
	u32 lexeme_length = lexeme_end_char_index - lexeme_start_char_index + 1;
	char* lexeme_start_ptr = scan->text + lexeme_start_char_index;
	
	ref_to_source.string_size = lexeme_length;
	ref_to_source.ptr = lexeme_start_ptr;

	return ref_to_source;
}

bool scan_token(scanner* scan)
{
	char c = get_current_char(scan);
	switch (c)
	{
		case '<':
		{
			char next_c = peek(scan);
			if (next_c == '?')
			{
				advance(scan);
				add_token(scan, xml_token_type::XML_TOKEN_LEFT_PROLOG_CHEVRON);
			}
			else if (next_c == '/')
			{
				advance(scan);
				add_token(scan, xml_token_type::XML_TOKEN_CLOSING_LEFT_CHEVRON);
			}
			else
			{
				add_token(scan, xml_token_type::XML_TOKEN_LEFT_CHEVRON);
			}

			omit_whitespace(scan);

			if (is_charater_allowed_for_text(peek(scan), false))
			{
				advance(scan);
				string_ref tag_name = scan_text(scan, false);
				add_token(scan, xml_token_type::XML_TOKEN_TAG, tag_name);
			}
		}
		break;
		case '?':
		{
			char next_c = peek(scan);
			if (next_c == '>')
			{
				advance(scan);
				add_token(scan, xml_token_type::XML_TOKEN_RIGHT_PROLOG_CHEVRON);

				omit_whitespace(scan);
				if (is_charater_allowed_for_text(peek(scan), true))
				{
					advance(scan);
					string_ref inner_text = scan_text(scan);
					add_token(scan, xml_token_type::XML_TOKEN_INNER_TEXT, inner_text);
				}
			}
			else
			{
				// błąd - samodzielny ?
				invalid_code_path;
			}
		}
		break;
		case '>':
		{
			add_token(scan, xml_token_type::XML_TOKEN_RIGHT_CHEVRON);			
			advance(scan);
			
			if (is_charater_allowed_for_text(peek(scan), true))
			{
				string_ref inner_text = scan_text(scan);
				add_token(scan, xml_token_type::XML_TOKEN_INNER_TEXT, inner_text);
			}

		} 
		break;
		case '/':
		{
			char next_c = peek(scan);
			if (next_c == '>')
			{
				advance(scan);
				add_token(scan, xml_token_type::XML_TOKEN_SELF_CLOSE_RIGHT_CHEVRON);
			}
			else
			{
				// błąd - samodzielny / bez >
				invalid_code_path;
			}
		}
		break;
		case '=':
		{
			while (peek(scan) != '"' && false == is_at_end(scan))
			{
				advance(scan);
			}

			omit_whitespace(scan);

			if (peek(scan) == '"')
			{
				advance(scan);
			}
			else
			{
				// mamy błąd - było = bez "
				invalid_code_path;
			}

			if (is_charater_allowed_for_text(peek(scan), true))
			{
				advance(scan);
				string_ref attr_value = scan_text(scan);
				add_token(scan, xml_token_type::XML_TOKEN_ATTRIBUTE_VALUE, attr_value);
			}
			else
			{
				// błąd - jako value nie mamy legalnego tekstu
				invalid_code_path;
			}

			if (peek(scan) == '"')
			{
				advance(scan);
			}
		}
		break;
		case '"':
		{
			// błąd - nie powinniśmy napotykać " poza ==
			invalid_code_path;
		}
		break;

		// ignorujemy whitespace
		case ' ':
		case '\r':
		case '\t':
		case '\n':
		break;

		default:
		{			
			// w tym wypadku mamy attribute name - przypadek inner tekst jest obsłużony przy >
			if (is_charater_allowed_for_text(c, false))
			{
				string_ref attr_value = scan_text(scan, false);
				add_token(scan, xml_token_type::XML_TOKEN_ATTRIBUTE_NAME, attr_value);
			}
		}
		break;
	}

	advance(scan);
	if (is_at_end(scan))
	{		
		return false;
	}
	else
	{
		return true;
	}
}

void better_parse_tilemap(read_file_result file)
{
	int memory_for_parsing_size = megabytes_to_bytes(10);
	void* memory_for_parsing = SDL_malloc(memory_for_parsing_size);

	memory_arena parsing_arena = {};
	initialize_memory_arena(&parsing_arena, memory_for_parsing_size, (byte*)memory_for_parsing);

	const char* string_test = "no i co teraz kurde balans";
	int string_test_count = SDL_strlen(string_test);

	string_ref str = {};
	str.string_size = string_test_count;
	
	copy_string_to_memory_arena(&parsing_arena, string_test, string_test_count);

	xml_token* tk = push_struct(&parsing_arena, xml_token);

	scanner scan = {};
	scan.text = (char*)file.contents;
	scan.text_length = file.size;
	//scan.temp_buf;
	//scan.temp_buf_index;
	//scan.temp_buf_length;
	//scan.current_char;
	//scan.current_lexeme_start_char;
	scan.last_token = NULL;
	scan.token_count = 0;
	scan.arena = &parsing_arena;

	while (scan_token(&scan));
	add_token(&scan, xml_token_type::XML_END_OF_FILE);

	SDL_free(memory_for_parsing);
}
