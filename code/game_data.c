#include "game_data.h"
#include "level_parsing.h"
#include "jorutils.h"
#include "entities.h"
#include "rendering.h"
#include "ui.h"

sprite_part get_square_sprite_part(v2 offset, u32 square_side, textures texture, u32 tile_x, u32 tile_y)
{
    sprite_part result = {0};
    result.texture = texture;
    result.texture_rect = get_rect_from_min_corner(
        get_v2(offset.x + (i32)tile_x * square_side,
               offset.y + (i32)tile_y * square_side),
        get_v2(square_side, square_side));
    return result;
}

animation_frame get_square_animation_frame(v2 offset, u32 square_side, memory_arena* arena, textures texture, u32 tile_x, u32 tile_y)
{
    animation_frame result = {0};
    result.sprite.parts_count = 1;
    result.sprite.parts = push_array(arena, result.sprite.parts_count, sprite_part);
    result.sprite.parts[0].texture = texture;
    result.sprite.parts[0].texture_rect = get_rect_from_min_corner(
        get_v2(offset.x + (i32)tile_x * square_side,
               offset.y + (i32)tile_y * square_side),
        get_v2(square_side, square_side));
    return result;
}

sprite_part get_16x16_sprite_part(v2 offset, textures texture, u32 tile_x, u32 tile_y)
{
    sprite_part result = get_square_sprite_part(offset, 16, texture, tile_x, tile_y);
    return result;
}

animation_frame get_16x16_animation_frame(v2 offset, memory_arena* arena, textures texture, u32 tile_x, u32 tile_y)
{
    animation_frame result = get_square_animation_frame(offset, 16, arena, texture, tile_x, tile_y);
    return result;
}

sprite get_square_sprite(memory_arena* arena, u32 square_side, textures texture, u32 tile_x, u32 tile_y, v2 offset)
{
    sprite result = {0};
    result.parts_count = 1;
    result.parts = push_array(arena, result.parts_count, sprite_part);
    result.parts[0] = get_square_sprite_part(get_zero_v2(), square_side, texture, tile_x, tile_y);
    result.parts[0].offset_in_pixels = offset;
    return result;
}

animation_frame get_tile_graphics(memory_arena* arena, u32 tile_value)
{
    animation_frame result = {0};
    result.sprite.parts_count = 1;
    result.sprite.parts = push_array(arena, result.sprite.parts_count, sprite_part);
    result.sprite.parts[0].texture = TEXTURE_TILESET;
    result.sprite.parts[0].texture_rect = get_tile_bitmap_rect(tile_value);
    return result;
}

u32 count_sprite_effects_types()
{
    u32 result = 0;
    for (i32 enum_value = (i32)_FIRST_SPRITE_EFFECT_TYPE;
        enum_value < (i32)_LAST_SPRITE_EFFECT_TYPE;
        enum_value++)
    {
        result++;
    }
    return result;
}

sprite_effect* add_sprite_effect(static_game_data* data, sprite_effects_types type)
{
    i32 index = (i32)type;
    assert(index > 0 && index < data->sprite_effects_count);
    sprite_effect* result = &data->sprite_effects[index];
    assert((i32)result->type == 0); // sprawdzamy, czy typ był nieużywany
    result->type = type;
    return result;
}

void fill_animation_frame(animation* animation, u32 frame_index, u32 part_index, sprite_part part, r32* duration)
{
    animation->frames[frame_index].sprite.parts[part_index] = part;
    if (duration)
    {
        animation->frames[frame_index].duration = *duration;
        animation->total_duration += *duration;
    }
}

sprite_part get_sprite_part_in_direction(textures texture, rect texture_rect, v2 sprite_offset, direction default_direction)
{
    sprite_part result = {0};
    result.texture = texture;
    result.texture_rect = texture_rect;
    result.offset_in_pixels = sprite_offset;
    result.default_direction = default_direction;
    return result;
}

sprite_part get_sprite_part(textures texture, rect texture_rect, v2 sprite_offset)
{
    sprite_part result = get_sprite_part_in_direction(texture, texture_rect, sprite_offset, DIRECTION_E);
    return result;
}

animation* get_walk_animation(memory_arena* arena, v2 bitmap_offset, b32 add_head, v2 display_offset)
{
    animation* new_animation = push_struct(arena, animation);
    new_animation->frames_count = 4;
    new_animation->frames = push_array(arena, new_animation->frames_count, animation_frame);

    r32 frame_duration = 0.2f;
    textures texture = TEXTURE_CHARSET;

    u32 parts_count = add_head ? 2 : 1;

    new_animation->frames[0].sprite.parts_count = parts_count;
    new_animation->frames[1].sprite.parts_count = parts_count;
    new_animation->frames[2].sprite.parts_count = parts_count;
    new_animation->frames[3].sprite.parts_count = parts_count;
    new_animation->frames[0].sprite.parts = push_array(arena, parts_count, sprite_part);
    new_animation->frames[1].sprite.parts = push_array(arena, parts_count, sprite_part);
    new_animation->frames[2].sprite.parts = push_array(arena, parts_count, sprite_part);
    new_animation->frames[3].sprite.parts = push_array(arena, parts_count, sprite_part);

    rect legs_rect = get_rect_from_min_corner(get_v2(bitmap_offset.x + 24, bitmap_offset.y + 24), get_v2(24, 24));
    fill_animation_frame(new_animation, 0, 0, get_sprite_part(texture, legs_rect, display_offset), &frame_duration);
    legs_rect = move_rect(legs_rect, get_v2(24, 0));
    fill_animation_frame(new_animation, 1, 0, get_sprite_part(texture, legs_rect, display_offset), &frame_duration);
    legs_rect = move_rect(legs_rect, get_v2(24, 0));
    fill_animation_frame(new_animation, 2, 0, get_sprite_part(texture, legs_rect, display_offset), &frame_duration);
    legs_rect = move_rect(legs_rect, get_v2(24, 0));
    fill_animation_frame(new_animation, 3, 0, get_sprite_part(texture, legs_rect, display_offset), &frame_duration);

    if (add_head) 
    {
        add_v2_in_place(&new_animation->frames[0].sprite.parts[0].offset_in_pixels, get_v2(0.0f, -5.0f));
        add_v2_in_place(&new_animation->frames[1].sprite.parts[0].offset_in_pixels, get_v2(0.0f, -5.0f));
        add_v2_in_place(&new_animation->frames[2].sprite.parts[0].offset_in_pixels, get_v2(0.0f, -5.0f));
        add_v2_in_place(&new_animation->frames[3].sprite.parts[0].offset_in_pixels, get_v2(0.0f, -5.0f));

        rect head_rect = get_rect_from_min_corner(get_v2(bitmap_offset.x, bitmap_offset.y), get_v2(24, 24));
        v2 head_offset = add_v2(get_v2(5, -19), display_offset);
        fill_animation_frame(new_animation, 0, 1, get_sprite_part(texture, head_rect, head_offset), NULL);	
        fill_animation_frame(new_animation, 1, 1, get_sprite_part(texture, head_rect, head_offset), NULL);
        fill_animation_frame(new_animation, 2, 1, get_sprite_part(texture, head_rect, head_offset), NULL);
        fill_animation_frame(new_animation, 3, 1, get_sprite_part(texture, head_rect, head_offset), NULL);
    }

    return new_animation;
}

