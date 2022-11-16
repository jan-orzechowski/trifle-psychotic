#pragma once

#include "jorutils.h"
#include "jormath.h"

typedef enum player_movement_mode
{
    PLAYER_MOVEMENT_MODE_JUMP,
    PLAYER_MOVEMENT_MODE_WALK,
    PLAYER_MOVEMENT_MODE_RECOIL
} player_movement_mode;

typedef struct player_standing_history
{
    b32* buffer;
    u32 current_index;
    u32 buffer_size;
} player_standing_history;

typedef struct player_movement
{
    player_movement_mode current_mode;
    u32 frame_duration; // 0 means the current frame
    player_movement_mode previous_mode;
    u32 previous_mode_frame_duration;

    r32 recoil_timer;
    r32 recoil_acceleration_timer;
    v2 recoil_acceleration;

    player_standing_history standing_history;
} player_movement;

typedef struct power_up_state
{
    r32 time_remaining;
} power_up_state;

typedef union power_ups
{
    struct
    {
        power_up_state invincibility;
        power_up_state speed;
        power_up_state damage;
        power_up_state spread;
    };
    power_up_state states[4];
} power_ups;
