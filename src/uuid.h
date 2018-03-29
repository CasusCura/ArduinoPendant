/*
 *  Module: UUID
 *
 *  A library for the ID types used in this project.
 *  The IDs are based on UUIDs.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2017 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _UUID_H_
#define _UUID_H_

#include "utils.h"

/* 32 Hex Characters, 4 Hyphens, 1 Null Term */
#define UUID_BUFFER_LENGTH 37

START_C_SECTION

typedef char_t uuid_t[UUID_BUFFER_LENGTH];
typedef char_t * uuid_ref_t;
typedef char_t const * uuid_kref_t;

extern kstring_t kZeroUUID;

bool_t uuid_is_uuid(uuid_kref_t uuid);
bool_t uuid_is_zero(uuid_kref_t uuid);

void uuid_set_zero(uuid_ref_t uuid);

END_C_SECTION

#endif /* _UUID_H_ */
