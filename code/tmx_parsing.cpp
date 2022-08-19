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

b32 operator ==(string_ref a, string_ref b)
{
	b32 result = true;

	if (a.ptr == NULL || b.ptr == NULL)
	{
		invalid_code_path;
		result = false;
	}

	if (a.string_size != b.string_size)
	{
		result = false;
	}
	else
	{
		for (u32 char_index = 0;
			char_index < a.string_size;
			char_index++)
		{
			char a_char = *(a.ptr + char_index);
			char b_char = *(b.ptr + char_index);
			if (a_char != b_char)
			{
				result = false;
				break;
			}
		}
	}
	return result;
}

b32 compare_c_string(string_ref my_str, const char* c_str)
{
	b32 result = true;

	if (my_str.ptr == NULL || c_str == NULL)
	{
		result = false;
		invalid_code_path;
	}

	u32 my_str_char_index = 0;
	while (*c_str)
	{
		if (my_str_char_index == my_str.string_size)
		{
			// stringi są różnej długości - przeszliśmy my_str, a nie przeszliśmy c_str
			result = false;
			break;
		}

		char my_char = *(my_str.ptr + my_str_char_index);
		char c_char = *c_str;
		if (my_char != c_char)
		{
			result = false;
			break;
		}

		my_str_char_index++;
		c_str++;
	}

	if (my_str_char_index < my_str.string_size)
	{
		// stringi są różnej długości - przeszliśmy c_str, a nie przeszliśmy my_str
		result = false;
	}

	return result;
}

i64 string_to_int(string_ref str)
{
	i64 result = 0;
	if (str.ptr)
	{
		char* next_after_last_char = str.ptr + str.string_size;
		result = SDL_strtol((char*)str.ptr, &next_after_last_char, 10);
	}
	return result;
}

r64 string_to_float(string_ref str)
{
	r64 result = 0;
	if (str.ptr)
	{
		char* next_after_last_char = str.ptr + str.string_size;
		result = SDL_strtod((char*)str.ptr, &next_after_last_char);
	}
	return result;
}

char* get_c_str(memory_arena* arena, string_ref str)
{
	char* new_c_str = (char*)push_size(arena, str.string_size + 1);
	{
		char* new_c_str_char = new_c_str;
		for (u32 char_index = 0; char_index < str.string_size; )
		{
			*new_c_str_char = *(str.ptr + char_index);
			new_c_str_char++;
			char_index++;
		}
		*(new_c_str + str.string_size) = '\0';
	}
	return new_c_str;
}

inline b32 is_whitespace(char c)
{
	b32 result = (c == ' ' || c == '\r' || c == '\t' || c == '\n');
	return result;
}

i32* parse_array_of_ints(memory_arena* arena, u32 array_length, string_ref str, char delimiter)
{
	i32* arr = push_array(arena, array_length, i32);
	u32 current_int_index = 0;
	char* start = 0;
	char* end = 0;
	for (u32 char_index = 0;
		char_index < str.string_size;
		char_index++)
	{
		char* c = str.ptr + char_index;
		if (*c == delimiter)
		{
			end = c;
			if (start)
			{
				int new_int = SDL_strtol(start, &end, 10);
				if (current_int_index < array_length)
				{
					arr[current_int_index++] = new_int;
					start = 0;
					end = 0;
				}
				else
				{
					// błąd - wyczerpaliśmy miejsce, a jest jeszcze string do parsowania
					break;
				}
			}
			else
			{
				// przypadek gdy mamy podwójny delimiter bądź plik zaczął się od delimitera
				end = 0;
				continue;
			}
		}
		else if (SDL_isdigit(*c))
		{
			if (start == 0)
			{
				// zaczyna się nowa liczba
				start = c;
			}
			else
			{
				// mamy starą liczbę
				continue;
			}
		}
		else if (is_whitespace(*c))
		{
			if (start != 0)
			{
				// traktujemy jako delimiter
				end = c;
				int new_int = SDL_strtol(start, &end, 10);
				if (current_int_index < array_length)
				{
					arr[current_int_index++] = new_int;
					start = 0;
					end = 0;
				}
				else
				{
					// błąd - wyczerpaliśmy miejsce, a jest jeszcze string do parsowania
					break;
				}
			}
			else
			{
				// po prostu idziemy naprzód
				continue;
			}
		}
		else
		{
			// mamy błąd - napotkaliśmy coś, co nie jest ani liczbą, ani delimiterem...
			continue;
		}
	}
	return arr;
}

enum xml_token_type
{
	LEFT_CHEVRON, // <
	CLOSING_LEFT_CHEVRON, // </
	RIGHT_CHEVRON, // >
	SELF_CLOSING_RIGHT_CHEVRON, // />
	LEFT_PROLOG_CHEVRON, // <?
	RIGHT_PROLOG_CHEVRON, // ?>
	TAG,
	ATTRIBUTE_NAME,
	ATTRIBUTE_VALUE,
	INNER_TEXT
};

