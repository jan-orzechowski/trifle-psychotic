﻿#if !defined(JORSTRING)

#include "jorutils.h"
#include "jormath.h"
#include <SDL.h>

struct string_ref
{
	i32 string_size;
	char* ptr;
};

b32 is_empty_string(string_ref str)
{
	b32 result = (str.ptr == 0 || str.string_size == 0);
	return result;
}

b32 operator ==(string_ref a, string_ref b)
{
	b32 result = true;

	if (is_empty_string(a) || is_empty_string(b))
	{
		invalid_code_path;
		result = false;
	}

	if (a.string_size != b.string_size)
	{
		result = false;
	}
	else
	{
		for (u32 char_index = 0;
			char_index < a.string_size;
			char_index++)
		{
			char a_char = *(a.ptr + char_index);
			char b_char = *(b.ptr + char_index);
			if (a_char != b_char)
			{
				result = false;
				break;
			}
		}
	}
	return result;
}

char* get_c_string(memory_arena* arena, string_ref str)
{
	char* new_c_str = (char*)push_size(arena, str.string_size + 1);
	{
		char* new_c_str_char = new_c_str;
		for (u32 char_index = 0; char_index < str.string_size; )
		{
			*new_c_str_char = *(str.ptr + char_index);
			new_c_str_char++;
			char_index++;
		}
		*(new_c_str + str.string_size) = '\0';
	}
	return new_c_str;
}

b32 compare_to_c_string(string_ref my_str, const char* c_str)
{
	b32 result = true;

	if (my_str.ptr == NULL || c_str == NULL)
	{
		result = false;
		invalid_code_path;
	}

	u32 my_str_char_index = 0;
	while (*c_str)
	{
		if (my_str_char_index == my_str.string_size)
		{
			// stringi są różnej długości - przeszliśmy my_str, a nie przeszliśmy c_str
			result = false;
			break;
		}

		char my_char = *(my_str.ptr + my_str_char_index);
		char c_char = *c_str;
		if (my_char != c_char)
		{
			result = false;
			break;
		}

		my_str_char_index++;
		c_str++;
	}

	if (my_str_char_index < my_str.string_size)
	{
		// stringi są różnej długości - przeszliśmy c_str, a nie przeszliśmy my_str
		result = false;
	}

	return result;
}

string_ref copy_c_string_to_memory_arena(memory_arena* arena, const char* str, u32 str_size)
{
	string_ref result = {};
	result.string_size = str_size;
	result.ptr = (char*)push_size(arena, str_size + 1);

	for (u32 index = 0;
		index < str_size;
		index++)
	{
		*(result.ptr + index) = *(str + index);
	}
	*(result.ptr + str_size) = '\0';

	return result;
}

inline b32 is_whitespace(char c)
{
	b32 result = (c == ' ' || c == '\r' || c == '\t' || c == '\n');
	return result;
}

b32 is_digit(char c)
{
	b32 result = (c >= '0' && c <= '9');
	return result;
}

b32 check_if_string_is_whitespace(string_ref str)
{
	b32 result = true;
	for (u32 char_index = 0;
		char_index < str.string_size;
		char_index++)
	{
		if (false == is_whitespace(*(str.ptr + char_index)))
		{
			result = false;
			break;
		}
	}

	return result;
}

string_ref omit_leading_whitespace(string_ref str)
{
	string_ref result = str;
	if (false == is_empty_string(str))
	{
		char* new_start = 0;
		for (u32 char_index = 0;
			char_index < str.string_size;
			char_index++)
		{
			char* c = (str.ptr + char_index);
			if (is_whitespace(*c))
			{
				new_start = c + 1;
			}
		}

		if (new_start != 0)
		{
			if (new_start >= str.ptr + str.string_size)
			{
				result.ptr = 0;
				result.string_size = 0;
			}
			else
			{
				char* old_end = str.ptr + str.string_size - 1;
				u32 new_size = old_end - new_start;
				result.ptr = new_start;
				result.string_size = new_size;
			}
		}
	}
	return result;
}

i32 parse_i32(string_ref str)
{
	i32 result = 0;

	str = omit_leading_whitespace(str);
	if (str.string_size > 0)
	{
		i32 digits_count = str.string_size;
		if (*(str.ptr + str.string_size) = '\0')
		{
			digits_count--;
		}
		i32 decimal_place_multiplier = power(10, digits_count - 1);

		for (u32 char_index = 0; char_index < str.string_size; char_index++)
		{
			u8 digit = 0;
			char c = *(str.ptr + char_index);
			switch (c)
			{
				case '0': digit = 0; break;
				case '1': digit = 1; break;
				case '2': digit = 2; break;
				case '3': digit = 3; break;
				case '4': digit = 4; break;
				case '5': digit = 5; break;
				case '6': digit = 6; break;
				case '7': digit = 7; break;
				case '8': digit = 8; break;
				case '9': digit = 9; break;
				case '\0': digit = 0; break;
				default: goto function_parse_i32_end;
			}
			result += digit * decimal_place_multiplier;
			decimal_place_multiplier = decimal_place_multiplier / 10;
		}
	}

function_parse_i32_end:
	return result;
}

i32 parse_i32(char* start, char* end)
{
	i32 result = 0;
	string_ref str = {};
	str.ptr = start;
	str.string_size = end - start;
	if (start && end && str.string_size > 0)
	{
		result = parse_i32(str);
	}
	return result;
}

r64 parse_r64(string_ref str)
{
	r64 result = 0;
	if (str.ptr)
	{
		char* next_after_last_char = str.ptr + str.string_size;
		result = SDL_strtod((char*)str.ptr, &next_after_last_char);
	}
	return result;
}

i32* parse_array_of_i32(memory_arena* arena, u32 array_length, string_ref str, char delimiter)
{
	i32* arr = push_array(arena, array_length, i32);
	u32 current_int_index = 0;
	char* start = 0;
	char* end = 0;
	for (u32 char_index = 0;
		char_index < str.string_size;
		char_index++)
	{
		char* c = str.ptr + char_index;
		if (*c == delimiter)
		{
			end = c;
			if (start)
			{
				i32 new_int = (i32)parse_i32(start, end);
				if (current_int_index < array_length)
				{
					arr[current_int_index++] = new_int;
					start = 0;
					end = 0;
				}
				else
				{
					// błąd - wyczerpaliśmy miejsce, a jest jeszcze string do parsowania
					break;
				}
			}
			else
			{
				// przypadek gdy mamy podwójny delimiter bądź plik zaczął się od delimitera
				end = 0;
				continue;
			}
		}
		else if (is_digit(*c))
		{
			if (start == 0)
			{
				// zaczyna się nowa liczba
				start = c;
			}
			else
			{
				// mamy starą liczbę
				continue;
			}
		}
		else if (is_whitespace(*c))
		{
			if (start != 0)
			{
				// traktujemy jako delimiter
				end = c;
				i32 new_int = (i32)parse_i32(start, end);
				if (current_int_index < array_length)
				{
					arr[current_int_index++] = new_int;
					start = 0;
					end = 0;
				}
				else
				{
					// błąd - wyczerpaliśmy miejsce, a jest jeszcze string do parsowania
					break;
				}
			}
			else
			{
				// po prostu idziemy naprzód
				continue;
			}
		}
		else
		{
			// mamy błąd - napotkaliśmy coś, co nie jest ani liczbą, ani delimiterem...
			continue;
		}
	}
	return arr;
}

#define JORSTRING
#endif