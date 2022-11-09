#include "main.h"
#include "map.h"
#include "tmx_parsing.h"
#include "level_parsing.h"
#include "text_rendering.h"

typedef struct tmx_errors_buffer
{
    char* message_buffer;
    u32 message_buffer_size;
    tmx_parsing_error_report* report;
} tmx_errors_buffer;

typedef struct level_parsing_context
{
    memory_arena* permanent_arena;
    memory_arena* transient_arena;
    tmx_errors_buffer* errors;
    map* level;
    i32 tileset_first_gid;
    i32 entity_tileset_first_gid;
    i32 entity_tileset_last_gid;
} level_parsing_context;

entity_to_spawn* add_entity_to_spawn(map* level, memory_arena* arena,
    entity_type_enum type, tile_position position, v4 gate_color)
{
    entity_to_spawn* new_entity = push_struct(arena, entity_to_spawn);
    new_entity->type = type;
    new_entity->position = position;
    new_entity->color = gate_color;

    if (level->first_entity_to_spawn == NULL)
    {
        level->first_entity_to_spawn = new_entity;
        level->last_entity_to_spawn = new_entity;
    }
    else
    {
        level->last_entity_to_spawn->next = new_entity;
        level->last_entity_to_spawn = level->last_entity_to_spawn->next;
    }

    return new_entity;
}

void add_error_to_report(memory_arena* arena, tmx_errors_buffer* errors, const char* error_message)
{
    errors->report->errors_count++;

    if (errors->report->last_error == NULL)
    {
        errors->report->first_error = push_struct(arena, tmx_parsing_error);
        errors->report->last_error = errors->report->first_error;
    }
    else
    {
        errors->report->last_error->next = push_struct(arena, tmx_parsing_error);
        errors->report->last_error = errors->report->last_error->next;
    }

    errors->report->last_error->message = copy_c_string_buffer(arena, error_message, errors->message_buffer_size);
}

void add_error(level_parsing_context* parsing, const char* message)
{
    add_error_to_report(parsing->transient_arena, parsing->errors, message);
}

void add_error_from_buffer(level_parsing_context* parsing)
{
    add_error_to_report(parsing->transient_arena, parsing->errors, parsing->errors->message_buffer);
}

entity_type_enum get_entity_type_enum_from_gid(i32 gid, i32 entity_tileset_first_gid)
{
    entity_type_enum result = ENTITY_TYPE_UNKNOWN;

    gid -= (entity_tileset_first_gid - 1);
    switch (gid)
    {
        case 6:  result = ENTITY_TYPE_ENEMY_SENTRY; break;
        case 7:  result = ENTITY_TYPE_ENEMY_GUARDIAN; break;
        case 8:  result = ENTITY_TYPE_ENEMY_FLYING_BOMB; break;
        case 9:  result = ENTITY_TYPE_ENEMY_ROBOT; break;
        case 10: result = ENTITY_TYPE_ENEMY_MESSENGER; break;
        case 20: result = ENTITY_TYPE_ENEMY_BIG_SENTRY; break;
        case 11: result = ENTITY_TYPE_POWER_UP_HEALTH; break;
        case 12: result = ENTITY_TYPE_POWER_UP_SPEED; break;
        case 13: result = ENTITY_TYPE_POWER_UP_SPREAD; break;
        case 14: result = ENTITY_TYPE_POWER_UP_DAMAGE; break;
        case 15: result = ENTITY_TYPE_POWER_UP_INVINCIBILITY; break;
        case 16: result = ENTITY_TYPE_GATE_SILVER; break;
        case 17: result = ENTITY_TYPE_GATE_GOLD; break;
        case 18: result = ENTITY_TYPE_GATE_RED; break;
        case 19: result = ENTITY_TYPE_GATE_GREEN; break;
        case 21: result = ENTITY_TYPE_SWITCH_SILVER; break;
        case 22: result = ENTITY_TYPE_SWITCH_GOLD; break;
        case 23: result = ENTITY_TYPE_SWITCH_RED; break;
        case 24: result = ENTITY_TYPE_SWITCH_GREEN; break;
        case 26: result = ENTITY_TYPE_MOVING_PLATFORM_HORIZONTAL_SILVER; break;
        case 27: result = ENTITY_TYPE_MOVING_PLATFORM_HORIZONTAL_GOLD; break;
        case 28: result = ENTITY_TYPE_MOVING_PLATFORM_HORIZONTAL_RED; break;
        case 29: result = ENTITY_TYPE_MOVING_PLATFORM_HORIZONTAL_GREEN; break;
        case 31: result = ENTITY_TYPE_MOVING_PLATFORM_VERTICAL_SILVER; break;
        case 32: result = ENTITY_TYPE_MOVING_PLATFORM_VERTICAL_GOLD; break;
        case 33: result = ENTITY_TYPE_MOVING_PLATFORM_VERTICAL_RED; break;
        case 34: result = ENTITY_TYPE_MOVING_PLATFORM_VERTICAL_GREEN; break;
        case 2:  result = ENTITY_TYPE_PLAYER; break;
        case 3:  result = ENTITY_TYPE_NEXT_LEVEL_TRANSITION; break;
        case 4:  result = ENTITY_TYPE_MESSAGE_DISPLAY; break;
    }

    return result;
}

