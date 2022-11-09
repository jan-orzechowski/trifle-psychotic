#include "main.h"
#include "entities.h"
#include "map.h"
#include "special_entities.h"
#include "collision.h"
#include "player.h"

b32 are_entity_flags_set(entity_flags* flags, entity_flags flag_values_to_check)
{
    b32 result = are_flags_set((u32*)flags, (u32)flag_values_to_check);
    return result;
}

void set_entity_flags(entity_flags* flags, entity_flags flag_values_to_check)
{
    set_flags((u32*)flags, (u32)flag_values_to_check);
}

void unset_entity_flags(entity_flags* flags, entity_flags flag_values_to_check)
{
    unset_flags((u32*)flags, (u32)flag_values_to_check);
}

b32 has_entity_flags_set(entity* entity, entity_flags flag_values)
{
    b32 result = are_flags_set((u32*)&entity->type->flags, (u32)flag_values);
    return result;
}

entity* add_entity_at_world_position(level_state* level, world_position position, entity_type* type)
{
    assert(type);

    entity* new_entity = NULL;
    if (level->entities_count < level->entities_max_count)
    {
        new_entity = &level->entities[level->entities_count];
        level->entities_count++;
        new_entity->position = renormalize_world_position(position);
        new_entity->type = type;
        new_entity->health = type->max_health;
        new_entity->direction = DIRECTION_W;
        new_entity->used = true;
    }
    
    return new_entity;
}

entity* add_entity_at_tile_position(level_state* level, tile_position position, entity_type* type)
{
    entity* result = add_entity_at_world_position(level, get_world_pos_from_tile_pos(position), type);
    return result;
}

void remove_entity(entity* entity_to_remove)
{
    *entity_to_remove = (entity){0};
}

entity_type_dictionary create_entity_types_dictionary(memory_arena* arena)
{
    entity_type_dictionary result = {0};
    result.type_ptrs_count = (i32)_LAST_ENTITY_TYPE + 1;
    result.type_ptrs = push_array(arena, result.type_ptrs_count, entity_type*);
    return result;
}

void set_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type, entity_type* enity_type_ptr)
{
    assert((i32)type < dictionary.type_ptrs_count);
    assert(dictionary.type_ptrs[(i32)type] == NULL);
    dictionary.type_ptrs[(i32)type] = enity_type_ptr;
}

entity_type* get_entity_type_ptr(entity_type_dictionary dictionary, entity_type_enum type)
{
    assert((i32)type < dictionary.type_ptrs_count);
    entity_type* result = dictionary.type_ptrs[(i32)type];
    return result;
}

b32 is_entity_moving_type_entity(entity* entity)
{
    b32 result = 
          (has_entity_flags_set(entity, ENTITY_FLAG_WALKS_HORIZONTALLY)
        || has_entity_flags_set(entity, ENTITY_FLAG_FLIES_HORIZONTALLY)
        || has_entity_flags_set(entity, ENTITY_FLAG_FLIES_VERTICALLY)
        || has_entity_flags_set(entity, ENTITY_FLAG_FLIES_TOWARDS_PLAYER)
        || has_entity_flags_set(entity, ENTITY_FLAG_MOVING_PLATFORM_HORIZONTAL)
        || has_entity_flags_set(entity, ENTITY_FLAG_MOVING_PLATFORM_VERTICAL));
    return result;
}

void fire_bullet(level_state* level, entity_type* bullet_type, world_position bullet_starting_position,
    v2 bullet_offset, v2 velocity)
{
    if (level->bullets_count < level->bullets_max_count)
    {
        bullet* bul = &level->bullets[level->bullets_count];
        bul->type = bullet_type;
        bullet_starting_position.pos_in_chunk = add_v2(bullet_starting_position.pos_in_chunk, bullet_offset);
        bul->position = renormalize_world_position(bullet_starting_position);
        bul->velocity = velocity;
        level->bullets_count++;
    }
}

void remove_bullet(level_state* level, i32* bullet_index)
{
    if (level->bullets_count > 0)
    {
        assert(*bullet_index >= 0);
        assert(*bullet_index < level->bullets_max_count);

        // compact array
        bullet* last_bullet = &level->bullets[level->bullets_count - 1];
        level->bullets[*bullet_index] = *last_bullet;
        level->bullets_count--;
        *last_bullet = (bullet){0}; // czyszczenie

        if (*bullet_index > 0)
        {
            (*bullet_index)--;
        }
    }
}

