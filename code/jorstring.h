#if !defined(JORSTRING)

#include "jorutils.h"

struct string_ref
{
	i32 string_size;
	char* ptr;
};

b32 is_empty_string(string_ref);
b32 operator ==(string_ref, string_ref);
char* get_c_string(memory_arena*, string_ref);
b32 compare_to_c_string(string_ref, const char*);
string_ref copy_c_string_to_memory_arena(memory_arena*, const char*, u32);
string_ref c_string_to_string_ref(memory_arena*, const char*, u32);
inline b32 is_whitespace(char);
b32 is_digit(char);
b32 is_all_digits(string_ref);
b32 check_if_string_is_whitespace(string_ref);
string_ref omit_leading_whitespace(string_ref);
string_ref omit_trailing_whitespace(string_ref);
i32 parse_i32(string_ref);
i32 parse_i32(char*, char*);
u32 how_many_digits(u32);
r32 parse_r32(string_ref, char);
i32* parse_array_of_i32(memory_arena*, u32, string_ref, char);
void string_function_test(memory_arena*);

#define JORSTRING
#endif