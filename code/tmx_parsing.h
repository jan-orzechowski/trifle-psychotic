#pragma once

#include "jorstring.h"

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

struct xml_node_search_result
{
	xml_node** found_nodes;
	u32 found_nodes_count;
};

xml_node* scan_and_parse_tmx(memory_arena* transient_arena, void* file_contents, u32 file_size);

xml_node* find_tag_in_children(xml_node* node, const char* tag);
xml_node* find_tag_in_nested_children(xml_node* node, const char* tag);
xml_node* find_tag_with_attribute_in_children(xml_node* node, const char* tag, const char* attr_name, const char* attr_value);
string_ref get_attribute_value(xml_node* node, const char* attribute_name);
xml_node_search_result* find_all_nodes_with_tag(memory_arena* arena, xml_node* root_node, const char* tag);