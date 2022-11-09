#include "main.h"
#include "map.h"
#include "animation.h"
#include "entities.h"
#include "player.h"

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
b32 check_segment_intersection(r32 movement_start_x, r32 movement_start_y,
    r32 movement_delta_x, r32 movement_delta_y,
    r32 line_x, r32 min_segment_y, r32 max_segment_y, r32* min_movement_perc)
{
    b32 result = false;
    r32 movement_perc = 0;
    if (check_line_intersection(movement_start_x, movement_delta_x, line_x, &movement_perc))
    {
        v2 movement_start = get_v2(movement_start_x, movement_start_y);
        v2 movement_delta = get_v2(movement_delta_x, movement_delta_y);
        v2 intersection_pos = add_v2(movement_start, scalar_multiply_v2(movement_delta, movement_perc));
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

entity_collision_data get_entity_collision_data(chunk_position reference_chunk, entity* entity)
{
    entity_collision_data result = {0};
    result.position = get_world_pos_and_chunk_position_difference(entity->position, reference_chunk);
    result.collision_rect_dim = entity->type->collision_rect_dim;
    result.collision_rect_offset = entity->type->collision_rect_offset;
    return result;
}

entity_collision_data get_bullet_collision_data(chunk_position reference_chunk, bullet* bullet)
{
    entity_collision_data result = {0};
    result.position = get_world_pos_and_chunk_position_difference(bullet->position, reference_chunk);
    result.collision_rect_dim = bullet->type->collision_rect_dim;
    result.collision_rect_offset = bullet->type->collision_rect_offset;
    return result;
}

entity_collision_data get_tile_collision_data(chunk_position reference_chunk, tile_position tile_pos)
{
    entity_collision_data result = {0};
    result.position = get_tile_pos_and_chunk_position_difference(tile_pos, reference_chunk);
    result.collision_rect_dim = get_v2(1.0f, 1.0f);
    return result;
}

entity_collision_data get_point_collision_data(chunk_position reference_chunk, world_position point)
{
    entity_collision_data result = {0};
    result.position = get_world_pos_and_chunk_position_difference(point, reference_chunk);
    result.collision_rect_dim = get_v2(0.01f, 0.01f);
    return result;
}

v2 get_collision_dim_from_tile_range(tile_range path)
{
    v2 result = {0};
    v2 distance = get_tile_position_difference(path.end, path.start);
    if (distance.x > 0)
    {
        // pozioma
        result.x = distance.x + 1.0f;
        result.y = 1.0f;
    }
    else
    {
        // pionowa
        result.x = 1.0f;
        result.y = distance.y + 1.0f;
    }
    return result;
}

collision check_minkowski_collision(
    entity_collision_data a,
    entity_collision_data b,
    v2 movement_delta, r32 min_movement_perc)
{
    collision result = {0};
    result.possible_movement_perc = min_movement_perc;

    v2 relative_pos = subtract_v2(
        add_v2(a.position, a.collision_rect_offset),
        add_v2(b.position, b.collision_rect_offset));

    // środkiem zsumowanej figury jest (0,0,0)
    // pozycję playera traktujemy jako odległość od 0
    // 0 jest pozycją entity, z którym sprawdzamy kolizję

    v2 minkowski_dimensions = add_v2(a.collision_rect_dim, b.collision_rect_dim);
    v2 min_corner = scalar_multiply_v2(minkowski_dimensions, -0.5f);
    v2 max_corner = scalar_multiply_v2(minkowski_dimensions, 0.5f);

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

    if (west_wall)
    {
        result.collided_wall = DIRECTION_W;
        result.collided_wall_normal = get_v2(-1, 0);
    }
    else if (east_wall)
    {
        result.collided_wall = DIRECTION_E;
        result.collided_wall_normal = get_v2(1, 0);
    }
    else if (north_wall)
    {
        result.collided_wall = DIRECTION_N;
        result.collided_wall_normal = get_v2(0, 1);
    }
    else if (south_wall)
    {
        result.collided_wall = DIRECTION_S;
        result.collided_wall_normal = get_v2(0, -1);
    }

    return result;
}

rect get_tiles_area_to_check_for_collision(world_position entity_position, v2 collision_rect_offset, v2 collision_rect_dim, world_position target_pos)
{
    entity_position.pos_in_chunk = add_v2(entity_position.pos_in_chunk, collision_rect_offset);

    tile_position entity_tile = get_tile_pos_from_world_pos(entity_position);
    tile_position target_tile = get_tile_pos_from_world_pos(target_pos);

    // domyślne zachowanie casta to obcięcie części ułamkowej
    i32 x_margin = (i32)ceil(collision_rect_dim.x);
    i32 y_margin = (i32)ceil(collision_rect_dim.y);

    rect result = {0};
    result.min_corner.x = (r32)min((i32)entity_tile.x - x_margin, (i32)target_tile.x - x_margin);
    result.min_corner.y = (r32)min((i32)entity_tile.y - y_margin, (i32)target_tile.y - y_margin);
    result.max_corner.x = (r32)max((i32)entity_tile.x + x_margin, (i32)target_tile.x + x_margin);
    result.max_corner.y = (r32)max((i32)entity_tile.y + y_margin, (i32)target_tile.y + y_margin);
    return result;
}

rect get_tiles_area_to_check_for_entity_collision(entity* entity, world_position target_pos)
{
    rect result = get_tiles_area_to_check_for_collision(
        entity->position, entity->type->collision_rect_offset, entity->type->collision_rect_dim, target_pos);
    return result;
}

rect get_tiles_area_to_check_for_bullet_collision(bullet* bullet, world_position target_pos)
{
    rect result = get_tiles_area_to_check_for_collision(
        bullet->position, bullet->type->collision_rect_offset, bullet->type->collision_rect_dim, target_pos);
    return result;
}

rect get_tile_colliding_rect(chunk_position reference_chunk, i32 tile_x, i32 tile_y)
{
    v2 position = get_tile_pos_and_chunk_position_difference(get_tile_pos(tile_x, tile_y), reference_chunk);
    v2 dimensions = get_v2(1.0f, 1.0f);
    rect result = get_rect_from_center_and_dimensions(position, dimensions);
    return result;
}

rect get_entity_colliding_rect(entity_collision_data collision_data)
{
    v2 position = add_v2(collision_data.position, collision_data.collision_rect_offset);
    v2 dimensions = collision_data.collision_rect_dim;
    rect result = get_rect_from_center_and_dimensions(position, dimensions);
    return result;
}

b32 check_if_sight_line_is_obstructed(level_state* level, world_position start, world_position end)
{
    b32 path_obstructed = false;
    chunk_position reference_chunk = get_chunk_pos_from_world_pos(start);

    v2 movement_delta = get_world_position_difference(end, start);

    entity_collision_data photon_collision = {0};
    photon_collision.position = get_world_pos_and_chunk_position_difference(start, reference_chunk);
    photon_collision.collision_rect_dim = get_v2(0.01f, 0.01f);
    photon_collision.collision_rect_offset = get_zero_v2();

    // collision with tiles
    rect area_to_check = get_tiles_area_to_check_for_collision(start, get_zero_v2(), get_zero_v2(), end);
    for (i32 tile_y_to_check = (i32)area_to_check.min_corner.y - 1;
        tile_y_to_check <= (i32)area_to_check.max_corner.y + 1;
        tile_y_to_check++)
    {
        for (i32 tile_x_to_check = (i32)area_to_check.min_corner.x - 1;
            tile_x_to_check <= (i32)area_to_check.max_corner.x + 1;
            tile_x_to_check++)
        {
            if (is_tile_at_coords_colliding(&level->current_map, tile_x_to_check, tile_y_to_check))
            {
                entity_collision_data entity_collision = get_tile_collision_data(
                    reference_chunk, get_tile_pos(tile_x_to_check, tile_y_to_check));
                collision collision = check_minkowski_collision(
                    photon_collision, entity_collision, movement_delta, 1.0f);
                
                if (collision.collided_wall != DIRECTION_NONE)
                {
                    path_obstructed = true;
                    goto check_sight_line_end;
                }
            }
        }
    }	

    // collision with entities
    for (u32 entity_index = 1; entity_index < level->entities_count; entity_index++)
    {
        entity* entity = level->entities + entity_index;
        if (false == entity->used)
        {
            continue;
        }

        if (has_entity_flags_set(entity, ENTITY_FLAG_BLOCKS_MOVEMENT)
            && (has_entity_flags_set(entity, ENTITY_FLAG_GATE)
                || has_entity_flags_set(entity, ENTITY_FLAG_SWITCH)
                || has_entity_flags_set(entity, ENTITY_FLAG_MOVING_PLATFORM_HORIZONTAL)
                || has_entity_flags_set(entity, ENTITY_FLAG_MOVING_PLATFORM_VERTICAL))
            && is_in_neighbouring_chunk(reference_chunk, entity->position))
        {
            entity_collision_data entity_collision = get_entity_collision_data(reference_chunk, entity);
            collision collision = check_minkowski_collision(
                photon_collision, entity_collision, movement_delta, 1.0f);
            
            if (collision.collided_wall != DIRECTION_NONE)
            {
                path_obstructed = true;
                goto check_sight_line_end;
            }
        }
    }
    

check_sight_line_end:
    return path_obstructed;
}

// uwaga - zakładamy, że początkowe i końcowe pole leżą na linii równoległej do jednej z osi układu współrzędnych
tile_range find_path_fragment_not_blocked_by_entities(level_state* level, tile_range path)
{
    tile_range result = get_invalid_tile_range();

    world_position start_pos = get_world_pos_from_tile_pos(path.start);
    chunk_position reference_chunk = get_tile_chunk_pos(path.start);

    v2 movement_delta = get_tile_position_difference(path.end, path.start);
    if (false == is_zero_v2(movement_delta))
    {
        collision closest_collision = {0};
        closest_collision.collided_wall = DIRECTION_NONE;
        closest_collision.possible_movement_perc = 1.0f;

        entity_collision_data point_collision = get_point_collision_data(reference_chunk, start_pos);
        for (u32 entity_index = 1; entity_index < level->entities_count; entity_index++)
        {
            entity* entity = level->entities + entity_index;
            if (false == entity->used)
            {
                continue;
            }

            if ((has_entity_flags_set(entity, ENTITY_FLAG_GATE)
                || has_entity_flags_set(entity, ENTITY_FLAG_SWITCH))
                && has_entity_flags_set(entity, ENTITY_FLAG_BLOCKS_MOVEMENT)
                && is_in_neighbouring_chunk(reference_chunk, entity->position))
            {
                entity_collision_data entity_collision = get_entity_collision_data(reference_chunk, entity);
                collision new_collision = check_minkowski_collision(point_collision, entity_collision,
                    movement_delta, closest_collision.possible_movement_perc);

                if (new_collision.collided_wall != DIRECTION_NONE)
                {
                    if (new_collision.possible_movement_perc < closest_collision.possible_movement_perc)
                    {
                        closest_collision = new_collision;
                    }
                }
            }
        }
                
        if (closest_collision.possible_movement_perc < 1.0f)
        {
            result.start = path.start;

            world_position collided_pos = add_to_world_position(start_pos, 
                scalar_multiply_v2(movement_delta, closest_collision.possible_movement_perc));
            tile_position collided_tile_pos = get_tile_pos_from_world_pos(collided_pos);

            if (movement_delta.x > 0)
            {
                result.end = collided_tile_pos;
            }
            else if (movement_delta.x < 0)
            {
                result.end = add_to_tile_position(collided_tile_pos, 1, 0);
            }
            else if (movement_delta.y > 0)
            {
                result.end = collided_tile_pos;
            }
            else if (movement_delta.y < 0)
            {
                result.end = add_to_tile_position(collided_tile_pos, 0, 1);
            }
        }
        else
        {
            result.start = path.start;
            result.end = path.end;
        }
    }
    else
    {
        result.start = path.start;
        result.end = path.end;
    }

    return result;
}

collision_result move(level_state* level, entity* moving_entity, world_position target_pos)
{
    collision_result result = {0};

    v2 movement_delta = get_world_position_difference(target_pos, moving_entity->position);
    chunk_position reference_chunk = get_chunk_pos_from_world_pos(moving_entity->position);

    if (false == is_zero_v2(movement_delta))
    {
        r32 movement_apron = 0.001f;

        for (u32 iteration = 0; iteration < 4; iteration++)
        {
            if (is_zero_v2(movement_delta))
            {
                break;
            }

            collision closest_collision = {0};
            closest_collision.collided_wall = DIRECTION_NONE;
            closest_collision.possible_movement_perc = 1.0f;

            // collision with tiles
            {
                rect area_to_check = get_tiles_area_to_check_for_entity_collision(moving_entity, target_pos);
                for (i32 tile_y_to_check = area_to_check.min_corner.y;
                    tile_y_to_check <= area_to_check.max_corner.y;
                    tile_y_to_check++)
                {
                    for (i32 tile_x_to_check = area_to_check.min_corner.x;
                        tile_x_to_check <= area_to_check.max_corner.x;
                        tile_x_to_check++)
                    {
                        tile_position tile_to_check_pos = get_tile_pos(tile_x_to_check, tile_y_to_check);
                        if (is_tile_at_coords_colliding(&level->current_map, tile_x_to_check, tile_y_to_check))
                        {
                            collision new_collision = check_minkowski_collision(
                                get_entity_collision_data(reference_chunk, moving_entity),
                                get_tile_collision_data(reference_chunk, tile_to_check_pos),
                                movement_delta, closest_collision.possible_movement_perc);

                            if (new_collision.collided_wall != DIRECTION_NONE)
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

            collision closest_tile_collision = closest_collision;

            // collision with entities
            {
                for (u32 entity_index = 0; entity_index < level->entities_count; entity_index++)
                {
                    entity* entity_to_check = level->entities + entity_index;
                    if (false == entity_to_check->used)
                    {
                        continue;
                    }

                    if (entity_to_check != moving_entity
                        && is_in_neighbouring_chunk(reference_chunk, entity_to_check->position))
                    {
                        collision new_collision = check_minkowski_collision(
                            get_entity_collision_data(reference_chunk, moving_entity),
                            get_entity_collision_data(reference_chunk, entity_to_check),
                            movement_delta, closest_collision.possible_movement_perc);

                        if (new_collision.collided_wall != DIRECTION_NONE)
                        {
                            // bierzemy tutaj uwagę tylko entities, z którymi kolidowalibyśmy wcześniej niż ze ścianą
                            if (new_collision.possible_movement_perc < closest_tile_collision.possible_movement_perc)
                            {
                                if (has_entity_flags_set(moving_entity, ENTITY_FLAG_PLAYER))
                                {
                                    if (has_entity_flags_set(entity_to_check, ENTITY_FLAG_ENEMY))
                                    {
                                        if (false == ignore_player_and_enemy_collisions(level))
                                        {
                                            result.collided_enemy = entity_to_check;
                                        }										
                                    }

                                    if (has_entity_flags_set(entity_to_check, ENTITY_FLAG_MOVING_PLATFORM_HORIZONTAL)
                                        || has_entity_flags_set(entity_to_check, ENTITY_FLAG_MOVING_PLATFORM_VERTICAL))
                                    {
                                        result.collided_platform = entity_to_check;
                                    }

                                    if (has_entity_flags_set(entity_to_check, ENTITY_FLAG_SWITCH))
                                    {
                                        result.collided_switch = entity_to_check;
                                    }

                                    if (has_entity_flags_set(entity_to_check, ENTITY_FLAG_POWER_UP))
                                    {
                                        result.collided_power_up = entity_to_check;
                                    }

                                    if (entity_to_check->type->type_enum == ENTITY_TYPE_NEXT_LEVEL_TRANSITION)
                                    {
                                        result.collided_transition = entity_to_check;
                                    }

                                    if (has_entity_flags_set(entity_to_check, ENTITY_FLAG_MESSAGE_DISPLAY))
                                    {
                                        result.collided_message_display = entity_to_check;
                                    }
                                }
                            }

                            if (has_entity_flags_set(entity_to_check, ENTITY_FLAG_BLOCKS_MOVEMENT))
                            {
                                if (has_entity_flags_set(moving_entity, ENTITY_FLAG_PLAYER)
                                    && has_entity_flags_set(entity_to_check, ENTITY_FLAG_ENEMY))
                                {
                                    if (false == ignore_player_and_enemy_collisions(level))
                                    {
                                        closest_collision = new_collision;
                                    }
                                }
                                else
                                {
                                    closest_collision = new_collision;
                                }
                            }
                        }
                    }
                }
            }

            // przesuwamy się o tyle, o ile możemy
            if ((closest_collision.possible_movement_perc - movement_apron) > 0.0f)
            {
                v2 possible_movement = scalar_multiply_v2(movement_delta, (closest_collision.possible_movement_perc - movement_apron));
                moving_entity->position = add_to_world_position(moving_entity->position, possible_movement);
                // pozostałą deltę zmniejszamy o tyle, o ile się poruszyliśmy
                movement_delta = subtract_v2(movement_delta, possible_movement);
            }

            result.collision_data = closest_collision;

            if (false == is_zero_v2(closest_collision.collided_wall_normal))
            {
                v2 wall_normal = closest_collision.collided_wall_normal;

                // i sprawdzamy, co zrobić z pozostałą deltą - czy możemy się poruszyć wzdłuż ściany lub odbić
                i32 how_many_times_subtract = 1; // 1 dla ślizgania się, 2 dla odbijania    
                v2 bounced = scalar_multiply_v2(
                    wall_normal, 
                    inner_v2(wall_normal, moving_entity->velocity));
                moving_entity->velocity = subtract_v2(
                    moving_entity->velocity, 
                    scalar_multiply_v2(bounced, how_many_times_subtract));

                movement_delta = subtract_v2(
                    movement_delta,
                    scalar_multiply_v2(
                        scalar_multiply_v2(
                            wall_normal, 
                            inner_v2(movement_delta, wall_normal)), 
                        how_many_times_subtract));
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

b32 move_bullet(level_state* level, bullet* moving_bullet, world_position target_pos)
{
    v2 movement_delta = get_world_position_difference(target_pos, moving_bullet->position);
    chunk_position reference_chunk = get_chunk_pos_from_world_pos(moving_bullet->position);

    v2 relative_target_pos = get_world_pos_and_chunk_position_difference(target_pos, reference_chunk);

    r32 collision_closest_distance = R32_MAX_VALUE;
    entity* hit_entity = NULL;
    b32 hit_wall = false;

    if (false == is_zero_v2(movement_delta))
    {
        // collision with tiles
        {
            rect area_to_check = get_tiles_area_to_check_for_bullet_collision(moving_bullet, target_pos);
            for (i32 tile_y_to_check = area_to_check.min_corner.y - 1;
                tile_y_to_check <= area_to_check.max_corner.y + 1;
                tile_y_to_check++)
            {
                for (i32 tile_x_to_check = area_to_check.min_corner.x - 1;
                    tile_x_to_check <= area_to_check.max_corner.x + 1;
                    tile_x_to_check++)
                {
                    if (is_tile_at_coords_colliding(&level->current_map, tile_x_to_check, tile_y_to_check))
                    {
                        rect tile_colliding_rect = get_tile_colliding_rect(reference_chunk, tile_x_to_check, tile_y_to_check);
                        if (is_point_inside_rect(tile_colliding_rect, relative_target_pos))
                        {
                            r32 distance = length_v2(get_tile_pos_and_world_position_difference(
                                get_tile_pos(tile_x_to_check, tile_y_to_check), moving_bullet->position));
                            if (distance < collision_closest_distance)
                            {
                                collision_closest_distance = distance;
                                hit_wall = true;
                            }
                        }
                    }
                }
            }
        }

        // collision with entities
        {
            for (u32 entity_index = 0; entity_index < level->entities_count; entity_index++)
            {
                entity* entity_to_check = level->entities + entity_index;
                if (false == entity_to_check->used)
                {
                    continue;
                }

                if (has_entity_flags_set(entity_to_check, ENTITY_FLAG_BLOCKS_MOVEMENT)
                    && is_in_neighbouring_chunk(reference_chunk, entity_to_check->position))
                {
                    rect entity_colliding_rect = get_entity_colliding_rect(get_entity_collision_data(reference_chunk, entity_to_check));
                    if (is_point_inside_rect(entity_colliding_rect, relative_target_pos))
                    {
                        r32 distance = length_v2(get_world_position_difference(entity_to_check->position, moving_bullet->position));

                        if (has_entity_flags_set(entity_to_check, ENTITY_FLAG_SWITCH)
                            || has_entity_flags_set(entity_to_check, ENTITY_FLAG_GATE)
                            || has_entity_flags_set(entity_to_check, ENTITY_FLAG_MOVING_PLATFORM_HORIZONTAL)
                            || has_entity_flags_set(entity_to_check, ENTITY_FLAG_MOVING_PLATFORM_VERTICAL))
                        {
                            collision_closest_distance = distance;
                            hit_wall = true;
                        }

                        if (are_entity_flags_set(&moving_bullet->type->flags, ENTITY_FLAG_DAMAGES_PLAYER))
                        {
                            if (entity_index == 0)
                            {
                                // mamy gracza							
                                if (distance < collision_closest_distance)
                                {
                                    collision_closest_distance = distance;
                                    hit_entity = entity_to_check;
                                }
                            }
                        }
                        else
                        {
                            if (entity_index != 0)
                            {
                                // mamy przeciwnika
                                if (distance < collision_closest_distance)
                                {
                                    collision_closest_distance = distance;
                                    hit_entity = entity_to_check;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (hit_entity)
        {
            if (false == has_entity_flags_set(hit_entity, ENTITY_FLAG_INDESTRUCTIBLE))
            {
                start_bullet_death_animation(level, moving_bullet);
            }
        }

        if (hit_entity)
        {
            handle_entity_and_bullet_collision(level, moving_bullet, hit_entity);
        }
        else if (false == hit_wall)
        {
            moving_bullet->position = add_to_world_position(moving_bullet->position, movement_delta);
        }
    }

    return (hit_entity || hit_wall);
}

b32 is_standing_on_ground(level_state* level, entity* entity_to_check, collision_result* collision_to_fill)
{
    b32 result = false;

    entity test_entity = *entity_to_check;
    world_position target_pos = add_to_world_position(test_entity.position, get_v2(0.0f, 0.1f));
    collision_result collision = move(level, &test_entity, target_pos);

    // przypadkowo możemy mieć kolizję z inną ścianą niż górna - ale tutaj to pomijamy
    if (collision.collision_data.collided_wall == DIRECTION_S)
    {
        result = true;
        if (collision_to_fill)
        {
            *collision_to_fill = collision;
        }
    }

    return result;
}