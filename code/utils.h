#pragma once

#define assert(statement) {if (!(statement)) { int x = 1; x = x / 0; }}
#define invalid_code_path { int x = 1; x = x / 0; }