tile_range find_walking_path(map* level, tile_position start_tile)
{
    b32 found_good_start_pos = false;
    tile_position good_start_tile = {0};

    tile_position test_tile = start_tile;
    i32 distance_checking_limit = 20;
    for (i32 distance = 0; distance < distance_checking_limit; distance++)
    {
        test_tile.y = start_tile.y + distance;
        if (is_good_for_walk_path(level, test_tile.x, test_tile.y))
        {
            good_start_tile = test_tile;
            found_good_start_pos = true;
            break;
        }
    }

    if (good_start_tile.x == 0 && good_start_tile.y == 0)
    {
        return (tile_range){0};
    }

    tile_position left_end = good_start_tile;
    tile_position right_end = good_start_tile;

    test_tile = good_start_tile;
    for (i32 distance = 0; distance <= distance_checking_limit; distance++)
    {
        test_tile.x = good_start_tile.x - distance;
        if (is_good_for_walk_path(level, test_tile.x, test_tile.y))
        {
            left_end = test_tile;
        }
        else
        {
            break;
        }
    }

    test_tile = good_start_tile;
    for (i32 distance = 0; distance <= distance_checking_limit; distance++)
    {
        test_tile.x = good_start_tile.x + distance;
        if (is_good_for_walk_path(level, test_tile.x, test_tile.y))
        {
            right_end = test_tile;
        }
        else
        {
            break;
        }
    }

    tile_range result = {0};
    result.start = left_end;
    result.end = right_end;
    return result;
}

tile_range find_walking_path_not_blocked_by_entities(level_state* level, tile_position starting_position)
{
    tile_range new_path = find_walking_path(&level->current_map, starting_position);

    tile_range first_part = get_tile_range(starting_position, new_path.start);
    tile_range second_part = get_tile_range(starting_position, new_path.end);
    first_part = find_path_fragment_not_blocked_by_entities(level, first_part);
    second_part = find_path_fragment_not_blocked_by_entities(level, second_part);

    new_path = get_tile_range(first_part.end, second_part.end);

    return new_path;
}

tile_range find_flying_path(level_state* level, tile_position starting_position, b32 vertical)
{
    tile_range new_path;
    
    if (vertical)
    {
        new_path = find_vertical_range_of_free_tiles(&level->current_map, 
            starting_position, level->current_map.height);
    }
    else
    {
        new_path = find_horizontal_range_of_free_tiles(&level->current_map, 
            starting_position, level->current_map.width);
    }
    
    tile_range first_part = get_tile_range(starting_position, new_path.start);
    tile_range second_part = get_tile_range(starting_position, new_path.end);
    first_part = find_path_fragment_not_blocked_by_entities(level, first_part);
    second_part = find_path_fragment_not_blocked_by_entities(level, second_part);

    new_path = get_tile_range(first_part.end, second_part.end);

    return new_path;
}

void find_walking_path_for_enemy(level_state* level, entity* enemy)
{
    tile_range new_path = find_walking_path_not_blocked_by_entities(level, get_tile_pos_from_world_pos(enemy->position));
    enemy->path = new_path;
    enemy->has_path = true;
}

void find_flying_path_for_enemy(level_state* level, entity* enemy, b32 vertical)
{
    tile_range new_path = find_flying_path(level, get_tile_pos_from_world_pos(enemy->position), vertical);
    
    // dodajemy ograniczenie, by zatrzymywać się dwa pola nad ziemią
    if (new_path.start.x != 0 && new_path.start.y != 0)
    {
        if (new_path.end.y - 2 > new_path.start.y)
        {
            new_path.end.y -= 2;
        }
    }

    enemy->path = new_path;
    enemy->has_path = true;
}

