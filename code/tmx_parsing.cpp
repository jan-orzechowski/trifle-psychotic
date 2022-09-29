#include "tmx_parsing.h"

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

struct xml_scanner
{
	char* source;
	u32 source_length;
	memory_arena* arena;

	u32 current_char_index;

	xml_token* first_token;
	xml_token* last_token;
	u32 token_count;
};

struct xml_parser
{
	xml_scanner scan;
	u32 current_token_index;
	xml_token* current_token;
	memory_arena* arena;
	u32 nodes_count;
};

inline b32 is_at_end(xml_scanner* scan)
{
	b32 result = (scan->current_char_index >= scan->source_length);
	return result;
}

xml_token* add_token(xml_scanner* scan, xml_token_type type)
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

xml_token* add_token(xml_scanner* scan, xml_token_type type, string_ref value)
{
	xml_token* new_token = add_token(scan, type);
	if (value.ptr)
	{
		new_token->value = value;
	}
	return new_token;
}

inline char get_current_char(xml_scanner* scan)
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

inline void advance(xml_scanner* scan)
{
	if (false == is_at_end(scan))
	{
		scan->current_char_index++;
	}
}

inline char peek(xml_scanner* scan)
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

void omit_whitespace(xml_scanner* scan)
{
	while (is_whitespace(peek(scan)))
	{
		scan->current_char_index++;
	}
}

b32 is_charater_allowed_for_text(char c, b32 allow_whitespace)
{
	b32 result = (c != '\0'
		&& c != '"'
		&& c != '<'
		&& c != '='
		&& c != '>'
		&& (is_whitespace(c) == false || allow_whitespace));
	return result;
}

string_ref scan_text(xml_scanner* scan, b32 allow_whitespace = true)
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

bool scan_token(xml_scanner* scan)
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
	xml_node* next; // np. następne dziecko tego samego rodzica
	string_ref inner_text;

	xml_attribute* first_attribute;
	u32 attributes_count;

	xml_node* first_child;
	u32 children_count;
};

struct xml_attribute
{
	string_ref name;
	string_ref value;
	xml_attribute* next;
};

b32 is_at_end(xml_parser* pars)
{
	b32 result = (pars->current_token_index >= pars->scan.token_count
		|| pars->current_token == NULL
		|| pars->current_token->next == NULL);
	return result;
}

xml_token* get_next_token(xml_parser* pars)
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

xml_token* peek_next_token(xml_parser* pars)
{
	xml_token* result = NULL;
	if (false == is_at_end(pars))
	{
		result = pars->current_token->next;
	}
	return result;
}

xml_token* skip_to_next_token_type_occurrence(xml_parser* pars, xml_token_type type)
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

