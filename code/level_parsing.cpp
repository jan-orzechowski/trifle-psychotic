#include "main.h"
#include "map.h"
#include "tmx_parsing.h"
#include "level_parsing.h"
#include "text_rendering.h"

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
	i32 gid = -1;
	if (gid_str.string_size)
	{
		gid = parse_i32(gid_str);
		gid -= (entity_tileset_first_gid - 1);
		switch (gid)
		{
			case 7:  type = entity_type_enum::ENEMY_ROBOT; break;
			case 8:  type = entity_type_enum::ENEMY_CULTIST; break;
			case 9:  type = entity_type_enum::ENEMY_FLYING_BOMB; break;
			case 10: type = entity_type_enum::ENEMY_GUARDIAN; break;
			case 11: type = entity_type_enum::ENEMY_SENTRY; break;
			case 13: type = entity_type_enum::POWER_UP_HEALTH; break;
			case 14: type = entity_type_enum::POWER_UP_INVINCIBILITY; break;
			case 15: type = entity_type_enum::POWER_UP_SPEED; break;
			case 16: type = entity_type_enum::POWER_UP_DAMAGE; break;
			case 17: type = entity_type_enum::POWER_UP_SPREAD; break;
			case 19: type = entity_type_enum::GATE_BLUE; break;
			case 20: type = entity_type_enum::GATE_GREY; break;
			case 21: type = entity_type_enum::GATE_RED; break;
			case 22: type = entity_type_enum::GATE_GREEN; break;
			case 25: type = entity_type_enum::SWITCH_BLUE; break;
			case 26: type = entity_type_enum::SWITCH_GREY; break;
			case 27: type = entity_type_enum::SWITCH_RED; break;
			case 28: type = entity_type_enum::SWITCH_GREEN; break;
			case 1:  type = entity_type_enum::PLAYER; break;
			case 2:  type = entity_type_enum::NEXT_LEVEL_TRANSITION; break;
			case 3:  type = entity_type_enum::MESSAGE_DISPLAY; break;
		}
	}

	switch (type)
	{
		case entity_type_enum::GATE_BLUE:
		case entity_type_enum::GATE_GREY:
		case entity_type_enum::GATE_RED:
		case entity_type_enum::GATE_GREEN:
		case entity_type_enum::SWITCH_BLUE:
		case entity_type_enum::SWITCH_GREY:
		case entity_type_enum::SWITCH_RED:
		case entity_type_enum::SWITCH_GREEN:
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
								snprintf(error_buffer, ERROR_BUFFER_LENGTH,
									"Next level name is longer than %d characters",
									MAX_LEVEL_NAME_LENGTH);
								add_error(transient_arena, errors, error_buffer);
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

string_ref get_parsing_errors_message(memory_arena* arena, render_group* render,
	font font, rect textbox_area, tmx_parsing_error_report* errors)
{
	text_area_limits limits = get_text_area_limits(font, textbox_area);

	string_builder builder = get_string_builder(arena, limits.max_character_count);
	push_string(&builder, "Unable to read a level from the TMX file! Errors:\n");

	char counter_buffer[10];
	u32 printed_errors_count = 0;
	u32 max_printer_errors_count = 100;

	tmx_parsing_error* error = errors->first_error;
	while (error && (printed_errors_count < max_printer_errors_count))
	{
		snprintf(counter_buffer, 10, "%d. ", printed_errors_count + 1);

		push_string(&builder, counter_buffer);
		push_string(&builder, error->message);
		push_string(&builder, "\n");

		printed_errors_count++;
		error = error->next;
	}

	i32 remaining_errors = (errors->errors_count - printed_errors_count);
	if (remaining_errors > 0)
	{
		snprintf(counter_buffer, 10, "And %d more errors", remaining_errors);
		push_string(&builder, counter_buffer);
	}

	string_ref result = get_string_from_string_builder(&builder);
	return result;
}