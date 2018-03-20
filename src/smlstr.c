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

#include <ctype.h>
#include "smlstr.h"

static kstring_t kDigits = "0123456789ABCDEF";

#define DEC_LAST_DIGIT(val) (kDigits[(val) % 10])
#define HEX_LAST_DIGIT(val) (kDigits[(val) % 0x10])

static inline uint8_t num_of_dec_digits(uint32_t val)
{
    uint8_t n;
    for (n = 0; val; n++)
    {
        val = val / 10;
    }
    return n ? n : 1; /* 0 needs a digit */
}

#ifdef SMLSTR_HEX_ENABLED

static inline uint8_t num_of_hex_digits(uint32_t val)
{
    uint8_t n;
    for (n = 0; val; n++)
    {
        val = val / 16;
    }
    return n;
}

#endif /* SMLSTR_HEX_ENABLED */


uint8_t smlstrcat(string_t dest, kstring_t src, uint8_t len)
{
    uint8_t required;
    string_t dptr;
    kstring_t sptr;

    required = 0;
    dptr = dest;
    sptr = src;

    if (!dest || !src)
    {
        return 0;
    }

    /* Find end of current string. */
    while (*dptr && required < (len-1))
    {
        dptr++;
        required++;
    }

    /* Concatinate the parts of the string which can fit */
    while (*sptr && len > 0 && required < (len-1))
    {
        *dptr++ = *sptr++;
        required++;
    }

    /* Skip over the rest. */
    while (*sptr)
    {
        sptr++;
        required++;
    }

    /* Add a null terminator if it is possible. */
    if (len > 0)
    {
        *dptr = 0;
    }

    return required;
}


uint8_t smlstrcpy(string_t dest, kstring_t src, uint8_t len)
{
    uint8_t required;
    string_t dptr;
    kstring_t sptr;

    required = 0;
    dptr = dest;
    sptr = src;

    if (!dest || !src)
    {
        return 0;
    }

    /* Copy the parts of the string which can fit */
    while (*sptr && len > 0 && required < (len-1))
    {
        *dptr++ = *sptr++;
        required++;
    }

    /* Skip over the rest. */
    while (*sptr)
    {
        sptr++;
        required++;
    }

    /* Add a null terminator if it is possible. */
    if (len > 0)
    {
        *dptr = 0;
    }

    return required;
}

uint8_t smluintfmt(string_t dest, uint32_t val, uint8_t len)
{
    uint8_t required, i;
    string_t dptr;

    if (!dest)
    {
        return 0;
    }

    required = num_of_dec_digits(val);

    if (len > 0 && required < (len-1))
    {
        /* Buffer is large enough to fit the whole conversion. */
        dptr = &dest[required];
        *dptr-- = 0;
    }
    else if (len > 0)
    {
        /* Buffer not large enough to fit the whole conversion. */
        dptr = &dest[len-1];
        *dptr-- = 0;

        for (i = 0; i <= (required-len); i++)
        {
            val = val / 10;
        }
    }

    while (len > 0 && dest <= dptr)
    {
        *dptr-- = DEC_LAST_DIGIT(val);
        val = val / 10;
    }

    return required;
}

uint8_t smlintfmt(string_t dest, int32_t val, uint8_t len)
{
    uint8_t required;

    if (!dest)
    {
        return 0;
    }

    if (val < 0 && (val-1) < 0)
    {
        required = smluintfmt(dest+1, -val, len ? len-1 : 0);
    }
    else if (val < 0) /* Value is max negative. */
    {
        /*
         *  For max negative, its negative is itself.  For
         *  this case, we get the unsigned format of -(value+1),
         *  then replace the last character with the correct
         *  digit.
         */
        required = smluintfmt(dest+1,  -(val+1), len ? len-1 : 0);

        /*
         * If possible, change the last digit to correct value.
         *  The last digit will be an odd number (max positive
         *  is always odd, b011...111).
         */
        if (len > 0 && required < (len-2))
        {
            /* Shifts digits up. */
            dest[required] = (dest[required] == '9') ? '0': dest[required]+1;
        }
    }
    else /* A positive value. */
    {
        required = smluintfmt(dest, (uint32_t) val, len);
    }


    if (val < 0)
    {
        if (len > 1)
        {
            *dest = '-';
        }
        required++;
    }

    if (len == 1)
    {
        *dest = 0;
    }

    return required;
}

bool_t smlisdec(kstring_t src)
{
    char_t const * ptr;

    if (!src)
    {
        return 0;
    }

    ptr = src;

    if (*ptr == '-') /* Check if negative. */
    {
        ptr++;
    }

    while (*ptr && isdigit(*ptr))
    {
        ptr++;
    }

    return !*ptr;
}

uint32_t smluintscan(kstring_t src)
{
    char_t const * ptr;
    uint32_t value;

    ptr = src;
    value = 0;

    if (!*ptr || *ptr == '-')
    {
        return 0;
    }

    while (*ptr && isdigit(*ptr))
    {
        value *= 10;
        value += (*ptr - '0');
        ptr++;
    }

    if (*ptr)
    {
        return 0;
    }

    return value;
}

int32_t smlintscan(kstring_t src)
{
    char_t const * ptr;
    uint8_t neg;
    int32_t value;

    if (!src)
    {
        return 0;
    }

    neg = (*ptr == '-') ? 1 : 0;
    value = smluintscan(&ptr[neg]);
    value = (value < 0) ? -value : value;
    value = (value < 0) ? 0 : value;
    return neg ? -value : value;
}
