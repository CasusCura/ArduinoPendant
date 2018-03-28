/*
 *  Module: smlstr (Small String)
 *
 *  A very tiny library for string manipulation
 *  and formating.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2017 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _SMLSTR_H_
#define _SMLSTR_H_

#include "utils.h"

START_C_SECTION

uint16_t smlstrcpy(string_t dest, kstring_t src, uint16_t len);
uint16_t smlstrcat(string_t dest, kstring_t src, uint16_t len);

uint16_t smluintfmt(string_t dest, uint32_t val, uint16_t len);
uint16_t smlintfmt(string_t dest, int32_t val, uint16_t len);

bool_t smlisdec(kstring_t src);
uint32_t smluintscan(kstring_t src);
int32_t smlintscan(kstring_t src);

END_C_SECTION

#endif /* _SMLSTR_H_ */
