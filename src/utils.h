/*
 *  Module: Project Utilities
 *
 *  A small set of standard definitions to make the rest of the
 *  programing much easier.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2017 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _UTILS_H_
#define _UTILS_H_


/* Determine if Package is Compiled using C or C++ */
#if defined(__cplusplus) || defined(__cplusplus__)
#define _CPLUSPLUS_
#endif

/* Standard Integer Types */
#include <stdint.h>

#ifdef _CPLUSPLUS_ /* If C++ */

#define START_C_SECTION extern "C" {
#define END_C_SECTION };
#define C_FUNCTION extern "C"

#ifndef NULL
#define NULL nullptr
#endif

#else /* C */

#define START_C_SECTION
#define END_C_SECTION
#define C_FUNCTION

#include <stdbool.h>

#ifndef NULL
#define NULL 0
#endif

#endif

/* Project standard byte type. */
typedef uint8_t byte_t;

/* Project Pin Value Type */
typedef int16_t pin_t;
typedef uint8_t pin_mode_t;

/* Project Micro Time Value */
typedef uint32_t time_us_t;
/* Project Milli Time Value */
typedef uint32_t time_ms_t;

typedef char char_t;
typedef bool bool_t;

/* Project String Konstants */
typedef char_t const * kstring_t;

typedef char_t * string_t;

#endif /* _UTILS_H_ */
