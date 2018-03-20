/*
 *  Module: Debug Logger
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#include <Arduino.h>
#include <string.h>

#include "dlog.h"

#define BAUD_RATE 115200
#define PREAMBLE_LENGTH 128


kstring_t kDLogInfo = "[INFO ]";
kstring_t kDLogWarn = "[WARN ]";
kstring_t kDLogError = "[ERROR]";

static bool_t dlog_initialized = false;


C_FUNCTION void _dlog_init(void)
{
    if (dlog_initialized) return;

    Serial.begin(BAUD_RATE);
    DLOG("DLOG Initialized");
    DLOG("Baud Rate: BAUD_RATE");
    dlog_initialized = true;
}

C_FUNCTION void _dlog(kstring_t file, kstring_t line, kstring_t level, kstring_t message, kstring_t sample)
{
    char_t preamble[PREAMBLE_LENGTH];
    string_t optr;
    kstring_t iptr;
    uint16_t len;

    if (!message) return;

    /*
     *  Preamble - [<LEVEL>] <FILE NAME>:<LINE NUMBER>
     */
    memset(preamble, 0, PREAMBLE_LENGTH);
    len = 0;
    optr = preamble;

    /* Log Level */
    iptr = level;
    while (*iptr && (len+1) < PREAMBLE_LENGTH)
    {
        *optr++ = *iptr++;
        len++;
    }
    if ((len+1) < PREAMBLE_LENGTH)
    {
        *optr++ = ' ';
        len++;
    }

    /* File Name */
    iptr = file;
    while (*iptr && (len+1) < PREAMBLE_LENGTH)
    {
        *optr++ = *iptr++;
        len++;
    }
    if ((len+1) < PREAMBLE_LENGTH)
    {
        *optr++ = ':';
        len++;
    }

    /* Line Number */
    iptr = line;
    while (*iptr && (len+1) < PREAMBLE_LENGTH)
    {
        *optr++ = *iptr++;
        len++;
    }
    if ((len+1) < PREAMBLE_LENGTH)
    {
        *optr++ = ' ';
        len++;
    }

    Serial.print(preamble);
    Serial.flush();

    /*
     *  Main Content.
     */


    Serial.print(message);
    Serial.flush();

    if (sample)
    {
        Serial.print(" - ");
        Serial.print(sample);
    }

    Serial.println("");
    Serial.flush();
}