void parse_entity_from_tile(level_parsing_context* parsing, u32 tile_index, i32 gid)
{
    tile_position entity_position = get_tile_pos_from_index(parsing->level, tile_index);

    entity_type_enum type = get_entity_type_enum_from_gid(gid, parsing->entity_tileset_first_gid);
    switch (type)
    {
        case ENTITY_TYPE_GATE_SILVER:
        case ENTITY_TYPE_GATE_GOLD:
        case ENTITY_TYPE_GATE_RED:
        case ENTITY_TYPE_GATE_GREEN:
        case ENTITY_TYPE_SWITCH_SILVER:
        case ENTITY_TYPE_SWITCH_GOLD:
        case ENTITY_TYPE_SWITCH_RED:
        case ENTITY_TYPE_SWITCH_GREEN:
        case ENTITY_TYPE_NEXT_LEVEL_TRANSITION:
        case ENTITY_TYPE_MESSAGE_DISPLAY:
        {
            snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                "Entity added as a tile at (%d, %d) is a switch, gate, next level transition or a message. Instead, it needs to be added in the entity layer with valid properties set.",
                entity_position.x, entity_position.y);
            add_error_from_buffer(parsing);
        }
        break;
        case ENTITY_TYPE_PLAYER:
        {
            if (parsing->level->starting_tile.x == -1
                && parsing->level->starting_tile.y == -1)
            {
                parsing->level->starting_tile = entity_position;
            }
            else
            {
                snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                    "More than one starting point set. The second starting point was added as a tile at (%d, %d).",
                    entity_position.x, entity_position.y);
                add_error_from_buffer(parsing);
            }
        }
        break;
        case ENTITY_TYPE_UNKNOWN:
        {
            // pomijamy
            debug_breakpoint;
        }
        break;
        default:
        {
            add_entity_to_spawn(parsing->level, parsing->transient_arena, type, entity_position, get_zero_v4());
        }
        break;
    }
}