animation_frame get_walk_idle_pose(memory_arena* arena, v2 bitmap_offset, b32 add_head, v2 display_offset)
{
    animation_frame result = {0};

    u32 parts_count = add_head ? 2 : 1;

    result.sprite.parts_count = parts_count;
    result.sprite.parts = push_array(arena, parts_count, sprite_part);

    rect legs_rect = get_rect_from_min_corner(get_v2(bitmap_offset.x, bitmap_offset.y + 24), get_v2(24, 24));
    result.sprite.parts[0] = get_sprite_part(TEXTURE_CHARSET, legs_rect, display_offset);

    if (add_head)
    {
        add_v2_in_place(&result.sprite.parts[0].offset_in_pixels, get_v2(0.0f, -5.0f));
        rect head_rect = get_rect_from_min_corner(get_v2(bitmap_offset.x, bitmap_offset.y), get_v2(24, 24));
        v2 head_offset = add_v2(get_v2(5, -19), display_offset);
        result.sprite.parts[1] = get_sprite_part(TEXTURE_CHARSET, head_rect, head_offset);
    }

    return result;
}

animation_frame get_player_pose(memory_arena* arena, i32 index)
{
    animation_frame result = {0};	

    rect legs_rect = get_rect_from_min_corner(get_v2(0, 24), get_v2(24, 24));
    legs_rect = move_rect(legs_rect, get_v2(index * 24, 0));

    result.sprite.parts_count = 1;
    result.sprite.parts = push_array(arena, result.sprite.parts_count, sprite_part);

    result.sprite.parts[0].texture = TEXTURE_CHARSET;
    result.sprite.parts[0].texture_rect = legs_rect;
    result.sprite.parts[0].default_direction = DIRECTION_E;

    return result;
}

animation_frame get_animation_frame_from_sprite(sprite sprite)
{
    animation_frame result = {0};
    result.sprite = sprite;
    return result;
}

animation* get_animation_from_sprite(memory_arena* arena, sprite sprite)
{
    animation* new_animation = push_struct(arena, animation);
    new_animation->frames_count = 1;
    new_animation->frames = push_array(arena, new_animation->frames_count, animation_frame);
    new_animation->frames[0].sprite = sprite;
    new_animation->frames[0].duration = 0.0f;
    return new_animation;
}

animation_frame get_bullet_graphics(memory_arena* arena, u32 x, u32 y)
{
    animation_frame result = {0};

    v2 offset = get_v2(120, 48);

    rect texture_rect = get_rect_from_min_corner(
        get_v2(offset.x + (x * 10), offset.y + (y * 10)), 
        get_v2(10, 10));

    result.sprite.parts_count = 1;
    result.sprite.parts = push_array(arena, result.sprite.parts_count, sprite_part);
    result.sprite.parts[0].texture = TEXTURE_CHARSET;
    result.sprite.parts[0].texture_rect = texture_rect;
    return result;
}

entity_type* add_entity_type(static_game_data* data, entity_type_enum type)
{
    assert(data->entity_types_count < ENTITY_STATIC_TYPES_MAX_COUNT);

    entity_type* result = &data->entity_types[data->entity_types_count];
    data->entity_types_count++;
    
    result->type_enum = type;
    set_entity_type_ptr(data->entity_types_dict, type, result);

    return result;
}

entity_type* add_bullet_type(static_game_data* data)
{
    assert(data->bullet_types_count < BULLET_TYPES_MAX_COUNT);

    entity_type* result = &data->bullet_types[data->bullet_types_count];
    data->bullet_types_count++;

    return result;
}

gate_graphics load_gate_graphics(u32 index)
{
    assert(index < 4);

    v2 offset = get_v2(5 * 24.0f, 4 * 24.0f);
    
    u32 tile_x = 3 + index;
    u32 tile_y = 1;

    gate_graphics result = {0};
    result.frame_upper = get_16x16_sprite_part(offset, TEXTURE_CHARSET, tile_x, tile_y);
    tile_y++;
    result.gate = get_16x16_sprite_part(offset, TEXTURE_CHARSET, tile_x, tile_y);
    tile_y++;
    result.frame_lower = get_16x16_sprite_part(offset, TEXTURE_CHARSET, tile_x, tile_y);

    return result;
}

switch_graphics load_switch_graphics(u32 index)
{
    v2 offset = get_v2(5 * 24.0f, 4 * 24.0f);

    u32 tile_x = 0;
    u32 tile_y = 1 + index;

    switch_graphics result = {0};
    result.frame_left = get_16x16_sprite_part(offset, TEXTURE_CHARSET, tile_x, tile_y);
    tile_x++;
    result.frame_middle = get_16x16_sprite_part(offset, TEXTURE_CHARSET, tile_x, tile_y);
    tile_x++;
    result.frame_right = get_16x16_sprite_part(offset, TEXTURE_CHARSET, tile_x, tile_y);
    return result;
}

display_graphics load_gate_switch_displays()
{
    v2 offset = get_v2(5 * 24.0f, 4 * 24.0f);

    display_graphics result = {0};
    result.switch_left_display = get_16x16_sprite_part(offset, TEXTURE_CHARSET, 0, 0);
    result.switch_middle_display = get_16x16_sprite_part(offset, TEXTURE_CHARSET, 1, 0);
    result.switch_right_display = get_16x16_sprite_part(offset, TEXTURE_CHARSET, 2, 0);
    result.gate_upper_display = get_16x16_sprite_part(offset, TEXTURE_CHARSET, 3, 0);
    result.gate_lower_display = get_16x16_sprite_part(offset, TEXTURE_CHARSET, 4, 0);
    return result;
}