void find_path_for_moving_platform(level_state* level, entity* entity, b32 vertical)
{
    if (vertical)
    {	
        // musimy wziąć pod uwagę cały rozmiar platformy
        tile_position platform_middle_position = get_tile_pos_from_world_pos(entity->position);
        tile_position platform_left_position = add_to_tile_position(platform_middle_position, -1, 0);
        tile_position platform_right_position = add_to_tile_position(platform_middle_position, 1, 0);

        tile_range middle_path = find_vertical_range_of_free_tiles_downwards(&level->current_map, 
            platform_middle_position, level->current_map.height);
        tile_range left_path = find_vertical_range_of_free_tiles_downwards(&level->current_map,
            platform_left_position, level->current_map.height);
        tile_range right_path = find_vertical_range_of_free_tiles_downwards(&level->current_map,
            platform_right_position, level->current_map.height);

        u32 min_y = max_of_three(middle_path.start.y, left_path.start.y, right_path.start.y);
        u32 max_y = min_of_three(middle_path.end.y, left_path.end.y, right_path.end.y);

        tile_range new_path = {0};
        new_path.start = get_tile_pos(middle_path.start.x, min_y);
        new_path.end = get_tile_pos(middle_path.end.x, max_y);

        entity->path = find_path_fragment_not_blocked_by_entities(level, new_path);
        entity->has_path = true;

        if (entity->path.start.x != 0 && entity->path.start.y != 0)
        {
            // zatrzymujemy się przed sufitem, żeby nie zmiażdżyć gracza
            tile_position tile_above = get_tile_pos(entity->path.start.x, entity->path.start.y - 1);
            tile_position two_tiles_above = get_tile_pos(entity->path.start.x, entity->path.start.y - 2);
            if (is_tile_colliding(&level->current_map, tile_above))
            {
                // przesuwamy dwa do dołu
                if (entity->path.start.y + 2 < entity->path.end.y)
                {
                    entity->path.start.y += 2;
                }
            }
            else 
            if (is_tile_colliding(&level->current_map, two_tiles_above))
            {
                // przesuwamy jeden do dołu
                if (entity->path.start.y + 1 < entity->path.end.y)
                {
                    entity->path.start.y += 1;
                }
            }
        }

        //printf("moving platform from (%d,%d) to (%d,%d)\n",
        //	entity->path.start.x, entity->path.start.y, entity->path.end.x, entity->path.end.y);
    }
    else
    {
        find_flying_path_for_enemy(level, entity, false);
        if (entity->path.start.x != 0 && entity->path.start.y != 0)
        {
            // z racji szerokości platformy musimy zatrzymać się o pole wcześniej
            if (entity->path.start.x + 2 < entity->path.end.x)
            {
                entity->path.start.x++;
                entity->path.end.x--;
            }
        }
    }
}

// domyślnie trójkąt jest zwrócony podstawą w prawo
// można podać x i y odwrotnie dla trójkątów zwróconych podstawą w górę lub w dół
b32 is_point_within_right_triangle(r32 triangle_height, r32 relative_x, r32 relative_y, b32 invert_sign)
{
    b32 result = false;
    relative_x = (invert_sign ? -relative_x : relative_x);
    if (relative_x < triangle_height)
    {
        // trzeba pamiętać, że y idą do dołu ekranu
        r32 max_y = relative_x; // bok wyrażony f(x) = x
        r32 min_y = -relative_x; // bok wyrażony f(x) = -x
        if (relative_y <= max_y && relative_y >= min_y)
        {
            // znajdujemy się w trójkącie
            result = true;
        }
    }

    return result;
}

b32 is_point_visible_within_90_degrees(world_position looking_point, direction looking_direction, r32 max_looking_distance, world_position point_to_check)
{
    assert(looking_direction != DIRECTION_NONE);

    // sprawdzamy trójkąt z wierzchołkiem umiejscowionym w patrzącym entity
    // kąt przy tym wierzchołku jest prosty
    // a więc boki trójkąta można określić funkcjami f(x)=x, f(x)=-1
    b32 result = false;
    r32 triangle_height = max_looking_distance;

    v2 relative_pos = get_world_position_difference(point_to_check, looking_point);
    if (looking_direction == DIRECTION_E && relative_pos.x > 0.0f)
    {
        result = is_point_within_right_triangle(triangle_height, relative_pos.x, relative_pos.y, false);
    }
    else
    if (looking_direction == DIRECTION_W && relative_pos.x < 0.0f)
    {
        result = is_point_within_right_triangle(triangle_height, relative_pos.x, relative_pos.y, true);
    }
    else // odwracamy x i y
    if (looking_direction == DIRECTION_N && relative_pos.y > 0.0f)
    {
        result = is_point_within_right_triangle(triangle_height, relative_pos.y, relative_pos.x, false);
    }
    else
    if (looking_direction == DIRECTION_S && relative_pos.y < 0.0f)
    {
        result = is_point_within_right_triangle(triangle_height, relative_pos.y, relative_pos.x, true);
    }

    return result;
}