struct xml_token
{
	xml_token_type type;
	string_ref value;
	xml_token* next;
};

struct scanner
{
	char* source;
	u32 source_length;

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

struct parser
{
	scanner scan;
	u32 current_token_index;
	xml_token* current_token;
	memory_arena* arena;
	u32 nodes_count;
};

inline b32 is_at_end(scanner* scan)
{
	b32 result = (scan->current_char_index >= scan->source_length);

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
		char result = *(scan->source + scan->current_char_index);
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
		char result = *(scan->source + scan->current_char_index + 1);
		return result;
	}
	else
	{
		return '\0';
	}
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

b32 check_if_string_is_whitespace(string_ref inner_text)
{
	b32 result = true;
	for (u32 char_index = 0;
		char_index < inner_text.string_size;
		char_index++)
	{
		if (false == is_whitespace(*(inner_text.ptr + char_index)))
		{
			result = false;
			break;
		}
	}

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
	char* lexeme_start_ptr = scan->source + lexeme_start_char_index;

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
			add_token(scan, xml_token_type::LEFT_PROLOG_CHEVRON);
		}
		else if (next_c == '/')
		{
			advance(scan);
			add_token(scan, xml_token_type::CLOSING_LEFT_CHEVRON);
		}
		else
		{
			add_token(scan, xml_token_type::LEFT_CHEVRON);
		}

		omit_whitespace(scan);