moving_platform_graphics load_moving_platform_graphics(u32 x, u32 y)
{
    v2 offset = get_v2(120.0f, 0.0f);

    moving_platform_graphics result = {0};
    result.left = get_16x16_sprite_part(offset, TEXTURE_CHARSET, 3 * x, y);
    result.middle = get_16x16_sprite_part(offset, TEXTURE_CHARSET, (3 * x) + 1, y);
    result.right = get_16x16_sprite_part(offset, TEXTURE_CHARSET, (3 * x) + 2, y);

    return result;
}

animation_frame load_power_up_graphics(memory_arena* arena, u32 index)
{
    v2 offset = get_v2(5 * 24.0f, 4 * 24.0f);

    u32 tile_x = index;
    u32 tile_y = 5;

    animation_frame result = get_16x16_animation_frame(offset, arena, TEXTURE_CHARSET, tile_x, tile_y);
    return result;
}

ui_graphics load_ui_graphics()
{
    v2 offset = get_v2(192.0f, 48.0f);

    ui_graphics result = {0};

    result.crosshair = get_rect_from_min_corner(get_v2(216, 48), get_v2(13, 13));

    result.healthbar_icon = get_rect_from_min_corner(offset, get_v2(8, 8));
    rect r = get_rect_from_min_corner(add_v2(offset, get_v2(8, 0)), get_v2(4, 8));
    result.healthbar_empty_bar = r;
    result.healthbar_white_bar = move_rect_in_place(&r, get_v2(4, 0));
    result.healthbar_red_bar = move_rect_in_place(&r, get_v2(4, 0));

    r = get_rect_from_min_corner(add_v2(offset, get_v2(0, 8)), get_v2(4, 4));
    result.msgbox_frame_upper_left = r;
    result.msgbox_frame_upper = move_rect_in_place(&r, get_v2(4, 0));
    result.msgbox_frame_upper_right = move_rect_in_place(&r, get_v2(4, 0));
    result.msgbox_frame_right = move_rect_in_place(&r, get_v2(0, 4));
    result.msgbox_frame_lower_right = move_rect_in_place(&r, get_v2(0, 4));
    result.msgbox_frame_lower = move_rect_in_place(&r, get_v2(-4, 0));
    result.msgbox_frame_lower_left = move_rect_in_place(&r, get_v2(-4, 0));
    result.msgbox_frame_left = move_rect_in_place(&r, get_v2(0, -4));
    result.msgbox_frame_background = move_rect_in_place(&r, get_v2(4, 0));

    r = get_rect_from_min_corner(get_v2(192.0f, 68.0f), get_v2(15, 5));
    result.msgbox_dots_1 = r;
    result.msgbox_dots_2 = move_rect_in_place(&r, get_v2(0, 5));
    result.msgbox_dots_3 = move_rect_in_place(&r, get_v2(0, 5));

    return result;
}

animation* load_explosion_animation(memory_arena* arena, v2 offset, u32 tile_side, u32 frames_count, r32 single_frame_duration)
{
    animation* new_animation = push_struct(arena, animation);
    new_animation->frames_count = frames_count;
    new_animation->frames = push_array(arena, new_animation->frames_count, animation_frame);

    textures texture = TEXTURE_EXPLOSION;
    rect frame_rect = get_rect_from_min_corner(offset, get_v2(tile_side, tile_side));

    for (u32 frame_index = 0; frame_index < frames_count; frame_index++)
    {
        new_animation->frames[frame_index].sprite.parts_count = 1;
        new_animation->frames[frame_index].sprite.parts = push_array(arena, 1, sprite_part);
        fill_animation_frame(new_animation, frame_index, 0, get_sprite_part(texture, frame_rect, get_zero_v2()), &single_frame_duration);
        move_rect_in_place(&frame_rect, get_v2(tile_side, 0));
    }

    return new_animation;
}

void add_death_animation(memory_arena* arena, entity_type* type, animation* death_animation)
{
    type->death_animation_variants = push_struct(arena, animation*);
    *type->death_animation_variants = death_animation;
    type->death_animation_variants_count = 1;
}

void add_death_animations(memory_arena* arena, entity_type* type, 
    animation* death_animation_1, animation* death_animation_2, animation* death_animation_3)
{
    type->death_animation_variants = push_array(arena, 3, animation*);
    type->death_animation_variants[0] = death_animation_1;
    type->death_animation_variants[1] = death_animation_2;
    type->death_animation_variants[2] = death_animation_3;
    type->death_animation_variants_count = 3;
}

void add_death_animations_standard(memory_arena* arena, entity_type* type, static_game_data* data)
{
    type->death_animation_variants = push_array(arena, 6, animation*);
    type->death_animation_variants[0] = data->explosion_animations.size_16x16_variant_1;
    type->death_animation_variants[1] = data->explosion_animations.size_16x16_variant_2;
    type->death_animation_variants[2] = data->explosion_animations.size_16x16_variant_3;
    type->death_animation_variants[3] = data->explosion_animations.size_16x16_variant_4;
    type->death_animation_variants[4] = data->explosion_animations.size_16x16_variant_5;
    type->death_animation_variants[5] = data->explosion_animations.size_16x16_variant_6;
    type->death_animation_variants_count = 6;
}

shooting_rotation_sprites* load_shooting_rotation_sprites_with_offset(memory_arena* arena, u32 tile_y, v2 offset_in_pixels)
{
    shooting_rotation_sprites* result = push_struct(arena, shooting_rotation_sprites);
    v2 offset_in_tiles = scalar_divide_v2(offset_in_pixels, TILE_SIDE_IN_PIXELS);

    result->up = get_square_sprite(arena, 24,
        TEXTURE_CHARSET, 4, tile_y, add_v2(get_v2(3.0f, -15.0f), offset_in_pixels));
    result->up_bullet_offset = add_v2(get_v2(0.2f, -1.4f), offset_in_tiles);

    result->right_up = get_square_sprite(arena, 24,
        TEXTURE_CHARSET, 1, tile_y, add_v2(get_v2(5.0f, -15.0f), offset_in_pixels));
    result->right_up_bullet_offset = add_v2(get_v2(0.65f, -1.1f), offset_in_tiles);

    result->right = get_square_sprite(arena, 24,
        TEXTURE_CHARSET, 0, tile_y, add_v2(get_v2(5.0f, -15.0f), offset_in_pixels));
    result->right_bullet_offset = add_v2(get_v2(0.65f, -0.60f), offset_in_tiles);

    result->right_down = get_square_sprite(arena, 24,
        TEXTURE_CHARSET, 2, tile_y, add_v2(get_v2(4.0f, -10.0f), offset_in_pixels));
    result->right_down_bullet_offset = add_v2(get_v2(0.65f, -0.15f), offset_in_tiles);

    result->down = get_square_sprite(arena, 24,
        TEXTURE_CHARSET, 3, tile_y, add_v2(get_v2(2.0f, -6.0f), offset_in_pixels));
    result->down_bullet_offset = add_v2(get_v2(0.35f, 0.15f), offset_in_tiles);

    return result;
}

