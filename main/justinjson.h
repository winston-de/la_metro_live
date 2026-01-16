// Created by Justin DeWitt

#ifndef JUSTINJSON_INCLUDED
#define JUSTINJSON_INCLUDED

#ifndef TYPES_INCLUDED
#define TYPES_INCLUDED
#include <stdint.h>
#include <stdbool.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#endif TYPES_INCLUDED

#define JSON_ARG(a, b, n, ...) n

// these macros override the functions to make the max_len an optional parameter which defaults to -1 (useful when writing)
#define json_str(...) json_str JSON_ARG(__VA_ARGS__, (__VA_ARGS__), (__VA_ARGS__, -1))
#define json_field(...) json_field JSON_ARG(__VA_ARGS__, (__VA_ARGS__), (__VA_ARGS__, -1))

// used for iterating through object fields / array elements when reading
#define json_obj_iter(...) for(bool _s = json_obj(); _s && (json_field(__VA_ARGS__) || (json_end(), 0));)
#define json_arr_iter() for(bool _s = json_arr(); _s; _s = !json_end())

bool json_write(char *path);
bool json_read(char *text, i32 index);
bool json_read_file(char *path);

bool json_skip(bool all);

// must be called before initiating another json_write / json_read
void json_reset();
bool json_end();
bool json_obj();

// when writing, writes field, returns true (-1 max_len means null terminated)
// when reading, fills name with the field name,
// returns whether field exists
bool json_field(char *name, i32 max_len);
bool json_arr();

// when writing, writes str up to max_len, returns true (-1 max_len means null terminated)
// when reading, copies into str up to max_len,
// returns whether element is string type.
bool json_str(char *str, i32 max_len);
bool json_decimal(double *d);
bool json_int(i32 *i);
bool json_bool(bool *b);
bool json_null();

#endif