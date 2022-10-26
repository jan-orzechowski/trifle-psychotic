#pragma once

#include "jorutils.h"
#include "jormath.h"

enum class player_movement_mode
{
	JUMP,
	WALK,
	RECOIL
};

struct player_standing_history
{
	b32* buffer;
	u32 current_index;
	u32 buffer_size;
};

struct player_movement
{
	player_movement_mode current_mode;
	u32 frame_duration; // 0 oznacza bieżącą klatkę
	player_movement_mode previous_mode;
	u32 previous_mode_frame_duration;

	r32 recoil_timer;
	r32 recoil_acceleration_timer;
	v2 recoil_acceleration;

	player_standing_history standing_history;
};

struct power_up_state
{
	r32 time_remaining;
};

union power_ups
{
	struct
	{
		power_up_state invincibility;
		power_up_state speed;
		power_up_state damage;
		power_up_state spread;
	};
	power_up_state states[4];
};