shooting_rotation_sprites* load_shooting_rotation_sprites(memory_arena* arena, u32 tile_y)
{
    shooting_rotation_sprites* result = push_struct(arena, shooting_rotation_sprites);

    result->up = get_square_sprite(arena, 24, TEXTURE_CHARSET, 4, tile_y, get_zero_v2());
    result->right_up = get_square_sprite(arena, 24, TEXTURE_CHARSET, 1, tile_y, get_zero_v2());
    result->right = get_square_sprite(arena, 24, TEXTURE_CHARSET, 0, tile_y, get_zero_v2());
    result->right_down = get_square_sprite(arena, 24, TEXTURE_CHARSET, 2, tile_y, get_zero_v2());
    result->down = get_square_sprite(arena, 24, TEXTURE_CHARSET, 3, tile_y, get_zero_v2());

    return result;
}

void load_static_game_data(platform_api* platform, static_game_data* data, memory_arena* arena, memory_arena* transient_arena)
{	
    temporary_memory transient_memory = begin_temporary_memory(transient_arena);

    // UI and fonts
    {
        data->ui_gfx = load_ui_graphics();

        data->ui_font = (font){0};
        data->ui_font.height_in_pixels = 8;
        data->ui_font.width_in_pixels = 8;
        data->ui_font.glyph_spacing_width = 1;
        data->ui_font.letter_spacing = -1;
        data->ui_font.line_spacing = 4;
        data->ui_font.texture = TEXTURE_FONT;

        data->title_font = (font){0};
        data->title_font.height_in_pixels = 32;
        data->title_font.width_in_pixels = 24;
        data->title_font.glyph_spacing_width = 0;
        data->title_font.letter_spacing = 0;
        data->title_font.line_spacing = 10;
        data->title_font.texture = TEXTURE_TITLE_FONT;
        data->title_font.uppercase_only = true;

        data->parsing_errors_text_options.font = data->ui_font;
        data->parsing_errors_text_options.allow_horizontal_overflow = false;
        data->parsing_errors_text_options.allow_vertical_overflow = true;
        data->parsing_errors_text_options.writing_area = get_whole_screen_text_area(2.0f);

        data->scrolling_text_options.font = data->ui_font;
        data->scrolling_text_options.allow_horizontal_overflow = false;
        data->scrolling_text_options.allow_vertical_overflow = true;
        data->scrolling_text_options.writing_area = get_whole_screen_text_area(5.0f);
    }
    
    // messages and captions
    {
        data->title_str = copy_c_string(arena, "Trifle Psychotic");

        data->menu_new_game_str = copy_c_string(arena, "New Game");
        data->menu_continue_str = copy_c_string(arena, "Continue");
        data->menu_credits_str = copy_c_string(arena, "Credits");
        data->menu_exit_str = copy_c_string(arena, "Exit");

        data->victory_str = copy_c_string(arena, "Victory");		
        data->checkpoint_str = copy_c_string(arena, "Checkpoint");		
        data->choose_level_message = copy_c_string(arena, "Choose a place to heal:");		
        data->exit_warning_message = copy_c_string(arena, "Are you sure you want to exit? Press ESC again to confirm");

        data->default_death_message = copy_c_string(arena, "You died.");
        data->death_messages_count = 10;
        data->death_messages = push_array(arena, data->death_messages_count, string_ref);
        data->death_messages[0] = copy_c_string(arena, "Death is not an event in life:\nwe do not live to experience death.");
        data->death_messages[1] = copy_c_string(arena, "Next time you die, pay attention.");
        data->death_messages[2] = copy_c_string(arena, "Every cradle is a grave.");
        data->death_messages[3] = copy_c_string(arena, "Sleep is good, death is better; but of course, the best would be never to have been born at all.");
        data->death_messages[4] = copy_c_string(arena, "Only existers suffer harm.");
        data->death_messages[5] = copy_c_string(arena, "Never to have been born is best. But if we must see the light, the next best is quickly returning whence we came.");
        data->death_messages[6] = copy_c_string(arena, "You have successfully avoided old age and sickness.");
        data->death_messages[7] = copy_c_string(arena, "Don't you forget about dying, don't you forget about your friend death.");
        data->death_messages[8] = copy_c_string(arena, "Long live the new flesh.");
        data->death_messages[9] = copy_c_string(arena, "You could write a treatise on the sudden transformation of life into archaeology.");
    }

    // levels in level choice screen
    {
        data->levels_count = 6;
        data->levels = push_array(arena, data->levels_count, level_choice);
        data->levels[0].name = copy_c_string(arena, "1. The Outpost (Intro)");
        data->levels[0].map_name = copy_c_string(arena, "map_01");

        data->levels[1].name = copy_c_string(arena, "2. The Spire");
        data->levels[1].map_name = copy_c_string(arena, "map_02");

        data->levels[2].name = copy_c_string(arena, "3. The Great Armada");
        data->levels[2].map_name = copy_c_string(arena, "map_03");

        data->levels[3].name = copy_c_string(arena, "4. The Orbital Fortress");
        data->levels[3].map_name = copy_c_string(arena, "map_04");

        data->levels[4].name = copy_c_string(arena, "5. The Holy Mountain");
        data->levels[4].map_name = copy_c_string(arena, "map_05");

        data->levels[5].name = copy_c_string(arena, "Custom level");
        data->levels[5].map_name = copy_c_string(arena, "custom");
    }
    
    // special entities graphics
    {
        data->gates_gfx.blue = load_gate_graphics(0);
        data->gates_gfx.grey = load_gate_graphics(1);
        data->gates_gfx.red = load_gate_graphics(2);
        data->gates_gfx.green = load_gate_graphics(3);
        data->switches_gfx.blue = load_switch_graphics(0);
        data->switches_gfx.grey = load_switch_graphics(1);
        data->switches_gfx.red = load_switch_graphics(2);
        data->switches_gfx.green = load_switch_graphics(3);
        data->display_gfx = load_gate_switch_displays();
        data->platforms_gfx.blue = load_moving_platform_graphics(0, 0);
        data->platforms_gfx.grey = load_moving_platform_graphics(0, 1);
        data->platforms_gfx.red = load_moving_platform_graphics(1, 0);
        data->platforms_gfx.green = load_moving_platform_graphics(1, 1);
    }

    // explosion animations
    {
        data->explosion_animations.size_16x16_variant_1 = load_explosion_animation(arena, get_v2(464, 0), 16, 7, 0.1f);
        data->explosion_animations.size_16x16_variant_2 = load_explosion_animation(arena, get_v2(464, 16), 16, 7, 0.1f);
        data->explosion_animations.size_16x16_variant_3 = load_explosion_animation(arena, get_v2(464, 32), 16, 7, 0.1f);
        data->explosion_animations.size_16x16_variant_4 = load_explosion_animation(arena, get_v2(464 + (16 * 7), 0), 16, 7, 0.1f);
        data->explosion_animations.size_16x16_variant_5 = load_explosion_animation(arena, get_v2(464 + (16 * 7), 16), 16, 7, 0.1f);
        data->explosion_animations.size_16x16_variant_6 = load_explosion_animation(arena, get_v2(464 + (16 * 7), 32), 16, 7, 0.1f);
        data->explosion_animations.size_16x16_variant_7_blue = load_explosion_animation(arena, get_v2(464 + (16 * 7) * 2, 32), 16, 7, 0.1f);
        data->explosion_animations.size_16x16_variant_8_blue = load_explosion_animation(arena, get_v2(464 + (16 * 7) * 2, 32), 16, 7, 0.1f);
        data->explosion_animations.size_16x16_variant_9_blue = load_explosion_animation(arena, get_v2(464 + (16 * 7) * 2, 32), 16, 7, 0.1f);
        data->explosion_animations.size_24x24 = load_explosion_animation(arena, get_v2(0, 0), 24, 12, 0.15f);
        data->explosion_animations.size_32x32 = load_explosion_animation(arena, get_v2(0, 24), 32, 12, 0.15f);
        data->explosion_animations.size_48x48 = load_explosion_animation(arena, get_v2(0, 24 + 32), 48, 12, 0.15f);
        data->explosion_animations.size_96x96 = load_explosion_animation(arena, get_v2(0, 24 + 32 + 48), 96, 12, 0.2f);
    }
    
    // other constants
    {		
        data->gravity = get_v2(0, 1.0f);
        data->moving_platform_velocity = 2.0f;

        data->menu_fade_speed = 1.5f;
        data->game_fade_out_speed = 0.5f;
        data->game_fade_in_speed = 1.0f;
        data->game_victory_fade_out_speed = 0.2f;
        data->introduction_fade_speed = 0.5f;
        data->death_screen_fade_speed = 0.5f;
        data->credits_screen_fade_speed = 1.5f;
        
        data->default_time_to_first_menu_interaction = 0.2f;
        data->default_introduction_can_be_skipped_timer = 5.0f;
                
        data->introduction_text_speed = 20.0f;
        data->credits_text_speed = 20.0f;

        string_ref credits_test = copy_c_string(arena,
"Multi line string 1\n\
Multi line string 2\n\
\n\
Multi line string 3\n\
Multi line string 4\n\
\n\
Multi line string 5\n\
Multi line string 6\n\
\n\
Multi line string 7\n\
Multi line string 8\n\
");
        data->credits_text_lines = get_division_of_text_into_lines(
            arena, &data->scrolling_text_options, credits_test);
    }

    // collision data
    {
        read_file_result collision_file = platform->read_file("data/collision_map.tmx");
        data->collision_reference = load_collision_map(arena, transient_arena, collision_file);
        free(collision_file.contents);
    }

    // entity types arrays
    {
        data->entity_types = push_array(arena, ENTITY_STATIC_TYPES_MAX_COUNT, entity_type);
        data->entity_types_count = 0;

        data->bullet_types = push_array(arena, BULLET_TYPES_MAX_COUNT, entity_type);
        data->bullet_types_count = 0;

        data->entity_types_dict = create_entity_types_dictionary(arena);
    }
    
    // player entity type and player constants
    {	
        data->player_jump_acceleration = 33.0f;
        data->player_walking_acceleration = 1.25f;
        data->player_in_air_acceleration = 1.0f;
        
        data->power_up_speed_multipliers = get_v2(1.7f, 1.3f);
        
        data->default_player_invincibility_cooldown = 0.2f;
        data->default_player_ignore_enemy_collision_cooldown = 1.0f;
        data->default_player_recoil_timer = 2.0f;
        data->default_player_recoil_acceleration_timer = 1.0f;

        data->player_idle_pose = get_player_pose(arena, 0);
        data->player_jump_pose = get_player_pose(arena, 3);
        data->player_recoil_pose = get_player_pose(arena, 3);
        data->player_as_target_offset = get_v2(0.0f, -0.3f);

        data->default_player_max_health = 100.0f;

        entity_type* player = add_entity_type(data, ENTITY_TYPE_PLAYER);
        player->flags = (entity_flags)((u32)ENTITY_FLAG_BLOCKS_MOVEMENT | (u32)ENTITY_FLAG_PLAYER);
        player->max_health = data->default_player_max_health;
        player->velocity_multiplier = 40.0f;
        player->slowdown_multiplier = 0.8f;
        player->default_attack_cooldown = 0.2f;
        player->collision_rect_dim = get_v2(0.35f, 1.7f);
        player->collision_rect_offset = get_v2(0.0f, -0.1f);
        player->walk_animation = get_walk_animation(arena, get_zero_v2(), false, get_zero_v2());
        player->rotation_sprites = load_shooting_rotation_sprites_with_offset(arena, 0, get_zero_v2());
        player->death_animation_offset = get_v2(0.0f, -0.75f);
        add_death_animation(arena, player, data->explosion_animations.size_48x48);

        entity_type* player_bullet = add_bullet_type(data);	
        player_bullet->damage_on_contact = 10.0f;
        player_bullet->constant_velocity = 14.0f;
        player_bullet->idle_pose = get_bullet_graphics(arena, 1, 1);
        add_death_animations_standard(arena, player_bullet, data);

        entity_type* power_up_bullet = add_bullet_type(data);
        power_up_bullet->damage_on_contact = 30.0f;
        power_up_bullet->constant_velocity = 18.0f;	
        power_up_bullet->idle_pose = get_bullet_graphics(arena, 3, 1);
        add_death_animations(arena, power_up_bullet,
            data->explosion_animations.size_16x16_variant_7_blue,
            data->explosion_animations.size_16x16_variant_8_blue,
            data->explosion_animations.size_16x16_variant_9_blue);

        data->player_normal_bullet_type = player_bullet;
        data->player_power_up_bullet_type = power_up_bullet;
        player->fired_bullet_type = data->player_normal_bullet_type;
    }

    // enemies entity types
    {
        {
            entity_type* sentry = add_entity_type(data, ENTITY_TYPE_ENEMY_SENTRY);
            sentry->flags = (entity_flags)(
                (u32)ENTITY_FLAG_BLOCKS_MOVEMENT
                | (u32)ENTITY_FLAG_ENEMY
                | (u32)ENTITY_FLAG_DESTRUCTION_ON_PLAYER_CONTACT);
            sentry->max_health = 20.0f;
            sentry->damage_on_contact = 20.0f;
            sentry->velocity_multiplier = 0.0f;
            sentry->detection_type = DETECT_360_DEGREES;
            sentry->detection_distance = 12.0f;
            sentry->stop_movement_distance = 4.0f;
            sentry->forget_detection_distance = 14.0f;
            sentry->default_attack_cooldown = 3.0f;
            sentry->default_attack_series_duration = 0.4f;
            sentry->default_attack_bullet_interval_duration = 0.15f;	
            sentry->collision_rect_dim = get_v2(0.75f, 0.75f);
            sentry->rotation_sprites = load_shooting_rotation_sprites(arena, 6);
            add_death_animation(arena, sentry, data->explosion_animations.size_24x24);

            entity_type* sentry_bullet_type = add_bullet_type(data);
            sentry_bullet_type->flags = ENTITY_FLAG_DAMAGES_PLAYER;
            sentry_bullet_type->damage_on_contact = 10.0f;
            sentry_bullet_type->constant_velocity = 7.0f;
            sentry_bullet_type->idle_pose = get_bullet_graphics(arena, 1, 0);
            add_death_animations_standard(arena, sentry_bullet_type, data);

            sentry->fired_bullet_type = sentry_bullet_type;
        }

        {
            entity_type* big_sentry = add_entity_type(data, ENTITY_TYPE_ENEMY_BIG_SENTRY);
            big_sentry->flags = (entity_flags)(
                (u32)ENTITY_FLAG_BLOCKS_MOVEMENT
                | (u32)ENTITY_FLAG_ENEMY);
            big_sentry->max_health = 120.0f;
            big_sentry->damage_on_contact = 40.0f;
            big_sentry->velocity_multiplier = 0.0f;
            big_sentry->detection_type = DETECT_360_DEGREES;
            big_sentry->detection_distance = 16.0f;
            big_sentry->stop_movement_distance = 0.0f;
            big_sentry->forget_detection_distance = 16.0f;
            big_sentry->default_attack_cooldown = 3.0f;
            big_sentry->default_attack_series_duration = 0.8f;
            big_sentry->default_attack_bullet_interval_duration = 0.025f;			
            big_sentry->collision_rect_dim = get_v2(0.9f, 0.9f);
            big_sentry->idle_pose = get_animation_frame_from_sprite(get_square_sprite(arena, 24, TEXTURE_CHARSET, 3, 7, get_zero_v2()));
            add_death_animation(arena, big_sentry, data->explosion_animations.size_32x32);

            entity_type* big_sentry_bullet = add_bullet_type(data);
            big_sentry_bullet->flags = ENTITY_FLAG_DAMAGES_PLAYER;
            big_sentry_bullet->damage_on_contact = 10.0f;
            big_sentry_bullet->constant_velocity = 7.0f;
            big_sentry_bullet->idle_pose = get_bullet_graphics(arena, 1, 0);
            add_death_animations_standard(arena, big_sentry_bullet, data);

            big_sentry->fired_bullet_type = big_sentry_bullet;
        }

        {
            entity_type* guardian = add_entity_type(data, ENTITY_TYPE_ENEMY_GUARDIAN);
            guardian->flags = (entity_flags)(
                (u32)ENTITY_FLAG_BLOCKS_MOVEMENT
                | (u32)ENTITY_FLAG_ENEMY
                | (u32)ENTITY_FLAG_DESTRUCTION_ON_PLAYER_CONTACT
                | (u32)ENTITY_FLAG_FLIES_VERTICALLY);
            guardian->max_health = 40.0f;
            guardian->damage_on_contact = 30.0f;
            guardian->velocity_multiplier = 3.0f;
            guardian->detection_type = DETECT_360_DEGREES;
            guardian->detection_distance = 10.0f;
            guardian->stop_movement_distance = 4.0f;
            guardian->forget_detection_distance = 12.0f;
            guardian->default_attack_cooldown = 0.5f;
            guardian->default_attack_series_duration = 2.0f;
            guardian->default_attack_bullet_interval_duration = 1.0f;		
            guardian->collision_rect_dim = get_v2(1.0f, 1.0f);
            guardian->idle_pose = get_animation_frame_from_sprite(get_square_sprite(arena, 24, TEXTURE_CHARSET, 1, 7, get_zero_v2()));
            guardian->walk_animation = get_animation_from_sprite(arena, get_square_sprite(arena, 24, TEXTURE_CHARSET, 1, 7, get_zero_v2()));
            add_death_animation(arena, guardian, data->explosion_animations.size_32x32);
            guardian->fired_bullet_offset = get_v2(0.0f, 0.6f);

            entity_type* guardian_bullet = add_bullet_type(data);
            guardian_bullet->flags = ENTITY_FLAG_DAMAGES_PLAYER;
            guardian_bullet->damage_on_contact = 20.0f;
            guardian_bullet->constant_velocity = 4.0f;
            guardian_bullet->idle_pose = get_bullet_graphics(arena, 0, 3);
            add_death_animations(arena, guardian_bullet,
                data->explosion_animations.size_16x16_variant_7_blue,
                data->explosion_animations.size_16x16_variant_8_blue,
                data->explosion_animations.size_16x16_variant_9_blue);

            guardian->fired_bullet_type = guardian_bullet;	
        }

        {
            entity_type* flying_bomb = add_entity_type(data, ENTITY_TYPE_ENEMY_FLYING_BOMB);

            flying_bomb->flags = (entity_flags)(
                (u32)ENTITY_FLAG_BLOCKS_MOVEMENT
                | (u32)ENTITY_FLAG_ENEMY
                | (u32)ENTITY_FLAG_FLIES_TOWARDS_PLAYER
                | (u32)ENTITY_FLAG_DESTRUCTION_ON_PLAYER_CONTACT);
            flying_bomb->max_health = 40.0f;
            flying_bomb->damage_on_contact = 80.0f;
            flying_bomb->velocity_multiplier = 2.0f;
            flying_bomb->detection_type = DETECT_360_DEGREES;
            flying_bomb->detection_distance = 10.0f;
            flying_bomb->forget_detection_distance = 12.0f;
            flying_bomb->collision_rect_dim = get_v2(1.1f, 0.5f);
            flying_bomb->idle_pose = get_animation_frame_from_sprite(get_square_sprite(arena, 24, TEXTURE_CHARSET, 0, 7, get_zero_v2()));
            add_death_animation(arena, flying_bomb, data->explosion_animations.size_96x96);
        }

        {
            entity_type* robot = add_entity_type(data, ENTITY_TYPE_ENEMY_ROBOT);

            robot->flags = (entity_flags)((u32)ENTITY_FLAG_BLOCKS_MOVEMENT
                | (u32)ENTITY_FLAG_WALKS_HORIZONTALLY
                | (u32)ENTITY_FLAG_ENEMY
                | (u32)ENTITY_FLAG_DESTRUCTION_ON_PLAYER_CONTACT);
            robot->max_health = 40.0f;
            robot->damage_on_contact = 30.0f;
            robot->velocity_multiplier = 3.0f;
            robot->detection_type = DETECT_90_DEGREES_IN_FRONT;
            robot->detection_distance = 10.0f;
            robot->stop_movement_distance = 4.0f;
            robot->forget_detection_distance = 15.0f;
            robot->default_attack_cooldown = 1.2f;
            robot->default_attack_series_duration = 0.6f;
            robot->default_attack_bullet_interval_duration = 0.15f;	
            robot->collision_rect_dim = get_v2(0.35f, 1.6f);
            robot->collision_rect_offset = get_v2(0.0f, -0.70f);	
            robot->idle_pose = get_walk_idle_pose(arena, get_v2(0, 2 * 24), true, get_v2(0.0f, 1.0f));
            robot->walk_animation = get_walk_animation(arena, get_v2(0, 2 * 24), true, get_v2(0.0f, 1.0f));
            robot->looking_position_offset = get_v2(0.0f, -1.0f);
            add_death_animation(arena, robot, data->explosion_animations.size_48x48);
            robot->death_animation_offset = get_v2(0.0f, -0.75f);
            robot->fired_bullet_offset = get_v2(0.75f, -0.75f);

            entity_type* robot_bullet = add_bullet_type(data);
            robot_bullet->flags = ENTITY_FLAG_DAMAGES_PLAYER;
            robot_bullet->damage_on_contact = 10.0f;
            robot_bullet->constant_velocity = 9.0f;
            robot_bullet->idle_pose = get_bullet_graphics(arena, 1, 0);
            add_death_animations_standard(arena, robot_bullet, data);

            robot->fired_bullet_type = robot_bullet;
        }

        {
            entity_type* messenger = add_entity_type(data, ENTITY_TYPE_ENEMY_MESSENGER);
            messenger->flags = (entity_flags)((u32)ENTITY_FLAG_BLOCKS_MOVEMENT
                | (u32)ENTITY_FLAG_WALKS_HORIZONTALLY
                | (u32)ENTITY_FLAG_ENEMY
                | (u32)ENTITY_FLAG_PLAYER_RECOIL_ON_CONTACT);
            messenger->max_health = 80.0f;
            messenger->damage_on_contact = 10.0f;
            messenger->velocity_multiplier = 4.0f;
            messenger->player_acceleration_on_collision = 3.0f;
            messenger->detection_type = DETECT_180_DEGREES_IN_FRONT;
            messenger->detection_distance = 14.0f;
            messenger->stop_movement_distance = 0.0f;
            messenger->forget_detection_distance = 18.0f;
            messenger->default_attack_cooldown = 1.0f;
            messenger->default_attack_series_duration = 0.2f;
            messenger->default_attack_bullet_interval_duration = 0.05f;	
            messenger->collision_rect_dim = get_v2(0.4f, 1.7f);
            messenger->collision_rect_offset = get_v2(0.0f, -0.4f);
            messenger->idle_pose = get_walk_idle_pose(arena, get_v2(0, 4 * 24), false, get_v2(0.0f, -4.0f));
            messenger->walk_animation = get_walk_animation(arena, get_v2(0, 4 * 24), false, get_v2(0.0f, -4.0f));
            messenger->rotation_sprites = load_shooting_rotation_sprites_with_offset(arena, 4, get_v2(0, -4));
            add_death_animation(arena, messenger, data->explosion_animations.size_48x48);
            messenger->death_animation_offset = get_v2(0.0f, -0.75f);
            messenger->looking_position_offset = get_v2(0.0f, -1.2f);

            entity_type* messenger_bullet = add_bullet_type(data);
            messenger_bullet->flags = ENTITY_FLAG_DAMAGES_PLAYER;
            messenger_bullet->damage_on_contact = 10.0f;
            messenger_bullet->constant_velocity = 10.0f;
            messenger_bullet->idle_pose = get_bullet_graphics(arena, 3, 0);
            add_death_animations_standard(arena, messenger_bullet, data);

            messenger->fired_bullet_type = messenger_bullet;
        }

        {
            entity_type* yellow_father = add_entity_type(data, ENTITY_TYPE_ENEMY_YELLOW_FATHER);
            yellow_father->flags = (entity_flags)((u32)ENTITY_FLAG_BLOCKS_MOVEMENT
                | (u32)ENTITY_FLAG_FLIES_TOWARDS_PLAYER
                | (u32)ENTITY_FLAG_ENEMY
                | (u32)ENTITY_FLAG_PLAYER_RECOIL_ON_CONTACT);
            yellow_father->max_health = 3000.0f;
            yellow_father->damage_on_contact = 20.0f;
            yellow_father->velocity_multiplier = 3.0f;
            yellow_father->player_acceleration_on_collision = 4.0f;
            yellow_father->detection_type = DETECT_360_DEGREES_IGNORE_WALLS;
            yellow_father->detection_distance = 25.0f;
            yellow_father->stop_movement_distance = 0.0f;
            yellow_father->forget_detection_distance = 25.0f;
            yellow_father->default_attack_cooldown = 0.6f;
            yellow_father->default_attack_series_duration = 0.4f;
            yellow_father->default_attack_bullet_interval_duration = 0.05f;
            yellow_father->collision_rect_dim = get_v2(0.6f, 2.0f);
            yellow_father->collision_rect_offset = get_v2(0.0f, -0.4f);
            yellow_father->idle_pose = get_walk_idle_pose(arena, get_v2(3 * 24, 8 * 24), false, get_v2(0.0f, -4.0f));
            yellow_father->rotation_sprites = load_shooting_rotation_sprites_with_offset(arena, 8, get_v2(0, -4));
            add_death_animation(arena, yellow_father, data->explosion_animations.size_96x96);
            yellow_father->death_animation_offset = get_v2(0.0f, -0.75f);
            yellow_father->looking_position_offset = get_v2(0.0f, -1.2f);

            entity_type* yellow_father_bullet = add_bullet_type(data);
            yellow_father_bullet->flags = ENTITY_FLAG_DAMAGES_PLAYER;
            yellow_father_bullet->damage_on_contact = 15.0f;
            yellow_father_bullet->constant_velocity = 10.0f;
            yellow_father_bullet->idle_pose = get_bullet_graphics(arena, 3, 0);
            add_death_animations_standard(arena, yellow_father_bullet, data);

            yellow_father->fired_bullet_type = yellow_father_bullet;
        }
    }

    // sprite effects
    {
        data->sprite_effects_count = count_sprite_effects_types();
        data->sprite_effects = push_array(arena, data->sprite_effects_count, sprite_effect);

        sprite_effect* death = add_sprite_effect(data, SPRITE_EFFECT_TYPE_DEATH);
        death->period = 0.0f;
        death->amplitude = 1.0f;
        death->duration = 0.0f;
        death->color = get_v4(255, 0, 0, 0);

        sprite_effect* bullet_hit = add_sprite_effect(data, SPRITE_EFFECT_TYPE_BULLET_HIT);
        bullet_hit->period = 1.0f;
        bullet_hit->amplitude = 0.8f;
        bullet_hit->duration = 1.0f;
        bullet_hit->flags = SPRITE_EFFECT_FLAG_ADDITIVE_MODE;
        bullet_hit->color = get_v4(255, 95, 31, 0);

        sprite_effect* invinvibility = add_sprite_effect(data, SPRITE_EFFECT_TYPE_INVINCIBILITY);
        invinvibility->period = 10.0f;
        invinvibility->amplitude = 1.5f;
        invinvibility->flags = (sprite_effect_flags)((u32)SPRITE_EFFECT_FLAG_REPEATS | (u32)SPRITE_EFFECT_FLAG_ADDITIVE_MODE);
        invinvibility->color = get_v4(0, 0, 255, 0);

        sprite_effect* shock = add_sprite_effect(data, SPRITE_EFFECT_TYPE_SHOCK);
        shock->period = 1.5f;
        shock->amplitude = 1.5f;
        shock->duration = 1.5f;
        shock->flags = SPRITE_EFFECT_FLAG_ADDITIVE_MODE;
        shock->color = get_v4(255, 255, 255, 0);

        sprite_effect* recoil = add_sprite_effect(data, SPRITE_EFFECT_TYPE_RECOIL);
        recoil->period = 0.5f;
        recoil->amplitude = 1.5f;
        recoil->flags = (sprite_effect_flags)((u32)SPRITE_EFFECT_FLAG_REPEATS | (u32)SPRITE_EFFECT_FLAG_ADDITIVE_MODE);
        recoil->color = get_v4(255, 255, 255, 0);

        sprite_effect* gate_display_fade = add_sprite_effect(data, SPRITE_EFFECT_TYPE_GATE_DISPLAY_INACTIVE);
        gate_display_fade->amplitude = 1.2f;
        gate_display_fade->color = get_v4(100, 100, 100, 0);

        sprite_effect* speed_up = add_sprite_effect(data, SPRITE_EFFECT_TYPE_SPEED);
        speed_up->amplitude = 1.2f;
        speed_up->flags = (sprite_effect_flags)((u32)SPRITE_EFFECT_FLAG_REPEATS | (u32)SPRITE_EFFECT_FLAG_ADDITIVE_MODE);
        speed_up->color = get_v4(0, 120, 0, 0);
    }
    
    // power ups
    {
        entity_flags power_up_flags = (entity_flags)((u32)ENTITY_FLAG_POWER_UP | (u32)ENTITY_FLAG_INDESTRUCTIBLE);
        v2 power_up_size = get_v2(0.5f, 0.5f);
        entity_type* invincibility = add_entity_type(data, ENTITY_TYPE_POWER_UP_INVINCIBILITY);
        invincibility->idle_pose = load_power_up_graphics(arena, 1);
        invincibility->flags = power_up_flags;
        invincibility->collision_rect_dim = power_up_size;

        entity_type* health = add_entity_type(data, ENTITY_TYPE_POWER_UP_HEALTH);
        health->idle_pose = load_power_up_graphics(arena, 0);
        health->flags = power_up_flags;
        health->collision_rect_dim = power_up_size;

        entity_type* speed = add_entity_type(data, ENTITY_TYPE_POWER_UP_SPEED);
        speed->idle_pose = load_power_up_graphics(arena, 2);
        speed->flags = power_up_flags;
        speed->collision_rect_dim = power_up_size;

        entity_type* damage = add_entity_type(data, ENTITY_TYPE_POWER_UP_DAMAGE);
        damage->idle_pose = load_power_up_graphics(arena, 3);
        damage->flags = power_up_flags;
        damage->collision_rect_dim = power_up_size;

        entity_type* spread = add_entity_type(data, ENTITY_TYPE_POWER_UP_SPREAD);
        spread->idle_pose = load_power_up_graphics(arena, 4);
        spread->flags = power_up_flags;
        spread->collision_rect_dim = power_up_size;

        data->default_power_up_invincibility_timer = 30.0f;
        data->default_power_up_speed_timer = 20.0f;
        data->default_power_up_damage_timer = 30.0f;
        data->default_power_up_spread_timer = 30.0f;
        data->default_power_up_health_bonus = 20.0f;
    }

    end_temporary_memory(transient_memory, true);
}