void parse_entity(level_parsing_context* parsing, xml_node* node)
{
    string_ref gid_str = get_attribute_value(node, "gid");
    string_ref x_str = get_attribute_value(node, "x");
    string_ref y_str = get_attribute_value(node, "y");

    tile_position position = get_tile_pos(-1, -1);
    if (x_str.string_size && y_str.string_size)
    {
        r32 x = parse_r32(x_str, '.');
        r32 y = parse_r32(y_str, '.');

        // przesunięcie dodane, ponieważ Tiled trakuje lewy dolny róg jako origin pola
        // znacznie bardziej intuicyjne jest w edytorze traktowanie tak środka
        i32 tile_x = (i32)((x / TILE_SIDE_IN_PIXELS) + 0.5f);
        i32 tile_y = (i32)((y / TILE_SIDE_IN_PIXELS) - 0.5f);
        position = get_tile_pos(tile_x, tile_y);
    }
    else
    {
        add_error(parsing, "There is an entity with no position set.");
    }

    i32 gid = -1;
    if (gid_str.string_size)
    {
        gid = parse_i32(gid_str);
    }
    else
    {
        snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
            "There is an entity with no ID at position (%d, %d).", position.x, position.y);
        add_error_from_buffer(parsing);
    }

    entity_type_enum type = get_entity_type_enum_from_gid(gid, parsing->entity_tileset_first_gid);
    switch (type)
    {
        case ENTITY_TYPE_GATE_SILVER:
        case ENTITY_TYPE_GATE_GOLD:
        case ENTITY_TYPE_GATE_RED:
        case ENTITY_TYPE_GATE_GREEN:
        case ENTITY_TYPE_SWITCH_SILVER:
        case ENTITY_TYPE_SWITCH_GOLD:
        case ENTITY_TYPE_SWITCH_RED:
        case ENTITY_TYPE_SWITCH_GREEN:
        {
            v4 gate_color = get_zero_v4();
            xml_node* properties_parent_node = find_tag_in_children(node, "properties");
            if (properties_parent_node)
            {
                xml_node_search_result* properties = find_all_nodes_with_tag(
                    parsing->transient_arena, properties_parent_node, "property");

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
                            if (false == is_zero_v4(color))
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
                                snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                                    "Gate/switch at position (%d, %d) has color property with zero value.", position.x, position.y);
                                add_error_from_buffer(parsing);
                            }
                        }
                        else
                        {
                            snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                                "Gate/switch at position (%d, %d) has color property with no value.", position.x, position.y);
                            add_error_from_buffer(parsing);
                        }
                    }
                    else
                    {
                        snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                            "Gate/switch at position (%d, %d) has no 'color' property.", position.x, position.y);
                        add_error_from_buffer(parsing);
                    }
                }
            }

            if (false == is_zero_v4(gate_color))
            {
                add_entity_to_spawn(parsing->level, parsing->transient_arena, type, position, gate_color);
            }
        }
        break;
        case ENTITY_TYPE_PLAYER:
        {
            if (parsing->level->starting_tile.x == -1
                && parsing->level->starting_tile.y == -1)
            {
                parsing->level->starting_tile = position;
            }
            else
            {
                snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                    "More than one starting point set. The second starting point was added at (%d, %d).",
                    position.x, position.y);
                add_error_from_buffer(parsing);
            }
        }
        break;
        case ENTITY_TYPE_NEXT_LEVEL_TRANSITION:
        {
            if (parsing->level->next_map.string_size > 0)
            {
                snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                    "More than one next level transition entity set. The second transition entity was added at (%d, %d).",
                    position.x, position.y);
                add_error_from_buffer(parsing);
                break;
            }

            string_ref next_level_name = {0};

            xml_node* properties_parent_node = find_tag_in_children(node, "properties");
            if (properties_parent_node)
            {
                xml_node_search_result* properties = find_all_nodes_with_tag(
                    parsing->transient_arena, properties_parent_node, "property");

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

                            if (next_level_str.string_size < MAX_LEVEL_NAME_LENGTH)
                            {
                                next_level_name = next_level_str;
                                break;
                            }
                            else
                            {
                                snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                                    "Next level name set in transition entity at (%d, %d) is longer than %d characters.",
                                    position.x, position.y, MAX_LEVEL_NAME_LENGTH);
                                add_error_from_buffer(parsing);
                            }							
                        }
                        else
                        {
                            snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                                "Transition entity at (%d, %d) has no 'next_level' property set.", position.x, position.y);
                            add_error_from_buffer(parsing);
                        }
                    }
                }
            }

            if (next_level_name.string_size)
            {
                add_entity_to_spawn(parsing->level, parsing->transient_arena, type, position, get_zero_v4());
                parsing->level->next_map = copy_string(parsing->permanent_arena, next_level_name);
            }
        }
        break;
        case ENTITY_TYPE_MESSAGE_DISPLAY:
        {
            xml_node* properties_parent_node = find_tag_in_children(node, "properties");
            if (properties_parent_node)
            {
                xml_node_search_result* properties = find_all_nodes_with_tag(
                    parsing->transient_arena, properties_parent_node, "property");

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
                            if (message_str.string_size <= 1000)
                            {
                                entity_to_spawn* message_entity = 
                                    add_entity_to_spawn(parsing->level, parsing->transient_arena, type, position, get_zero_v4());
                                message_entity->message = copy_string(parsing->permanent_arena, message_str);
                            }
                            else
                            {
                                snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                                    "The message display entity at position (%d, %d) has message longer than 1000 characters.",
                                    position.x, position.y);
                                add_error_from_buffer(parsing);
                            }
                        }
                        else
                        {
                            snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                                "There is a message display entity with no 'message' property set at position (%d, %d).",
                                position.x, position.y);
                            add_error_from_buffer(parsing);
                        }
                    }
                }
            }
        }
        break;
        case  ENTITY_TYPE_UNKNOWN:
        {
            // pomijamy
        }
        break;
        default:
        {
            add_entity_to_spawn(parsing->level, parsing->transient_arena, type, position, get_zero_v4());
        }
        break;
    }
}