b32 is_point_visible_for_entity(level_state* level, entity* looking_entity, world_position point)
{
    b32 result = false;
    world_position looking_point = add_to_world_position(looking_entity->position, looking_entity->type->looking_position_offset);
    if (looking_entity->type->detection_type != DETECT_NOTHING)
    {
        v2 distance_to_point = get_world_position_difference(point, looking_point);
        if (length_v2(distance_to_point) <= looking_entity->type->detection_distance)
        {
            switch (looking_entity->type->detection_type)
            {
                case DETECT_180_DEGREES_BELOW:
                {
                    if (distance_to_point.y < 0)
                    {
                        result = true;
                    }
                }
                break;
                case DETECT_180_DEGREES_IN_FRONT:
                {
                    if (looking_entity->direction == DIRECTION_W)
                    {
                        if (distance_to_point.x <= 0)
                        {
                            result = true;
                        }
                    }
                    else
                    {
                        if (distance_to_point.x >= 0)
                        {
                            result = true;
                        }
                    }					
                }
                break;
                case DETECT_360_DEGREES:
                {
                    result = true;
                }
                break;
                case DETECT_90_DEGREES_IN_FRONT:
                {
                    result = is_point_visible_within_90_degrees(looking_point, looking_entity->direction,
                        looking_entity->type->detection_distance, point);
                }
                break;
            }
        }

        if (result)
        {
            if (check_if_sight_line_is_obstructed(level, looking_point, point))
            {
                result = false;
            }
        }
    }

    return result;
};

entity* add_explosion(level_state* level, world_position position, animation* explosion_animation)
{
    assert(explosion_animation);
    
    entity* new_explosion = NULL;
    if (level->explosions_count < level->explosions_max_count)
    {
        new_explosion = &level->explosions[level->explosions_count];
        level->explosions_count++;
        new_explosion->position = renormalize_world_position(position);
        new_explosion->current_animation = explosion_animation;
        new_explosion->animation_duration = 0.0f;

        // lustrzane odbicie animacji dla większej różnorodności wizualnej
        b32 flip = (rand() % 2 == 1);
        if (flip)
        {
            new_explosion->direction = DIRECTION_W;		
        }
        else
        {
            new_explosion->direction = DIRECTION_E;
        }
    }
    return new_explosion;
}

void remove_explosion(level_state* level, i32* explosion_index)
{
    if (level->explosions_count > 0)
    {
        assert(*explosion_index >= 0);
        assert(*explosion_index < level->explosions_max_count);

        // compact array - działa też w przypadku explosion_index == explosions_count - 1
        entity* last_explosion = &level->explosions[level->explosions_count - 1];
        level->explosions[*explosion_index] = *last_explosion;
        level->explosions_count--;
        *last_explosion = (entity){0}; // czyszczenie

        if (*explosion_index > 0)
        {
            (*explosion_index)--;
        }
    }
}

shooting_rotation get_entity_shooting_rotation(shooting_rotation_sprites* rotation_sprites, v2 shooting_direction)
{
    assert(rotation_sprites);
    shooting_rotation result = {0};

    r32 angle = atan2(shooting_direction.y, shooting_direction.x) * (180.0f / pi32);
    angle += 45.0f / 2.0f;
    
    if (angle > 180.0f)
    {
        angle = -360.0f + angle;
    }

    if (angle < -180.0f)
    {
        angle = 360.0f + angle;
    }

    // przypomnienie: N jest w dół ekranu!

    direction direction = DIRECTION_NONE;
    if      (0.0f    < angle && angle <= 45.0f)   direction = DIRECTION_E;
    else if (45.0f   < angle && angle <= 90.0f)   direction = DIRECTION_NE;
    else if (90.0f   < angle && angle <= 135.0f)  direction = DIRECTION_N;
    else if (135.0f  < angle && angle <= 180.0f)  direction = DIRECTION_NW;
    else if (-45.0f  < angle && angle <= 0.0f)	  direction = DIRECTION_SE;
    else if (-90.0f  < angle && angle <= -45.0f)  direction = DIRECTION_S;
    else if (-135.0f < angle && angle <= -90.0f)  direction = DIRECTION_SW;
    else if (-180.0f < angle && angle <= -135.0f) direction = DIRECTION_W;

    result.bullet_offset = rotation_sprites->right_bullet_offset;
    result.rotated_sprite = rotation_sprites->right;
    result.flip_horizontally = false;

    switch (direction)
    {
        case DIRECTION_E:
        {
            result.bullet_offset = rotation_sprites->right_bullet_offset;
            result.rotated_sprite = rotation_sprites->right;
        }
        break;
        case DIRECTION_NE:
        {
            result.bullet_offset = rotation_sprites->right_down_bullet_offset;
            result.rotated_sprite = rotation_sprites->right_down;
        }
        break;
        case DIRECTION_N:
        {
            result.bullet_offset = rotation_sprites->down_bullet_offset;
            result.rotated_sprite = rotation_sprites->down;
        }
        break;
        case DIRECTION_NW:
        {
            result.bullet_offset = reflection_over_y_axis_v2(
                rotation_sprites->right_down_bullet_offset);
            result.rotated_sprite = rotation_sprites->right_down;
            result.flip_horizontally = true;
        }
        break;
        case DIRECTION_W:
        {
            result.bullet_offset = reflection_over_y_axis_v2(
                rotation_sprites->right_bullet_offset);
            result.rotated_sprite = rotation_sprites->right;
            result.flip_horizontally = true;
        }
        break;
        case DIRECTION_SW:
        {
            result.bullet_offset = reflection_over_y_axis_v2(
                rotation_sprites->right_up_bullet_offset);
            result.rotated_sprite = rotation_sprites->right_up;
            result.flip_horizontally = true;
        }
        break;
        case DIRECTION_S:
        {
            result.bullet_offset = rotation_sprites->up_bullet_offset;
            result.rotated_sprite = rotation_sprites->up;
        }
        break;
        case DIRECTION_SE:
        {
            result.bullet_offset = rotation_sprites->right_up_bullet_offset;
            result.rotated_sprite = rotation_sprites->right_up;
        }
        break;
        invalid_default_case;
    }

    return result;
}

