﻿#pragma once

#define internal static 
#define local_persist static 
#define global_variable static

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef int32_t b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float r32;
typedef double r64;

typedef size_t memory_index;
typedef uint8_t byte;

#define I32_MAX_VALUE ((i32)0x7FFFFFFF)
#define I64_MAX_VALUE ((i64)0x7FFFFFFFFFFFFFFF)

#define U32_MAX_VALUE (0xFFFFFFFF)
#define U64_MAX_VALUE (0xFFFFFFFFFFFFFFFF)

#define R32_MAX_VALUE (FLT_MAX)

#undef min
#undef max
#define min(arg1, arg2) ((arg1) < (arg2) ? (arg1) : (arg2))
#define max(arg1, arg2) ((arg1) > (arg2) ? (arg1) : (arg2))	
#define min_of_three(arg1, arg2, arg3) (min(arg1, min(arg2, arg3)))
#define max_of_three(arg1, arg2, arg3) (max(arg1, max(arg2, arg3)))

#if TRIFLE_DEBUG
#define assert(expression) if(!(expression)) {*(int *)0 = 0;}
#define invalid_code_path assert(!"Invalid code path!")
#define invalid_default_case default: invalid_code_path; break;
#define debug_breakpoint { int debug = 1; }
#else
#define assert(expression)
#define invalid_code_path 
#define invalid_default_case
#define debug_breakpoint
#endif

#define kilobytes_to_bytes(value) ((value) * 1024LL)
#define megabytes_to_bytes(value) (kilobytes_to_bytes(value) * 1024LL)
#define gigabytes_to_bytes(value) (megabytes_to_bytes(value) * 1024LL)
#define terabytes_to_bytes(value) (gigabytes_to_bytes(value) * 1024LL)

#define array_count(arg) (sizeof(arg) / sizeof((arg)[0]))

#define pi32 3.14159265359f

typedef struct memory_arena
{
    byte* base;
    memory_index size;
    memory_index size_used;

    u32 temporary_memory_stack_frame_count;
} memory_arena;

internal memory_arena* initialize_memory_arena(memory_index size, byte* base)
{
    memory_arena* arena = (memory_arena*)base;
    arena->size = size;
    arena->base = base;
    arena->size_used = sizeof(memory_arena);
    arena->temporary_memory_stack_frame_count = 0;
    return arena;
}

#define push_struct(arena, type) (type *)push_size_impl(arena, sizeof(type))
#define push_array(arena, count, type) (type *)push_size_impl(arena, (count) * sizeof(type))
#define push_size(arena, count) push_size_impl(arena, (count))

internal void* push_size_impl(memory_arena* arena, memory_index size_to_push)
{
    assert((arena->size_used + size_to_push) <= arena->size);

    void* result = arena->base + arena->size_used;
    arena->size_used += size_to_push;
    return result;
}

internal void zero_memory(memory_index size, void* base)
{
    u8* byte = (u8*)base;
    while (size--)
    {
        *byte++ = 0;
    }
}

typedef struct temporary_memory
{
    memory_arena* arena;
    memory_index size_used_at_creation;
} temporary_memory;

internal temporary_memory begin_temporary_memory(memory_arena* arena)
{
    temporary_memory result = {0};

    result.arena = arena;
    result.size_used_at_creation = arena->size_used;

    arena->temporary_memory_stack_frame_count++;

    return result;
}

internal void end_temporary_memory(temporary_memory temp, b32 zero_bytes)
{
    memory_arena* arena = temp.arena;
    assert(arena->temporary_memory_stack_frame_count > 0);
    assert(arena->size_used >= temp.size_used_at_creation);

    if (zero_bytes)
    {
        u32 size_of_temporary_memory = arena->size_used - temp.size_used_at_creation;
        void* temporary_memory_base = arena->base + temp.size_used_at_creation;
        zero_memory(size_of_temporary_memory, temporary_memory_base);
    }

    arena->size_used = temp.size_used_at_creation;
    arena->temporary_memory_stack_frame_count--;
}

internal void check_arena(memory_arena* arena)
{
#if TRIFLE_DEBUG
    assert(arena->temporary_memory_stack_frame_count == 0);
#endif
}

#define zero_struct(some_struct) zero_memory(sizeof(some_struct), some_struct)

#define swap_pointers(a, b, type) { type* temp = a; a = b; b = temp; }
#define swap_values(a, b, type) { type temp = a; a = b; b = temp; }

internal b32 are_flags_set(u32* flags, u32 flag_values_to_check)
{
    b32 result = *flags & flag_values_to_check;
    return result;
}

internal void set_flags(u32* flags, u32 flag_values_to_check)
{
    *flags |= flag_values_to_check;
}

internal void unset_flags(u32* flags, u32 flag_values_to_check)
{
    *flags &= ~flag_values_to_check;
}