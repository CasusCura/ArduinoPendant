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

#include "smlstr.h"
#include "dlog.h"

#define BAUD_RATE 115200
#define PREAMBLE_LENGTH 128
#define LINE_BUF_LENGTH 8


kstring_t kDLogInfo = "[INFO ]";
kstring_t kDLogWarn = "[WARN ]";
kstring_t kDLogError = "[ERROR]";

static bool_t dlog_initialized = false;


C_FUNCTION void _dlog_init(void)
{
    char_t baud_buf[16];
    if (dlog_initialized) return;

    memset(baud_buf, 0, sizeof(baud_buf));
    smluintfmt(baud_buf, BAUD_RATE, sizeof(baud_buf));

    Serial.begin(BAUD_RATE);
    delay(2500);

    DLOG("DLOG Initialized");
    DLOG2("Baud Rate", baud_buf);
    dlog_initialized = true;
}

C_FUNCTION void _dlog(kstring_t file, uint16_t line, kstring_t level, kstring_t message, kstring_t sample)
{
    char_t preamble[PREAMBLE_LENGTH];
    char_t line_buf[LINE_BUF_LENGTH];
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
    smluintfmt(line_buf, line, LINE_BUF_LENGTH);

    iptr = line_buf;
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