void set_entity_rotated_graphics(entity* entity, world_position* target)
{
    if (entity->type->rotation_sprites)
    {
        if (target)
        {
            v2 shooting_direction = get_unit_v2(get_world_position_difference(*target, entity->position));
            shooting_rotation rotation = get_entity_shooting_rotation(entity->type->rotation_sprites, shooting_direction);
            entity->shooting_sprite = rotation.rotated_sprite;
            entity->shooting_sprite.flip_horizontally = rotation.flip_horizontally;
        }
        else
        {
            entity->shooting_sprite = entity->type->rotation_sprites->right;
            if (entity->direction == DIRECTION_W)
            {
                entity->shooting_sprite.flip_horizontally = true;
            }
        }
    }
}

void move_entity_on_path(level_state* level, entity* entity_to_move, tile_position current_start, tile_position current_goal,
    entity* player, r32 delta_time)
{
    if (false == tile_pos_equals(current_goal, current_start))
    {
        v2 distance = get_tile_pos_and_world_position_difference(current_goal, entity_to_move->position);
        r32 distance_length = length_v2(distance);
        v2 distance_to_start = get_tile_pos_and_world_position_difference(current_start, entity_to_move->position);
        r32 distance_to_start_length = length_v2(distance_to_start);

        v2 direction = get_zero_v2();
        if (distance_length != 0)
        {
            direction = get_unit_v2(distance);
        }

        assert(entity_to_move->type->velocity_multiplier != 0.0f);
        r32 velocity = entity_to_move->type->velocity_multiplier;

        r32 slowdown_threshold = 2.0f;
        r32 margin = 0.1f;
        if (distance_length < slowdown_threshold)
        {
            velocity *= ((distance_length + margin) / slowdown_threshold);
        }
        else if (distance_to_start_length < slowdown_threshold)
        {
            velocity *= ((distance_to_start_length + margin) / slowdown_threshold);
        }

        entity_to_move->velocity = scalar_multiply_v2(direction, velocity);
        if (entity_to_move->velocity.x < 0.0f)
        {
            entity_to_move->direction = DIRECTION_W;
        }
        else
        {
            entity_to_move->direction = DIRECTION_E;
        }

        world_position new_position = add_to_world_position(entity_to_move->position, 
            scalar_multiply_v2(scalar_multiply_v2(direction, velocity), delta_time));
        v2 movement_delta = get_world_position_difference(new_position, entity_to_move->position);

        if (has_entity_flags_set(entity_to_move, ENTITY_FLAG_ENEMY))
        {
            // sprawdzenie kolizji z graczem
            chunk_position reference_chunk = get_chunk_pos_from_world_pos(entity_to_move->position);
            collision new_collision = check_minkowski_collision(
                get_entity_collision_data(reference_chunk, entity_to_move),
                get_entity_collision_data(reference_chunk, player),
                movement_delta, 1.0f);

            if (new_collision.collided_wall != DIRECTION_NONE)
            {
                handle_player_and_enemy_collision(level, player, entity_to_move);
            }

            entity_to_move->position = new_position;
        }

        if (has_entity_flags_set(entity_to_move, ENTITY_FLAG_MOVING_PLATFORM_HORIZONTAL)
            || has_entity_flags_set(entity_to_move, ENTITY_FLAG_MOVING_PLATFORM_VERTICAL))
        {
            chunk_position reference_chunk = get_chunk_pos_from_world_pos(entity_to_move->position);
            collision new_collision = check_minkowski_collision(
                get_entity_collision_data(reference_chunk, entity_to_move),
                get_entity_collision_data(reference_chunk, player),
                movement_delta, 1.0f);

            // jeśli platforma jedzie do góry i gracz stoi na niej
            if (new_collision.collided_wall == DIRECTION_N
                && has_entity_flags_set(entity_to_move, ENTITY_FLAG_MOVING_PLATFORM_VERTICAL)
                && movement_delta.y < 0.0f)
            {
                // przesuwamy gracza wyżej
                world_position player_target_position = add_to_world_position(player->position,
                    subtract_v2(movement_delta, get_v2(0.0f, 0.01f)));				

                collision_result collision = move(level, player, player_target_position);
                if (collision.collision_data.collided_wall == DIRECTION_NONE)
                {
                    // jeśli gracz się nie zablokował, możemy jechać
                    entity_to_move->position = add_to_world_position(entity_to_move->position, movement_delta);
                }				
            }
            // jeśli platforma jedzie do dołu i gracz jest pod nią
            else if (new_collision.collided_wall == DIRECTION_S
                && has_entity_flags_set(entity_to_move, ENTITY_FLAG_MOVING_PLATFORM_VERTICAL)
                && movement_delta.y > 0.0f)
            {
                // popychamy gracza do dołu
                world_position player_target_position = add_to_world_position(player->position,
                    add_v2(movement_delta, get_v2(0.0f, 0.01f)));

                collision_result collision = move(level, player, player_target_position);
                if (collision.collision_data.collided_wall == DIRECTION_NONE)
                {
                    entity_to_move->position = add_to_world_position(entity_to_move->position, movement_delta);
                }
                else
                {
                    damage_player(level, 1.0f, true);
                }
            }
            else if (new_collision.collided_wall == DIRECTION_NONE)
            {
                entity_to_move->position = add_to_world_position(entity_to_move->position,
                    scalar_multiply_v2(movement_delta, new_collision.possible_movement_perc));
            }
        }

        if (length_v2(get_tile_pos_and_world_position_difference(current_goal, entity_to_move->position)) < 0.01f)
        {
            if (entity_to_move->goal_path_point == 0)
            {
                entity_to_move->goal_path_point = 1;
            }
            else if (entity_to_move->goal_path_point == 1)
            {
                entity_to_move->goal_path_point = 0;
            }
        }
    }
    else
    {
        entity_to_move->velocity = get_zero_v2();
    }
}