void parse_backdrop_texture_property(backdrop_properties* backdrop, string_ref name)
{
    backdrop->texture = TEXTURE_NONE;

    if (compare_to_c_string(name, "desert"))
    {
        backdrop->texture = TEXTURE_BACKGROUND_DESERT;
        backdrop->size = get_v2(320, 420);
    }
    else if (compare_to_c_string(name, "ice_desert"))
    {
        backdrop->texture = TEXTURE_BACKGROUND_ICE_DESERT;
        backdrop->size = get_v2(320, 420);
    }
    else if (compare_to_c_string(name, "clouds"))
    {
        backdrop->texture = TEXTURE_BACKGROUND_CLOUDS;
        backdrop->size = get_v2(2048, 400);
    }
    else if (compare_to_c_string(name, "red_planet_desert"))
    {
        backdrop->texture = TEXTURE_BACKGROUND_RED_PLANET_DESERT;
        backdrop->size = get_v2(410, 480);
    }
    else if (compare_to_c_string(name, "red_planet_sky"))
    {
        backdrop->texture = TEXTURE_BACKGROUND_RED_PLANET_SKY;
        backdrop->size = get_v2(410, 600);
    }
    else if (compare_to_c_string(name, "planet_orbit"))
    {
        backdrop->texture = TEXTURE_BACKGROUND_PLANET_ORBIT;
        backdrop->size = get_v2(384, 320);
    }
    else
    {
        debug_breakpoint;
    }
}

