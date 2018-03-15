/*
 *  Module: LED
 *
 *  An abstract LED object.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#include <Arduino.h>

#include "led.hpp"

static inline bool_t flash_state(void)
{
    return ((millis() % LED_FLASH_RATE_MS) << 1) >= LED_FLASH_RATE_MS;
}

Led::Led(Pin * pin):
    _pin(pin),
    _led_mode(OFF_MODE)
{
    pin->deactivate();
}

void Led::off(void)
{
    if (_led_mode == OFF_MODE) return;

    _led_mode = OFF_MODE;
    _pin->deactivate();
}

void Led::on(void)
{
    if (_led_mode == ON_MODE) return;

    _led_mode = ON_MODE;
    _pin->activate();
}

void Led::flash(void)
{
    if (_led_mode == FLASH_MODE) return;

    if (flash_state())
    {
        _pin->activate();
    }
    else
    {
        _pin->deactivate();
    }
}

bool_t Led::is_off(void) const
{
    return _led_mode == OFF_MODE;
}

bool_t Led::is_on(void) const
{
    return _led_mode == ON_MODE;
}

bool_t Led::is_flashing(void) const
{
    return _led_mode == FLASH_MODE;
}

void Led::loop(void)
{
    bool_t on;

    if (_led_mode != FLASH_MODE) return;

    on = flash_state();

    if (on && !_pin->active())
    {
        _pin->activate();
    }
    else if (!on && _pin->active())
    {
        _pin->deactivate();
    }
}
