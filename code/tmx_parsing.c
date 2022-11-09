#include "tmx_parsing.h"

inline b32 is_scanner_at_end(xml_scanner* scan)
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

xml_token* add_token_with_value(xml_scanner* scan, xml_token_type type, string_ref value)
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
    if (false == is_scanner_at_end(scan))
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
    if (false == is_scanner_at_end(scan))
    {
        scan->current_char_index++;
    }
}

inline char peek(xml_scanner* scan)
{
    if (false == is_scanner_at_end(scan))
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

string_ref scan_text(xml_scanner* scan, b32 allow_whitespace)
{
    string_ref ref_to_source = {0};

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
                add_token(scan, LEFT_PROLOG_CHEVRON);
            }
            else if (next_c == '/')
            {
                advance(scan);
                add_token(scan, CLOSING_LEFT_CHEVRON);
            }
            else
            {
                add_token(scan, LEFT_CHEVRON);
            }

            omit_whitespace(scan);

            if (is_charater_allowed_for_text(peek(scan), false))
            {
                advance(scan);
                string_ref tag_name = scan_text(scan, false);
                add_token_with_value(scan, TAG, tag_name);
            }
        }
        break;
        case '?':
        {
            char next_c = peek(scan);
            if (next_c == '>')
            {
                advance(scan);
                add_token(scan, RIGHT_PROLOG_CHEVRON);

                omit_whitespace(scan);
                if (is_charater_allowed_for_text(peek(scan), true))
                {
                    advance(scan);
                    string_ref inner_text = scan_text(scan, true);
                    add_token_with_value(scan, INNER_TEXT, inner_text);
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
            add_token(scan, RIGHT_CHEVRON);
            advance(scan);

            if (is_charater_allowed_for_text(peek(scan), true))
            {
                string_ref inner_text = scan_text(scan, true);
                if (false == check_if_string_is_whitespace(inner_text))
                {
                    add_token_with_value(scan, INNER_TEXT, inner_text);
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
                add_token(scan, SELF_CLOSING_RIGHT_CHEVRON);
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
            while (peek(scan) != '"' && false == is_scanner_at_end(scan))
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
                string_ref attr_value = scan_text(scan, true);
                add_token_with_value(scan, ATTRIBUTE_VALUE, attr_value);
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
                add_token_with_value(scan, ATTRIBUTE_NAME, attr_value);
            }
        }
        break;
    }

    advance(scan);
    if (is_scanner_at_end(scan))
    {
        return false;
    }
    else
    {
        return true;
    }
}

b32 is_parser_at_end(xml_parser* pars)
{
    b32 result = (pars->current_token_index >= pars->scan.token_count
        || pars->current_token == NULL
        || pars->current_token->next == NULL);
    return result;
}

xml_token* get_next_token(xml_parser* pars)
{
    xml_token* result = NULL;
    if (false == is_parser_at_end(pars))
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
    if (false == is_parser_at_end(pars))
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
        if (t->type == LEFT_PROLOG_CHEVRON)
        {
            t = skip_to_next_token_type_occurrence(pars, RIGHT_PROLOG_CHEVRON);
            continue;
        }

        if (t->type == LEFT_CHEVRON)
        {
            t = get_next_token(pars);
            if (t && t->type == TAG)
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
                    && t->type != RIGHT_CHEVRON
                    && t->type != SELF_CLOSING_RIGHT_CHEVRON)
                {
                    // teraz mamy pary atrybut wartość
                    if (t->type == ATTRIBUTE_NAME)
                    {
                        xml_attribute* new_attribute = push_struct(pars->arena, xml_attribute);
                        new_attribute->name = t->value;

                        t = get_next_token(pars);
                        if (t && t->type == ATTRIBUTE_VALUE)
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

                if (false == is_parser_at_end(pars))
                {
                    if (t->type != SELF_CLOSING_RIGHT_CHEVRON)
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

        if (t->type == INNER_TEXT)
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

        if (t->type == CLOSING_LEFT_CHEVRON) // czyli </tag>
        {
            // musimy iść w górę
            t = get_next_token(pars);
            if (t && t->type == TAG)
            {
                string_ref tag = t->value;
                if (current_node
                    && equals_string_ref(current_node->tag, tag))
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
                    && equals_string_ref(current_node->parent->tag, tag))
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

xml_node* scan_and_parse_tmx(memory_arena* transient_arena, void* file_contents, u32 file_size)
{
    xml_node* result = NULL;

    xml_scanner scan = {0};
    scan.source = (char*)file_contents;
    scan.source_length = file_size;
    scan.last_token = NULL;
    scan.token_count = 0;
    scan.arena = transient_arena;

    while (scan_token(&scan));
    if (scan.token_count > 0)
    {
        xml_parser pars = {0};
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
    string_ref result = {0};
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