void parse_map_properties(level_parsing_context* parsing, xml_node* map_node)
{
    xml_node* properties_node = find_tag_in_children(map_node, "properties");
    if (properties_node)
    {
        xml_node_search_result* map_properties = find_all_nodes_with_tag(
            parsing->transient_arena, properties_node, "property");
        for (u32 property_index = 0;
            property_index < map_properties->found_nodes_count;
            property_index++)
        {
            xml_node* property_node = map_properties->found_nodes[property_index];
            string_ref name = get_attribute_value(property_node, "name");
            string_ref type = get_attribute_value(property_node, "type");
            string_ref value = get_attribute_value(property_node, "value");

            if (name.string_size == 0)
            {
                add_error(parsing, "TMX contains map property without name.");
                continue;
            }

            if (value.string_size == 0)
            {	
                value = property_node->inner_text;
            }

            if (value.string_size == 0)
            {				
                snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                    "Property '%s' has no value set.", get_c_string(parsing->transient_arena, name));
                add_error_from_buffer(parsing);
                continue;
            }

            if (compare_to_c_string(name, "backdrop"))
            {
                parse_backdrop_texture_property(&parsing->level->first_backdrop, value);
            }
            else if (compare_to_c_string(name, "backdrop_slowdown_x"))
            {
                if (compare_to_c_string(type, "int"))
                {
                    parsing->level->first_backdrop.x_slowdown = parse_i32(value);
                }
                else
                {
                    snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                        "Property '%s' should have type '%s' set, has '%s' instead.",
                        "backdrop_slowdown_x", "int", get_c_string(parsing->transient_arena, type));
                    add_error_from_buffer(parsing);
                }
            }
            else if (compare_to_c_string(name, "backdrop_slowdown_y"))
            {
                if (compare_to_c_string(type, "int"))
                {
                    parsing->level->first_backdrop.y_slowdown = parse_i32(value);
                }
                else
                {
                    snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                        "Property '%s' should have type '%s' set, has '%s' instead.",
                        "backdrop_slowdown_y", "int", get_c_string(parsing->transient_arena, type));
                    add_error_from_buffer(parsing);
                }
            }
            else if (compare_to_c_string(name, "backdrop_speed_x"))
            {
                if (compare_to_c_string(type, "float"))
                {
                    parsing->level->first_backdrop.x_speed = parse_r32(value, '.');
                }
                else
                {
                    snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                        "Property '%s' should have type '%s' set, has '%s' instead.",
                        "backdrop_speed_x", "float", get_c_string(parsing->transient_arena, type));
                    add_error_from_buffer(parsing);
                }
            }
            else if (compare_to_c_string(name, "backdrop_speed_y"))
            {
                if (compare_to_c_string(type, "float"))
                {
                    parsing->level->first_backdrop.y_speed = parse_r32(value, '.');
                }
                else
                {
                    snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                        "Property '%s' should have type '%s' set, has '%s' instead.",
                        "backdrop_speed_y", "float", get_c_string(parsing->transient_arena, type));
                    add_error_from_buffer(parsing);
                }
            }
            else if (compare_to_c_string(name, "second_backdrop"))
            {
                parse_backdrop_texture_property(&parsing->level->second_backdrop, value);
            }
            else if (compare_to_c_string(name, "second_backdrop_slowdown_x"))
            {
                if (compare_to_c_string(type, "int"))
                {
                    parsing->level->second_backdrop.x_slowdown = parse_i32(value);
                }
                else
                {
                    snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                        "Property '%s' should have type '%s' set, has '%s' instead.",
                        "second_backdrop_slowdown_x", "int", get_c_string(parsing->transient_arena, type));
                    add_error_from_buffer(parsing);
                }
            }
            else if (compare_to_c_string(name, "second_backdrop_slowdown_y"))
            {
                if (compare_to_c_string(type, "int"))
                {
                    parsing->level->second_backdrop.y_slowdown = parse_i32(value);
                }
                else
                {
                    snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                        "Property '%s' should have type '%s' set, has '%s' instead.",
                        "second_backdrop_slowdown_y", "int", get_c_string(parsing->transient_arena, type));
                    add_error_from_buffer(parsing);
                }
            }
            else if (compare_to_c_string(name, "initial_health"))
            {
                if (compare_to_c_string(type, "int"))
                {
                    parsing->level->initial_max_player_health = (r32)parse_i32(value);
                }
                else
                {
                    snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                        "Property '%s' should have type '%s' set, has '%s' instead.",
                        "initial_health", "int", get_c_string(parsing->transient_arena, type));
                    add_error_from_buffer(parsing);
                }
            }
            else if (compare_to_c_string(name, "introduction"))
            {
                if (type.string_size == 0)
                {
                    parsing->level->introduction = copy_string(parsing->permanent_arena, value);
                }
                else
                {
                    snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                        "Property '%s' should have type '%s' set, has '%s' instead.",
                        "introduction", "string", get_c_string(parsing->transient_arena, type));
                    add_error_from_buffer(parsing);
                }
            }
            else if (compare_to_c_string(name, "win_when_all_messengers_killed"))
            {
                if (compare_to_c_string(type, "bool"))
                {
                    if (compare_to_c_string(value, "true"))
                    {
                        parsing->level->complete_when_all_messengers_killed = true;
                    }					
                }
                else
                {
                    snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                        "Property '%s' should have type '%s' set, has '%s' instead.",
                        "win_when_all_messengers_killed", "bool", get_c_string(parsing->transient_arena, type));
                    add_error_from_buffer(parsing);
                }
            }
            else if (compare_to_c_string(name, "music"))
            {
                if (type.string_size == 0)
                {	
                    parsing->level->music_file_name = copy_string(parsing->permanent_arena, value);
                }
                else
                {
                    snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                        "Property '%s' should have type '%s' set, has '%s' instead.",
                        "music", "string", get_c_string(parsing->transient_arena, type));
                    add_error_from_buffer(parsing);
                }
            }
            else
            {
                snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                    "Unknown map property: '%s'", get_c_string(parsing->transient_arena, name));
                add_error_to_report(parsing->transient_arena, parsing->errors, parsing->errors->message_buffer);
            }
        }
    }
}

