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

#include <string.h>

#include "smlstr.h"
#include "uuid.h"

kstring_t kZeroUUID = "00000000-0000-0000-0000-000000000000";

static bool_t is_hex(char_t c)
{
    return (c >= '0' && c <= '9')
        || (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z');
}

bool_t uuid_is_uuid(uuid_kref_t uuid)
{
    static uint16_t groups[] = {8, 4, 4, 4, 12, 0};
    uint16_t i, g;
    char_t const * ptr;

    /* Check for NULL */
    if (!uuid)
    {
        return false;
    }

    ptr = uuid;
    for (g = 0; groups[g] > 0; g++)
    {
        /* Check for hyphen between groups */
        if (g != 0 && *ptr++ != '-')
        {
            return false;
        }
        /* Check each group is hexidecimal values. */
        for (i = 0; i < groups[g]; i++)
        {
            if (!is_hex(*ptr++))
            {
                return false;
            }
        }
    }

    /* Check that there are no additional characters */
    return !*ptr;
}

bool_t uuid_is_zero(uuid_kref_t uuid)
{
    if (!uuid_is_uuid(uuid)) return false;
    return strncmp(uuid, kZeroUUID, UUID_BUFFER_LENGTH);
}

void uuid_set_zero(uuid_ref_t uuid)
{
    if (!uuid) return;
    smlstrcpy(uuid, kZeroUUID, UUID_BUFFER_LENGTH);
}
