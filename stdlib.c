typedef __INT64_TYPE__   int64;  // todo: endianness
typedef __UINT64_TYPE__  uint64; // todo: endianness


void delight_print(const void* string_ptr, uint64 character_count) {
    printf("%.*s", (int)character_count, (const char*)string_ptr);
}