map_layer parse_map_layer(level_parsing_context* parsing,
    xml_node* root_node, const char* layer_name, b32 is_layer_required)
{
    map_layer layer = {0};
    xml_node* layer_node = find_tag_with_attribute_in_children(root_node, "layer", "name", layer_name);
    if (layer_node)
    {
        string_ref layer_width_str = get_attribute_value(layer_node, "width");
        string_ref layer_height_str = get_attribute_value(layer_node, "height");
        if (layer_width_str.ptr && layer_height_str.ptr)
        {
            u32 layer_width = (u32)parse_i32(layer_width_str);
            u32 layer_height = (u32)parse_i32(layer_height_str);
            if (layer_width == parsing->level->width
                && layer_height == parsing->level->height)
            {			
                layer.width = layer_width;
                layer.height = layer_height;

                xml_node* data_node = find_tag_in_children(layer_node, "data");
                if (data_node)
                {
                    string_ref data = data_node->inner_text;
                    string_ref encoding_str = get_attribute_value(data_node, "encoding");
                    if (compare_to_c_string(encoding_str, "csv"))
                    {
                        layer.tiles_count = layer_width * layer_height;
                        layer.tiles = parse_array_of_i32(parsing->permanent_arena, layer.tiles_count, data, ',');

                        for (u32 tile_index = 0; tile_index < layer.tiles_count; tile_index++)
                        {
                            i32 gid = layer.tiles[tile_index];
                            if (gid >= parsing->entity_tileset_first_gid
                                && gid <= parsing->entity_tileset_last_gid)
                            {
                                parse_entity_from_tile(parsing, tile_index, gid);
                                // dodajemy puste pole w miejscu, gdzie zostało zdefiniowane entity
                                layer.tiles[tile_index] = 1;
                            }
                            else
                            {
                                layer.tiles[tile_index] -= (parsing->tileset_first_gid - 1);
                            }
                        }						
                    }
                    else
                    {
                        snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                            "Format of the layer '%s' is not set to 'csv'.", layer_name);
                        add_error_from_buffer(parsing);
                    }
                }
                else
                {
                    snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                        "The 'data' element is missing in the layer '%s'.", layer_name);
                    add_error_from_buffer(parsing);
                }
            }
            else
            {
                snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                    "Size of the layer '%s' (%d, %d) doesn't match the map size (%d, %d).",
                    layer_name, layer_width, layer_height, parsing->level->width, parsing->level->height);
                add_error_from_buffer(parsing);
            }
        }
        else
        {
            snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                "Layer '%s' doesn't have defined width or height.", layer_name);
            add_error_from_buffer(parsing);
        }
    }
    else
    {
        if (is_layer_required)
        {
            snprintf(parsing->errors->message_buffer, parsing->errors->message_buffer_size,
                "Layer '%s' not found.", layer_name);
            add_error_from_buffer(parsing);
        }
    }

    return layer;
}