void move_entity_towards_player(level_state* level, entity* entity_to_move, entity* player, r32 delta_time)
{
    assert(entity_to_move->type->velocity_multiplier != 0.0f);
    r32 velocity = entity_to_move->type->velocity_multiplier;

    v2 direction = get_unit_v2(
        get_world_position_difference(player->position, entity_to_move->position));

    entity_to_move->velocity = scalar_multiply_v2(direction, velocity);
    if (entity_to_move->velocity.x < 0.0f)
    {
        entity_to_move->direction = DIRECTION_W;
    }
    else
    {
        entity_to_move->direction = DIRECTION_E;
    }

    world_position new_position = add_to_world_position(entity_to_move->position, 
        scalar_multiply_v2(scalar_multiply_v2(direction, velocity), delta_time));
    v2 movement_delta = get_world_position_difference(new_position, entity_to_move->position);
    entity_to_move->position = new_position;

    if (has_entity_flags_set(entity_to_move, ENTITY_FLAG_ENEMY))
    {
        // sprawdzenie kolizji z graczem
        chunk_position reference_chunk = get_chunk_pos_from_world_pos(entity_to_move->position);
        collision new_collision = check_minkowski_collision(
            get_entity_collision_data(reference_chunk, entity_to_move),
            get_entity_collision_data(reference_chunk, player),
            movement_delta, 1.0f);

        if (new_collision.collided_wall != DIRECTION_NONE)
        {
            handle_player_and_enemy_collision(level, player, entity_to_move);
        }
    }
}

