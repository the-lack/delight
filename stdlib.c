#include <stdio.h>

typedef __INT64_TYPE__   int64;  // todo: endianness
typedef __UINT64_TYPE__  uint64; // todo: endianness

void delight_print(char* string_ptr, uint64 character_count) {
    printf("%.*s uuh", (int)character_count, string_ptr);
}