tmx_map_parsing_result read_map_from_tmx_file(memory_arena* permanent_arena, memory_arena* transient_arena, 
    read_file_result file, string_ref map_name, b32 clean_up_transient_arena)
{
    tmx_map_parsing_result result = {0};
    map level = {0};

    // dla późniejszego sprawdzenia, czy pozycja startowa została ustawiona
    level.starting_tile = get_tile_pos(-1, -1);

    temporary_memory memory_for_parsing = {0};
    if (clean_up_transient_arena)
    {
        memory_for_parsing = begin_temporary_memory(transient_arena);
    }

    tmx_errors_buffer errors = {0};
    errors.message_buffer_size = 1000;
    errors.message_buffer = push_array(transient_arena, errors.message_buffer_size, char);
    errors.report = push_struct(transient_arena, tmx_parsing_error_report);

    xml_node* root = scan_and_parse_tmx(transient_arena, file.contents, file.size);
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
            add_error_to_report(transient_arena, &errors, "Tileset 'map_tileset.tsx' not added.");
            goto end_of_read_map_from_tmx_file_function;
        }

        if (entity_first_gid == -1 || entity_first_gid == 0)
        {
            add_error_to_report(transient_arena, &errors, "Tileset 'entities_tileset.tsx' not added.");
            goto end_of_read_map_from_tmx_file_function;
        }

        level_parsing_context parsing = {0};
        parsing.tileset_first_gid = tileset_first_gid;
        parsing.entity_tileset_first_gid = entity_first_gid;
        parsing.entity_tileset_last_gid = entity_first_gid + 35;
        parsing.errors = &errors;
        parsing.permanent_arena = permanent_arena;
        parsing.transient_arena = transient_arena;
        parsing.level = &level;

        xml_node* map_node = find_tag_in_children(root, "map");
        if (map_node)
        {
            string_ref renderorder = get_attribute_value(map_node, "renderorder");
            if (renderorder.string_size == 0
                || false == compare_to_c_string(renderorder, "right-down"))
            {
                add_error_to_report(transient_arena, &errors, "Map property 'Tile Render Order' should be set to 'Right Down'.");
            }

            string_ref orientation = get_attribute_value(map_node, "orientation");
            if (orientation.string_size == 0
                || false == compare_to_c_string(orientation, "orthogonal"))
            {
                add_error_to_report(transient_arena, &errors, "Map property 'Orientation' should be set to 'Orthogonal'.");
            }

            string_ref tilewidth = get_attribute_value(map_node, "tilewidth");
            if (tilewidth.string_size == 0
                || false == compare_to_c_string(tilewidth, "16"))
            {
                add_error_to_report(transient_arena, &errors, "Map property 'Tile Width' should be set to 16.");
            }

            string_ref tileheight = get_attribute_value(map_node, "tileheight");
            if (tileheight.string_size == 0
                || false == compare_to_c_string(tileheight, "16"))
            {
                add_error_to_report(transient_arena, &errors, "Map property 'Tile Height' should be set to 16.");
            }

            string_ref width = get_attribute_value(map_node, "width");
            string_ref height = get_attribute_value(map_node, "height");
            if (width.ptr && height.ptr)
            {
                level.width = parse_i32(width);
                level.height = parse_i32(height);

                level.map = parse_map_layer(&parsing, map_node, "map", true);
                level.background = parse_map_layer(&parsing, map_node, "background", true);
                level.foreground = parse_map_layer(&parsing, map_node, "foreground", true);
            }
            else
            {
                add_error_to_report(transient_arena, &errors, "Map doesn't have defined width or height.");
            }
            
            parse_map_properties(&parsing, map_node);	
        }
        else
        {
            add_error_to_report(transient_arena, &errors, "The 'map' element is missing.");
        }

        xml_node* objectgroup_node = find_tag_in_children(root, "objectgroup");
        if (objectgroup_node)
        {
            xml_node_search_result* objects = find_all_nodes_with_tag(transient_arena, objectgroup_node, "object");
            if (objects->found_nodes_count > 0)
            {
                for (u32 xml_node_index = 0; xml_node_index < objects->found_nodes_count; xml_node_index++)
                {
                    xml_node* node = *(objects->found_nodes + xml_node_index);
                    if (node)
                    {
                        parse_entity(&parsing, node);
                    }
                }
            }

            if (level.starting_tile.x == -1 || level.starting_tile.y == -1)
            {
                add_error_to_report(transient_arena, &errors, "Starting position is not set.");
            }
        }
        else
        {
            add_error_to_report(transient_arena, &errors, "The 'objectgroup' element is missing.");
        }
    }
    else
    {
        if (file.contents)
        {
            add_error_to_report(transient_arena, &errors, "File is not a valid TMX format file.");
        }
        else
        {
            snprintf(errors.message_buffer, errors.message_buffer_size,
                "File '%.*s.tmx' not found.", map_name.string_size, map_name.ptr);
            add_error_to_report(transient_arena, &errors, errors.message_buffer);
        }
    }