void handle_entity_and_bullet_collision(level_state* level, bullet* moving_bullet, entity* hit_entity)
{
    if (has_entity_flags_set(hit_entity, ENTITY_FLAG_PLAYER))
    {
        start_visual_effect_by_type(level, hit_entity, SPRITE_EFFECT_TYPE_BULLET_HIT);

        damage_player(level, moving_bullet->type->damage_on_contact, false);
    }
    else
    {
        if (false == has_entity_flags_set(hit_entity, ENTITY_FLAG_INDESTRUCTIBLE))
        {
            start_visual_effect_by_type(level, hit_entity, SPRITE_EFFECT_TYPE_BULLET_HIT);

            hit_entity->health -= moving_bullet->type->damage_on_contact;
            
            if (false == hit_entity->player_detected)
            {
                direction previous_direction = hit_entity->direction;
                v2 bullet_direction = get_world_position_difference(moving_bullet->position, hit_entity->position);
                if (bullet_direction.x < 0)
                {
                    hit_entity->direction = DIRECTION_W;
                }
                else
                {
                    hit_entity->direction = DIRECTION_E;
                }

                entity* player = get_player(level);
                if (is_point_visible_for_entity(level, hit_entity, player->position))
                {
                    hit_entity->player_detected = true;					
                }
                else
                {
                    hit_entity->direction = previous_direction;
                }
            }	
        }
    }
}

void enemy_fire_bullet(level_state* level, entity* enemy, entity* target, v2 target_offset)
{
    world_position target_position = add_to_world_position(target->position, target_offset);
    v2 target_relative_pos = get_world_position_difference(target_position, enemy->position);
    v2 direction_to_target = get_unit_v2(target_relative_pos);

    v2 bullet_offset = get_zero_v2();
    if (enemy->type->rotation_sprites)
    {
        shooting_rotation rotation = get_entity_shooting_rotation(enemy->type->rotation_sprites, direction_to_target);
        bullet_offset = rotation.bullet_offset;		
    }
    else
    {
        bullet_offset = (enemy->direction == DIRECTION_E
            ? enemy->type->fired_bullet_offset
            : get_v2(-enemy->type->fired_bullet_offset.x, enemy->type->fired_bullet_offset.y));
    }

    direction_to_target = get_unit_v2(get_world_position_difference(
        target_position, add_to_world_position(enemy->position, bullet_offset)));

    fire_bullet(level, enemy->type->fired_bullet_type, enemy->position, bullet_offset,
        scalar_multiply_v2(direction_to_target, enemy->type->fired_bullet_type->constant_velocity));				
}

void enemy_attack(level_state* level, entity* enemy, entity* player, r32 delta_time)
{
    if (enemy->type->fired_bullet_type)
    {
        world_position player_as_target = add_to_world_position(player->position, 
            level->static_data->player_as_target_offset);

        if (is_point_visible_for_entity(level, enemy, player_as_target))
        {
            if (false == enemy->player_detected)
            {
                // zaczynamy od ataku
                enemy->attack_series_duration = enemy->type->default_attack_series_duration;
            }

            enemy->player_detected = true;

            if (enemy->attack_cooldown > 0.0f)
            {
                // nie robimy nic
                enemy->attack_cooldown -= delta_time;

                if (enemy->attack_cooldown <= 0.0f)
                {
                    enemy->attack_series_duration = enemy->type->default_attack_series_duration;
                }
            }
            else
            {
                if (enemy->attack_series_duration > 0.0f)
                {
                    enemy->attack_series_duration -= delta_time;

                    if (enemy->attack_bullet_interval_duration > 0.0f)
                    {
                        // przerwa, nic nie robimy
                        enemy->attack_bullet_interval_duration -= delta_time;
                    }
                    else
                    {
                        // wystrzeliwujemy jeden pocisk
                        enemy_fire_bullet(level, enemy, player, level->static_data->player_as_target_offset);
                        enemy->attack_bullet_interval_duration =
                            enemy->type->default_attack_bullet_interval_duration;
                    }
                }
                else
                {
                    // przywracamy cooldown
                    enemy->attack_cooldown = enemy->type->default_attack_cooldown;
                }
            }
        }
        else
        {
            enemy->player_detected = false;
            enemy->attack_cooldown = 0.0f;
            enemy->attack_series_duration = 0.0f;
            enemy->attack_bullet_interval_duration = 0.0f;
        }
    }
}