		if (is_charater_allowed_for_text(peek(scan), false))
		{
			advance(scan);
			string_ref tag_name = scan_text(scan, false);
			add_token(scan, xml_token_type::TAG, tag_name);
		}
	}
	break;
	case '?':
	{
		char next_c = peek(scan);
		if (next_c == '>')
		{
			advance(scan);
			add_token(scan, xml_token_type::RIGHT_PROLOG_CHEVRON);

			omit_whitespace(scan);
			if (is_charater_allowed_for_text(peek(scan), true))
			{
				advance(scan);
				string_ref inner_text = scan_text(scan);
				add_token(scan, xml_token_type::INNER_TEXT, inner_text);
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
		add_token(scan, xml_token_type::RIGHT_CHEVRON);
		advance(scan);

		if (is_charater_allowed_for_text(peek(scan), true))
		{
			string_ref inner_text = scan_text(scan);
			if (false == check_if_string_is_whitespace(inner_text))
			{
				add_token(scan, xml_token_type::INNER_TEXT, inner_text);
			}
		}
	}
	break;
	case '/':
	{
		char next_c = peek(scan);
		if (next_c == '>')
		{
			advance(scan);
			add_token(scan, xml_token_type::SELF_CLOSING_RIGHT_CHEVRON);
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
			add_token(scan, xml_token_type::ATTRIBUTE_VALUE, attr_value);
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
			add_token(scan, xml_token_type::ATTRIBUTE_NAME, attr_value);
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

struct xml_attribute;

struct xml_node
{
	string_ref tag;
	xml_node* parent;

	xml_attribute* first_attribute;
	u32 attributes_count;

	xml_node* first_child;
	u32 children_count;

	//xml_node* prev;
	xml_node* next; // np. następne dziecko tego samego rodzica

	string_ref inner_text;
};

struct xml_attribute
{
	string_ref name;
	string_ref value;

	xml_node* owner;
	xml_attribute* next;
};

b32 is_at_end(parser* pars)
{
	b32 result = (pars->current_token_index >= pars->scan.token_count
		|| pars->current_token == NULL
		|| pars->current_token->next == NULL);
	return result;
}

xml_token* get_next_token(parser* pars)
{
	xml_token* result = NULL;
	if (false == is_at_end(pars))
	{
		pars->current_token = pars->current_token->next;
		pars->current_token_index++;
		result = pars->current_token;
	}
	return result;
}

xml_token* peek_next_token(parser* pars)
{
	xml_token* result = NULL;
	if (false == is_at_end(pars))
	{
		result = pars->current_token->next;
	}
	return result;
}

xml_token* skip_to_next_type_occurrence(parser* pars, xml_token_type type)
{
	xml_token* t = pars->current_token;
	while (t)
	{
		if (t->type == type)
		{
			break;
		}
		else
		{
			t = get_next_token(pars);
		}
	}

	if (t->type == type)
	{
		return t;
	}
	else
	{
		return NULL;
	}
}

void add_to_children(xml_node* parent_node, xml_node* new_node)
{
	new_node->parent = parent_node;
	parent_node->children_count++;

	if (parent_node->first_child)
	{
		xml_node* last_child = parent_node->first_child;
		while (last_child->next)
		{
			last_child = last_child->next;
		}
		last_child->next = new_node;
	}
	else
	{
		parent_node->first_child = new_node;
	}
}

void add_to_attributes(xml_node* node, xml_attribute* new_attribute)
{
	new_attribute->owner = node;
	node->attributes_count++;

	if (node->first_attribute)
	{
		xml_attribute* last_attribute = node->first_attribute;
		while (last_attribute->next)
		{
			last_attribute = last_attribute->next;
		}
		last_attribute->next = new_attribute;
		node->attributes_count++;
	}
	else
	{
		node->first_attribute = new_attribute;
	}
}

xml_node* parse_tokens(parser* pars)
{
	xml_token* t = pars->current_token;
	xml_node* current_node = NULL;

	while (t)
	{
		if (t->type == xml_token_type::LEFT_PROLOG_CHEVRON)
		{
			t = skip_to_next_type_occurrence(pars, xml_token_type::RIGHT_PROLOG_CHEVRON);
			continue;
		}

		if (t->type == xml_token_type::LEFT_CHEVRON)
		{
			t = get_next_token(pars);
			if (t && t->type == xml_token_type::TAG)
			{
				xml_node* new_node = push_struct(pars->arena, xml_node);
				new_node->tag = t->value;
				if (current_node)
				{
					new_node->parent = current_node;
					add_to_children(current_node, new_node);
				}
				else
				{
					// XML ma zawsze jeden root element
					current_node = new_node;
				}

				t = get_next_token(pars);
				while (t
					&& t->type != xml_token_type::RIGHT_CHEVRON
					&& t->type != xml_token_type::SELF_CLOSING_RIGHT_CHEVRON)
				{
					// teraz mamy pary atrybut wartość
					if (t->type == xml_token_type::ATTRIBUTE_NAME)
					{
						xml_attribute* new_attribute = push_struct(pars->arena, xml_attribute);
						new_attribute->name = t->value;

						t = get_next_token(pars);
						if (t && t->type == xml_token_type::ATTRIBUTE_VALUE)
						{
							new_attribute->value = t->value;
							add_to_attributes(new_node, new_attribute);
						}
						else
						{
							// błąd - nazwa bez wartości
							invalid_code_path;
						}
					}
					else
					{
						//błąd - oczekiwany attribute name
						invalid_code_path;
					}

					t = get_next_token(pars);
				}

				if (false == is_at_end(pars))
				{
					if (t->type != xml_token_type::SELF_CLOSING_RIGHT_CHEVRON)
					{
						current_node = new_node;
					}
				}
				else
				{
					// mamy błąd - doszliśmy do końca, a nie ma taga zamykającego
					invalid_code_path;
				}

				t = get_next_token(pars);
				continue;
			}
			else
			{
				// błąd - element bez taga
				invalid_code_path;
			}
		}

		if (t->type == xml_token_type::INNER_TEXT)
		{
			if (current_node)
			{
				current_node->inner_text = t->value;
				t = get_next_token(pars);
				continue;
			}
			else
			{
				// błąd
			}
		}

		if (t->type == xml_token_type::CLOSING_LEFT_CHEVRON) // czyli </tag>
		{
			// musimy iść w górę
			t = get_next_token(pars);
			if (t && t->type == xml_token_type::TAG)
			{
				string_ref tag = t->value;
				if (current_node
					&& current_node->tag == tag)
				{
					if (current_node->parent)
					{
						current_node = current_node->parent;
					}
					else
					{
						break;
						// skończyliśmy dokument
					}
				}
				else if (current_node
					&& current_node->parent
					&& current_node->parent->tag == tag)
				{
					// to wystąpi w takiej sytuacji: <tag><innytag/></tag>
					if (current_node->parent->parent)
					{
						current_node = current_node->parent->parent;
					}
					else
					{
						break;
						// skończyliśmy dokument
					}
				}
				else
				{
					invalid_code_path;
					// błąd - nie mamy pasującego otwierającego taga
				}

				t = get_next_token(pars);
				continue;
			}
		}

		t = get_next_token(pars);
	}

	return current_node;
}

xml_node* find_tag_in_children(xml_node* node, const char* tag)
{
	xml_node* result = NULL;
	if (node)
	{
		if (compare_c_string(node->tag, tag))
		{
			result = node;
		}
		else
		{
			if (node->first_child)
			{
				xml_node* child = node->first_child;
				while (child)
				{
					if (compare_c_string(child->tag, tag))
					{
						result = child;
						break;
					}
					else
					{
						child = child->next;
					}
				}
			}
		}
	}

	return result;
}

xml_node* find_tag_in_nested_children(xml_node* node, const char* tag)
{
	xml_node* result = NULL;
	if (node && node->first_child)
	{
		xml_node* child = node->first_child;
		while (child)
		{
			if (compare_c_string(child->tag, tag))
			{
				result = child;
				break;
			}
			else
			{
				xml_node* tag_in_child = find_tag_in_nested_children(child, tag);
				if (tag_in_child)
				{
					result = tag_in_child;
					break;
				}
				else
				{
					child = child->next;
				}
			}
		}
	}

	return result;
}

xml_node* find_tag_with_attribute_in_children(xml_node* node, const char* tag, const char* attribute_name)
{
	xml_node* result = NULL;

	xml_node* node_with_tag = NULL;
	if (node && node->first_child)
	{
		xml_node* child = node->first_child;
		while (child)
		{
			if (compare_c_string(child->tag, tag))
			{
				node_with_tag = child;
				break;
			}
			else
			{
				child = child->next;
			}
		}
	}

	if (node_with_tag)
	{
		xml_attribute* attribute = node_with_tag->first_attribute;
		while (attribute)
		{
			if (compare_c_string(attribute->name, attribute_name))
			{
				result = node_with_tag;
				break;
			}
			else
			{
				attribute = attribute->next;
			}
		}
	}

	return result;
}

string_ref get_attribute_value(xml_node* node, const char* attribute_name)
{
	string_ref result = {};
	if (node)
	{
		xml_attribute* attribute = node->first_attribute;
		while (attribute)
		{
			if (compare_c_string(attribute->name, attribute_name))
			{
				result = attribute->value;
				break;
			}
			else
			{
				attribute = attribute->next;
			}
		}
	}
	return result;
}

tilemap better_parse_tilemap(memory_arena* permanent_arena, read_file_result file)
{
	tilemap map = {};

	int memory_for_parsing_size = megabytes_to_bytes(10);
	void* memory_for_parsing = SDL_malloc(memory_for_parsing_size);

	memory_arena parsing_arena = {};
	initialize_memory_arena(&parsing_arena, memory_for_parsing_size, (byte*)memory_for_parsing);

	const char* string_test = "no i co teraz kurde balans";
	int string_test_count = SDL_strlen(string_test);

	string_ref my_string = copy_string_to_memory_arena(&parsing_arena, string_test, string_test_count);

	//my_string.string_size--;
	//*(my_string.ptr + my_string.string_size - 1) = 'x';
	b32 compare = compare_c_string(my_string, string_test);

	//xml_token* tk = push_struct(&parsing_arena, xml_token);

	scanner scan = {};
	scan.source = (char*)file.contents;
	scan.source_length = file.size;
	//scan.temp_buf;
	//scan.temp_buf_index;
	//scan.temp_buf_length;
	//scan.current_char;
	//scan.current_lexeme_start_char;
	scan.last_token = NULL;
	scan.token_count = 0;
	scan.arena = &parsing_arena;

	while (scan_token(&scan));
	if (scan.token_count > 0)
	{
		parser pars = {};
		pars.scan = scan;
		pars.current_token = scan.first_token;
		pars.current_token_index = 0;
		pars.arena = scan.arena;

		xml_node* root = parse_tokens(&pars);
		if (root)
		{		
			xml_node* object_test = find_tag_in_nested_children(root, "object");

			xml_node* map_node = find_tag_in_children(root, "map");
			if (map_node)
			{
				string_ref width = get_attribute_value(map_node, "width");
				string_ref height = get_attribute_value(map_node, "height");
							
				if (width.ptr && height.ptr)
				{
					i32 map_width = string_to_int(width);
					i32 map_height = string_to_int(height);

					xml_node* layer_node = find_tag_in_children(root, "layer");
					string_ref layer_width_str = get_attribute_value(layer_node, "width");
					string_ref layer_height_str = get_attribute_value(layer_node, "height");

					if (layer_width_str.ptr && layer_height_str.ptr)
					{
						i32 layer_width = string_to_int(width);
						i32 layer_height = string_to_int(height);
						if (layer_width == map_width && layer_height == map_height)
						{
							map.width = map_width;
							map.height = map_height;

							xml_node* data_node = find_tag_in_children(layer_node, "data");
							if (data_node)
							{
								string_ref data = data_node->inner_text;		

								string_ref encoding_str = get_attribute_value(data_node, "encoding");
								if (compare_c_string(encoding_str, "csv"))
								{
									map.tiles_count = map.width * map.height;
									// to powinno być już do permanent area
									map.tiles = parse_array_of_ints(permanent_arena, map.tiles_count, data, ',');

									debug_breakpoint;
								}
								else
								{
									// błąd
								}
							}
							else
							{
								// błąd
							}
						}
						else
						{
							// błąd
						}
					}
					else
					{
						// błąd
					}					
				}
				else
				{
					// błąd
				}
			}
			else
			{
				// błąd
			}
		}
	}

	SDL_free(memory_for_parsing);

	return map;
}