end_of_read_map_from_tmx_file_function:

    if (clean_up_transient_arena)
    {
        end_temporary_memory(memory_for_parsing, true);
    }

    result.parsed_map = level;
    result.errors = errors.report;

    return result;
}

map_layer load_collision_map(memory_arena* permanent_arena, memory_arena* transient_arena, read_file_result file)
{
    map_layer result = {0};

    temporary_memory memory_for_parsing = begin_temporary_memory(transient_arena);

    xml_node* root = scan_and_parse_tmx(transient_arena, file.contents, file.size);
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

tmx_map_parsing_result load_map(platform_api* platform, string_ref map_name, memory_arena* arena, memory_arena* transient_arena)
{
    tmx_map_parsing_result result = {0};

    char path_buffer[1000];
    snprintf(path_buffer, 1000, "data/%.*s.tmx", map_name.string_size, map_name.ptr);

    read_file_result map_file = platform->read_file(path_buffer);	
    if (map_file.contents)
    {
        result = read_map_from_tmx_file(arena, transient_arena, map_file, map_name, false);
        free(map_file.contents);
    }
    else
    {
        snprintf(path_buffer, 1000, "mapmaking/%.*s.tmx", map_name.string_size, map_name.ptr);
        map_file = platform->read_file(path_buffer);
        result = read_map_from_tmx_file(arena, transient_arena, map_file, map_name, false);
        free(map_file.contents);
    }	

    return result;
}

string_ref get_parsing_errors_message(memory_arena* arena, 
    render_text_options* text_options, tmx_parsing_error_report* errors)
{
    text_area_limits limits = get_text_area_limits(text_options);

    string_builder builder = get_string_builder(arena, limits.max_character_count);
    push_c_string_to_builder(&builder, "Unable to read a level from the TMX file! Errors:\n");

    char counter_buffer[10];
    u32 printed_errors_count = 0;
    u32 max_printer_errors_count = 100;

    tmx_parsing_error* error = errors->first_error;
    while (error && (printed_errors_count < max_printer_errors_count))
    {
        snprintf(counter_buffer, 10, "%d. ", printed_errors_count + 1);

        push_c_string_to_builder(&builder, counter_buffer);
        push_string_to_builder(&builder, error->message);
        if (false == ends_with(error->message, "."))
        {
            push_char_to_builder(&builder, '.');
        }
        push_char_to_builder(&builder, '\n');

        printed_errors_count++;
        error = error->next;
    }

    i32 remaining_errors = (errors->errors_count - printed_errors_count);
    if (remaining_errors > 0)
    {
        snprintf(counter_buffer, 10, "And %d more errors", remaining_errors);
        push_c_string_to_builder(&builder, counter_buffer);
    }

    string_ref result = get_string_from_string_builder(&builder);
    return result;
}