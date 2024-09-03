#pragma once

typedef unsigned char        ui8;
typedef signed char           i8;
typedef unsigned short      ui16;
typedef signed short         i16;
typedef unsigned int        ui32;
typedef signed int           i32;
typedef unsigned long long  ui64;
typedef signed long long     i64;

static_assert(sizeof(ui8) == 1, "FATAL ERROR: Size of ui8 is not 1 byte. Switch compiler.");
static_assert(sizeof(i8) == 1, "FATAL ERROR: Size of i8 is not 1 byte. Switch compiler.");
static_assert(sizeof(ui16) == 2, "FATAL ERROR: Size of ui16 is not 2 bytes. Switch compiler.");
static_assert(sizeof(i16) == 2, "FATAL ERROR: Size of i16 is not 2 bytes. Switch compiler.");
static_assert(sizeof(ui32) == 4, "FATAL ERROR: Size of ui32 is not 4 bytes. Switch compiler.");
static_assert(sizeof(i32) == 4, "FATAL ERROR: Size of i32 is not 4 bytes. Switch compiler.");
static_assert(sizeof(ui64) == 8, "FATAL ERROR: Size of ui64 is not 8 bytes. Switch compiler.");
static_assert(sizeof(i64) == 8, "FATAL ERROR: Size of i64 is not 8 bytes. Switch compiler.");


typedef ui16 relptr_t;