xml_node* parse_tokens(xml_parser* pars)
{
	xml_token* t = pars->current_token;
	xml_node* current_node = NULL;

	while (t)
	{
		if (t->type == xml_token_type::LEFT_PROLOG_CHEVRON)
		{
			t = skip_to_next_token_type_occurrence(pars, xml_token_type::RIGHT_PROLOG_CHEVRON);
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

xml_node* scan_and_parse_tmx(memory_arena* transient_arena, read_file_result file)
{
	xml_node* result = NULL;

	xml_scanner scan = {};
	scan.source = (char*)file.contents;
	scan.source_length = file.size;
	scan.last_token = NULL;
	scan.token_count = 0;
	scan.arena = transient_arena;

	while (scan_token(&scan));
	if (scan.token_count > 0)
	{
		xml_parser pars = {};
		pars.scan = scan;
		pars.current_token = scan.first_token;
		pars.current_token_index = 0;
		pars.arena = scan.arena;

		result = parse_tokens(&pars);
	}

	return result;
}

xml_node* find_tag_in_children(xml_node* node, const char* tag)
{
	xml_node* result = NULL;
	if (node)
	{
		if (compare_to_c_string(node->tag, tag))
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
					if (compare_to_c_string(child->tag, tag))
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
			if (compare_to_c_string(child->tag, tag))
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

xml_node* find_tag_with_attribute_in_children(xml_node* node, const char* tag, const char* attr_name, const char* attr_value)
{
	xml_node* result = NULL;

	xml_node* node_with_tag = NULL;
	if (node && node->first_child)
	{
		xml_node* child = node->first_child;
		while (child)
		{
			if (compare_to_c_string(child->tag, tag))
			{
				node_with_tag = child;
				xml_attribute* attribute = node_with_tag->first_attribute;
				while (attribute)
				{
					if (compare_to_c_string(attribute->name, attr_name)
						&& compare_to_c_string(attribute->value, attr_value))
					{
						result = node_with_tag;
						goto find_tag_with_attribute_in_children_end;
					}
					else
					{
						attribute = attribute->next;
					}
				}
			}
						
			child = child->next;			
		}
	}

	find_tag_with_attribute_in_children_end:
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
			if (compare_to_c_string(attribute->name, attribute_name))
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

struct xml_node_search_result
{
	xml_node** found_nodes;
	u32 found_nodes_count;
};

void push_next_found_node(memory_arena* arena, xml_node_search_result* search_results, xml_node* new_found_node)
{
	// zakładamy, ze dalsza część pamięci jest wolna, i możemy w ten sposób zrobić pseudo "dynamiczną" tablicę
	xml_node** new_pointer = push_struct(arena, xml_node*);
	*new_pointer = new_found_node;
	search_results->found_nodes_count++;
}

xml_node_search_result* find_all_nodes_with_tag(memory_arena* arena, xml_node* root_node, const char* tag)
{
	xml_node_search_result* result = push_struct(arena, xml_node_search_result);
	result->found_nodes_count = 0;
	result->found_nodes = NULL;
	if (root_node && root_node->first_child)
	{	
		xml_node* child = root_node->first_child;
		while (child)
		{
			if (compare_to_c_string(child->tag, tag))
			{
				xml_node** new_pointer = push_struct(arena, xml_node*);
				*new_pointer = child;
				result->found_nodes_count++;			
				if (result->found_nodes == NULL)
				{
					result->found_nodes = new_pointer;
				}
			}
			child = child->next;
		}
	}
	return result;
}

entity_to_spawn* add_read_entity(map* level, memory_arena* arena, entity_type_enum type, tile_position position, v4 gate_color = get_zero_v4())
{
	entity_to_spawn* new_entity = push_struct(arena, entity_to_spawn);
	new_entity->type = type;
	new_entity->position = position;
	new_entity->color = gate_color;

	level->entities_to_spawn_count++;
	if (level->entities_to_spawn == NULL)
	{
		level->entities_to_spawn = new_entity;
	}

	return new_entity;
}

#define ERROR_BUFFER_LENGTH 1000

void add_error(memory_arena* arena, tmx_parsing_error_report* errors, const char* error_message)
{
	errors->errors_count++;

	if (errors->last_error == NULL)
	{
		errors->first_error = push_struct(arena, tmx_parsing_error);
		errors->last_error = errors->first_error;
	}
	else
	{
		errors->last_error->next = push_struct(arena, tmx_parsing_error);
		errors->last_error = errors->last_error->next;
	}

	errors->last_error->message = copy_c_string_buffer_to_memory_arena(arena, error_message, ERROR_BUFFER_LENGTH);
}

void read_entity(memory_arena* permanent_arena, memory_arena* transient_arena, map* level, tmx_parsing_error_report* errors, xml_node* node, i32 entity_tileset_first_gid)
{
	char error_buffer[ERROR_BUFFER_LENGTH];

	string_ref gid_str = get_attribute_value(node, "gid");
	string_ref x_str = get_attribute_value(node, "x");
	string_ref y_str = get_attribute_value(node, "y");

	tile_position position = get_tile_position(-1, -1);
	if (x_str.string_size && y_str.string_size)
	{
		r32 x = parse_r32(x_str, '.');
		r32 y = parse_r32(y_str, '.');
		// przesunięcie dodane, ponieważ Tiled trakuje lewy dolny róg jako origin pola
		// znacznie bardziej intuicyjne jest w edytorze traktowanie tak środka
		i32 tile_x = (i32)((x / TILE_SIDE_IN_PIXELS) + 0.5f);
		i32 tile_y = (i32)((y / TILE_SIDE_IN_PIXELS) - 0.5f);
		position = get_tile_position(tile_x, tile_y);
	}
	else
	{

	}

	string_ref next_level_name = {};

	entity_type_enum type = entity_type_enum::UNKNOWN;
	if (gid_str.string_size)
	{
		i32 gid = parse_i32(gid_str);
		gid -= (entity_tileset_first_gid - 1);
		switch (gid)
		{
			case 66: type = entity_type_enum::ENEMY_SENTRY; break;
			case 131: type = entity_type_enum::ENEMY_CULTIST; break;
			case 48: type = entity_type_enum::ENEMY_ROBOT; break;
			case 1601: type = entity_type_enum::GATE; break;
			case 1606: type = entity_type_enum::SWITCH; break;
			case 961: type = entity_type_enum::POWER_UP_INVINCIBILITY; break;
			case 962: type = entity_type_enum::POWER_UP_HEALTH; break;
			case 963: type = entity_type_enum::POWER_UP_SPEED; break;
			case 964: type = entity_type_enum::POWER_UP_DAMAGE; break;
			case 965: type = entity_type_enum::POWER_UP_SPREAD; break;
			case 906: type = entity_type_enum::NEXT_LEVEL_TRANSITION; break;
			case 1: type = entity_type_enum::PLAYER; break;			
			case 1928: type = entity_type_enum::MESSAGE_DISPLAY; break;
		}
	}

	switch (type)
	{
		case entity_type_enum::GATE:
		case entity_type_enum::SWITCH:
		{
			v4 gate_color = get_zero_v4();
			xml_node* properties_parent_node = find_tag_in_children(node, "properties");
			if (properties_parent_node)
			{
				xml_node_search_result* properties = find_all_nodes_with_tag(
					transient_arena, properties_parent_node, "property");

				for (u32 property_index = 0;
					property_index < properties->found_nodes_count;
					property_index++)
				{
					xml_node* prop = properties->found_nodes[property_index];
					string_ref name = get_attribute_value(prop, "name");
					string_ref type = get_attribute_value(prop, "type");

					if (compare_to_c_string(name, "color")
						&& compare_to_c_string(type, "color"))
					{
						string_ref color_str = get_attribute_value(prop, "value");
						if (color_str.string_size)
						{
							v4 color = parse_color_from_hexadecimal(color_str);
							if (false == is_zero(color))
							{
								// Tiled zapisuje alpha jako pierwsze
								v4 swapped_color = get_v4(
									color.g,
									color.b,
									color.a,
									color.r
								);
								// alpha pomijamy
								swapped_color.a = 255;
								gate_color = swapped_color;
								break;
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

			if (false == is_zero(gate_color))
			{
				add_read_entity(level, permanent_arena, type, position, gate_color);
			}
		}
		break;
		case entity_type_enum::PLAYER:
		{
			if (level->starting_tile.x == -1 && level->starting_tile.y == -1)
			{
				level->starting_tile = position;				
			}
			else
			{
				snprintf(error_buffer, ERROR_BUFFER_LENGTH,
					"More than one starting point set. Starting point at (%d, %d) ignored",
					position.x, position.y);
				add_error(transient_arena, errors, error_buffer);
			}
		} 
		break;
		case entity_type_enum::NEXT_LEVEL_TRANSITION:
		{
			if (level->next_map.string_size > 0)
			{
				add_error(transient_arena, errors, "More than one starting points set. Point at () ignored");
				break;
			}

			xml_node* properties_parent_node = find_tag_in_children(node, "properties");
			if (properties_parent_node)
			{
				xml_node_search_result* properties = find_all_nodes_with_tag(
					transient_arena, properties_parent_node, "property");

				for (u32 property_index = 0;
					property_index < properties->found_nodes_count;
					property_index++)
				{
					xml_node* prop = properties->found_nodes[property_index];
					string_ref name = get_attribute_value(prop, "name");
					if (compare_to_c_string(name, "next_level"))
					{
						string_ref next_level_str = get_attribute_value(prop, "value");
						if (next_level_str.string_size)
						{
							if (ends_with(next_level_str, ".tmx"))
							{
								next_level_str.string_size -= 4;
							}
							next_level_name = next_level_str;					
							break;
						}
					}
				}
			}

			if (next_level_name.string_size)
			{
				add_read_entity(level, permanent_arena, type, position);
				level->next_map = next_level_name;
			}
		}
		break;
		case entity_type_enum::MESSAGE_DISPLAY:
		{
			xml_node* properties_parent_node = find_tag_in_children(node, "properties");
			if (properties_parent_node)
			{
				xml_node_search_result* properties = find_all_nodes_with_tag(
					transient_arena, properties_parent_node, "property");

				for (u32 property_index = 0;
					property_index < properties->found_nodes_count;
					property_index++)
				{
					xml_node* prop = properties->found_nodes[property_index];
					string_ref name = get_attribute_value(prop, "name");
					if (compare_to_c_string(name, "message"))
					{
						string_ref message_str = get_attribute_value(prop, "value");
						if (message_str.string_size)
						{
							if (message_str.string_size < 1000)
							{							
								entity_to_spawn* message_entity = add_read_entity(level, permanent_arena, type, position);									
								message_entity->message = message_str;
							}
							else
							{
								// osiągniety max string size
							}
						}
					}
				}
			}
		}
		break;
		default:
		{
			add_read_entity(level, permanent_arena, type, position);
		}
		break;
	}
}



tmx_map_parsing_result read_map_from_tmx_file(memory_arena* permanent_arena, memory_arena* transient_arena, read_file_result file, const char* layer_name, b32 clean_up_transient_arena)
{
	tmx_map_parsing_result result = {};
	map level = {};
	
	// dla późniejszego sprawdzenia, czy została pozycja startowa została ustawiona
	level.starting_tile = get_tile_position(-1, -1);

	temporary_memory memory_for_parsing = {};
	if (clean_up_transient_arena)
	{
		memory_for_parsing = begin_temporary_memory(transient_arena);
	} 
	
	char error_buffer[ERROR_BUFFER_LENGTH];
	tmx_parsing_error_report* errors = push_struct(transient_arena, tmx_parsing_error_report);

	xml_node* root = scan_and_parse_tmx(transient_arena, file);
	if (root)
	{		
		i32 tileset_first_gid = -1;
		i32 entity_first_gid = -1;

		xml_node_search_result* tilesets = find_all_nodes_with_tag(transient_arena, root, "tileset");
		for (u32 tileset_node_index = 0;
			tileset_node_index < tilesets->found_nodes_count;
			tileset_node_index++)
		{
			xml_node* tileset_node = tilesets->found_nodes[tileset_node_index];
			string_ref firstgid_attr = get_attribute_value(tileset_node, "firstgid");
			string_ref source_attr = get_attribute_value(tileset_node, "source");
				
			if (ends_with(source_attr, "map_tileset.tsx"))
			{
				if (firstgid_attr.string_size)
				{
					tileset_first_gid = parse_i32(firstgid_attr);
				}
			}

			if (ends_with(source_attr, "entities_tileset.tsx"))
			{
				if (firstgid_attr.string_size)
				{
					entity_first_gid = parse_i32(firstgid_attr);
				}
			}
		}

		if (tileset_first_gid == -1 || tileset_first_gid == 0)
		{
			add_error(transient_arena, errors, "Tileset 'map_tileset.tsx' not added");
		}

		if (entity_first_gid == -1 || entity_first_gid == 0)
		{
			add_error(transient_arena, errors, "Tileset 'entities_tileset.tsx' not added");
		}

		xml_node* map_node = find_tag_in_children(root, "map");
		if (map_node)
		{
			string_ref width = get_attribute_value(map_node, "width");
			string_ref height = get_attribute_value(map_node, "height");							
			if (width.ptr && height.ptr)
			{
				i32 map_width = parse_i32(width);
				i32 map_height = parse_i32(height);

				xml_node* layer_node = find_tag_with_attribute_in_children(root, "layer", "name", layer_name);

				string_ref layer_width_str = get_attribute_value(layer_node, "width");
				string_ref layer_height_str = get_attribute_value(layer_node, "height");

				if (layer_width_str.ptr && layer_height_str.ptr)
				{
					i32 layer_width = parse_i32(layer_width_str);
					i32 layer_height = parse_i32(layer_height_str);
					if (layer_width == map_width && layer_height == map_height)
					{
						level.width = map_width;
						level.height = map_height;

						xml_node* data_node = find_tag_in_children(layer_node, "data");
						if (data_node)
						{
							string_ref data = data_node->inner_text;		

							string_ref encoding_str = get_attribute_value(data_node, "encoding");
							if (compare_to_c_string(encoding_str, "csv"))
							{
								level.tiles_count = level.width * level.height;
								level.tiles = parse_array_of_i32(permanent_arena, level.tiles_count, data, ',');

								if (tileset_first_gid != -1 
									&& tileset_first_gid != 0 
									&& tileset_first_gid != 1)
								{
									for (u32 tile_index = 0; tile_index < level.tiles_count; tile_index++)
									{
										i32 original_gid = level.tiles[tile_index];
										if (original_gid < tileset_first_gid)
										{
											// błąd											
										}

										level.tiles[tile_index] -= (tileset_first_gid - 1);
									}
								}
							}
							else
							{
								add_error(transient_arena, errors, "File format is not set to 'csv'");
								goto end_of_read_map_from_tmx_file_function;
							}
						}
						else
						{
							add_error(transient_arena, errors, "The 'data' element is missing");
							goto end_of_read_map_from_tmx_file_function;
						}
					}
					else
					{
						snprintf(error_buffer, ERROR_BUFFER_LENGTH,
							"Layer size (%d, %d) doesn't match map size (%d, %d)",
							layer_width, layer_height, map_width, map_height);
						add_error(transient_arena, errors, error_buffer);
						goto end_of_read_map_from_tmx_file_function;
					}
				}
				else
				{
					add_error(transient_arena, errors, "Layer doesn't have defined width or height");
					goto end_of_read_map_from_tmx_file_function;
				}					
			}
			else
			{
				add_error(transient_arena, errors, "Map doesn't have defined width or height");
				goto end_of_read_map_from_tmx_file_function;
			}
		}
		else
		{
			add_error(transient_arena, errors, "The 'map' element is missing");
			goto end_of_read_map_from_tmx_file_function;
		}

		xml_node* objectgroup_node = find_tag_in_children(root, "objectgroup");
		if (objectgroup_node)
		{
			xml_node_search_result* objects = find_all_nodes_with_tag(transient_arena, objectgroup_node, "object");
			if (objects->found_nodes_count > 0)
			{
				level.entities_to_spawn_count = 0;
				level.entities_to_spawn = NULL;
				for (u32 xml_node_index = 0; xml_node_index < objects->found_nodes_count; xml_node_index++)
				{
					xml_node* node = *(objects->found_nodes + xml_node_index);
					if (node)
					{
						read_entity(permanent_arena, transient_arena, &level, errors, node, entity_first_gid);
					}
				}
			}

			// tutaj błęd entities
			if (level.starting_tile.x == -1 || level.starting_tile.y == -1)
			{
				add_error(transient_arena, errors, "Starting position not set");
			}
		} 
		else
		{
			add_error(transient_arena, errors, "The 'objectgroup' element is missing");
		}

		if (level.next_map.string_size > 0)
		{
			// kopiujemy do permanent arena na samym końcu, ponieważ lista nowych entities jest "dynamiczna"
			level.next_map = copy_string(permanent_arena, level.next_map);
		}

		for (u32 entity_index = 0;
			entity_index < level.entities_to_spawn_count;
			entity_index++)
		{
			entity_to_spawn* entity_to_spawn = level.entities_to_spawn + entity_index;
			if (entity_to_spawn->type == entity_type_enum::MESSAGE_DISPLAY)
			{
				// tak samo jak z nazwą następnego poziomu
				entity_to_spawn->message = copy_string(permanent_arena, entity_to_spawn->message);
				printf(entity_to_spawn->message.ptr);
			}
		}
	}
	else
	{
		add_error(transient_arena, errors, "File is not a proper TMX format file");
	}

end_of_read_map_from_tmx_file_function:

	if (clean_up_transient_arena)
	{
		end_temporary_memory(memory_for_parsing, true);
	}

	result.parsed_map = level;
	result.errors = errors;

	return result;
}

map read_collision_map(memory_arena* permanent_arena, memory_arena* transient_arena, read_file_result file)
{
	map result = {};

	temporary_memory memory_for_parsing = begin_temporary_memory(transient_arena);

	xml_node* root = scan_and_parse_tmx(transient_arena, file);
	if (root)
	{
		i32 collision_tileset_first_gid = -1;
		xml_node_search_result* tilesets = find_all_nodes_with_tag(transient_arena, root, "tileset");
		for (u32 tileset_node_index = 0;
			tileset_node_index < tilesets->found_nodes_count;
			tileset_node_index++)
		{
			xml_node* tileset_node = tilesets->found_nodes[tileset_node_index];
			string_ref firstgid_attr = get_attribute_value(tileset_node, "firstgid");
			string_ref source_attr = get_attribute_value(tileset_node, "source");

			if (ends_with(source_attr, "collision_tileset.tsx"))
			{
				if (firstgid_attr.string_size)
				{
					collision_tileset_first_gid = parse_i32(firstgid_attr);
				}
			}
		}

		xml_node* layer_node = find_tag_with_attribute_in_children(root, "layer", "name", "collision");
		if (layer_node)
		{
			string_ref layer_width_str = get_attribute_value(layer_node, "width");
			string_ref layer_height_str = get_attribute_value(layer_node, "height");
			if (layer_width_str.ptr && layer_height_str.ptr)
			{
				i32 layer_width = parse_i32(layer_width_str);
				i32 layer_height = parse_i32(layer_height_str);
				result.width = layer_width;
				result.height = layer_height;
			}

			xml_node* data_node = find_tag_in_children(layer_node, "data");
			if (data_node)
			{	
				string_ref encoding_str = get_attribute_value(data_node, "encoding");
				if (compare_to_c_string(encoding_str, "csv"))
				{
					string_ref data = data_node->inner_text;
					result.tiles_count = result.width * result.height;
					result.tiles = parse_array_of_i32(permanent_arena, result.tiles_count, data, ',');

					if (collision_tileset_first_gid != -1 && collision_tileset_first_gid != 1)
					{
						for (u32 tile_index = 0; tile_index < result.tiles_count; tile_index++)
						{
							i32 original_gid = result.tiles[tile_index];
							result.tiles[tile_index] -= (collision_tileset_first_gid - 1);
						}
					}
				}
			}
		}
	}

	end_temporary_memory(memory_for_parsing, true);

	return result;
}