void process_entity_movement(level_state* level, entity* entity_to_move, entity* player, r32 delta_time)
{
    v2 distance_to_player = get_world_position_difference(player->position, entity_to_move->position);
    r32 distance_to_player_length = length_v2(distance_to_player);

    if (false == entity_to_move->has_path)
    {
        if (has_entity_flags_set(entity_to_move, ENTITY_FLAG_WALKS_HORIZONTALLY))
        {
            find_walking_path_for_enemy(level, entity_to_move);
        }

        if (has_entity_flags_set(entity_to_move, ENTITY_FLAG_FLIES_HORIZONTALLY))
        {
            find_flying_path_for_enemy(level, entity_to_move, false);
        }

        if (has_entity_flags_set(entity_to_move, ENTITY_FLAG_FLIES_VERTICALLY))
        {
            find_flying_path_for_enemy(level, entity_to_move, true);
        }

        if (has_entity_flags_set(entity_to_move, ENTITY_FLAG_MOVING_PLATFORM_HORIZONTAL))
        {
            find_path_for_moving_platform(level, entity_to_move, false);
        }

        if (has_entity_flags_set(entity_to_move, ENTITY_FLAG_MOVING_PLATFORM_VERTICAL))
        {
            find_path_for_moving_platform(level, entity_to_move, true);
        }
    }

    if (entity_to_move->has_path)
    {
        tile_position current_goal;
        tile_position current_start;

        if (entity_to_move->goal_path_point == 0)
        {
            current_goal = entity_to_move->path.start;
            current_start = entity_to_move->path.end;
        }
        else if (entity_to_move->goal_path_point == 1)
        {
            current_goal = entity_to_move->path.end;
            current_start = entity_to_move->path.start;
        }
        else
        {
            // wracamy na początek
            entity_to_move->goal_path_point = 0;
            current_goal = entity_to_move->path.start;
            current_start = entity_to_move->path.end;
        }

        if (has_entity_flags_set(entity_to_move, ENTITY_FLAG_ENEMY))
        {
            enemy_attack(level, entity_to_move, player, delta_time);

            // zatrzymywanie się lub podchodzenie w zależności od pozycji gracza				
            if (entity_to_move->player_detected)
            {
                set_entity_rotated_graphics(entity_to_move, &player->position);

                if (distance_to_player_length > entity_to_move->type->forget_detection_distance)
                {
                    entity_to_move->player_detected = false;
                }
                else
                {
                    if (distance_to_player_length < entity_to_move->type->stop_movement_distance)
                    {
                        tile_position entity_position = get_tile_pos_from_world_pos(entity_to_move->position);
                        current_goal = entity_position;
                        current_start = entity_position;
                    }
                    else
                    {
                        tile_position closest_end = get_closest_end_from_tile_range(entity_to_move->path, player->position);
                        current_goal = closest_end;
                    }
                }
            }
            else
            {
                set_entity_rotated_graphics(entity_to_move, NULL);
            }
        }	

        move_entity_on_path(level, entity_to_move, current_start, current_goal, player, delta_time);

        if (length_v2(entity_to_move->velocity) <= 0.5f)
        {
            entity_to_move->current_animation = NULL;
            entity_to_move->animation_duration = 0.0f;
        }
        else
        {
            if (entity_to_move->current_animation != entity_to_move->type->walk_animation)
            {
                entity_to_move->current_animation = entity_to_move->type->walk_animation;
                entity_to_move->animation_duration = 0.0f;
            }
        }	
    }
    else
    {
        if (has_entity_flags_set(entity_to_move, ENTITY_FLAG_FLIES_TOWARDS_PLAYER))
        {
            world_position player_as_target = add_to_world_position(player->position, 
                level->static_data->player_as_target_offset);
            if (is_point_visible_for_entity(level, entity_to_move, player_as_target))
            {
                entity_to_move->player_detected = true;
            }
            else
            {
                entity_to_move->player_detected = false;
            }

            if (entity_to_move->player_detected)
            {
                if (has_entity_flags_set(entity_to_move, ENTITY_FLAG_ENEMY))
                {
                    enemy_attack(level, entity_to_move, player, delta_time);
                }

                set_entity_rotated_graphics(entity_to_move, &player->position);                              

                if (distance_to_player_length > entity_to_move->type->forget_detection_distance)
                {
                    entity_to_move->player_detected = false;
                }
                else
                {
                    move_entity_towards_player(level, entity_to_move, player, delta_time);
                }
            }
            else
            {
                set_entity_rotated_graphics(entity_to_move, NULL);
            }

            if (has_entity_flags_set(entity_to_move, ENTITY_FLAG_ENEMY)
                && distance_to_player_length < 0.05f)
            {
                handle_player_and_enemy_collision(level, player, entity_to_move);
            }
        }
    }
}