/*
 *  Module: Help Button
 *
 *  Help & reset button interface.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

/* Standard Library */

/* 3rd-Party Library */
#include <Arduino.h>

/* Project Library */
#include "pins.h"

/* Help Button Header */
#include "help_button.h"


#define CONSEC_READ 5


static bool_t read_pin(pin_t pin)
{
    int16_t val, hc, lc;

    lc = hc = 0;
    while (lc < CONSEC_READ && hc < CONSEC_READ)
    {
        val = digitalRead(pin);
        if (val == HIGH)
        {
            lc = 0;
            hc++;
        }
        else
        {
            hc = 0;
            lc++;
        }
    }

    return (lc == CONSEC_READ);
}


void help_button_setup(void)
{
    pinMode(PINS_HELP_BUTTON, INPUT_PULLUP);
    pinMode(PINS_RESET_BUTTON, INPUT_PULLUP);
}

bool_t help_button_read(void)
{
    return read_pin(PINS_HELP_BUTTON);
}

bool_t help_button_read_reset(void)
{
    return read_pin(PINS_RESET_BUTTON);
}
