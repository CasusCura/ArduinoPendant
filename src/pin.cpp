/*
 *  Module: Pin
 *
 *  An abstract Pin object.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#include <Arduino.h>

#include "pin.hpp"

Pin::Pin(pin_t pin_num, pin_mode_t pin_mode, bool_t active_low):
    _pin_num(pin_num),
    _input((pin_mode == INPUT_MODE)?1:0),
    _active_low((active_low)?1:0)
{
    pinMode(pin_num, _input ? INPUT : OUTPUT);
    deactivate();
}

Pin::operator bool_t(void) const
{
    return active();
}

bool_t Pin::active(void) const
{
    uint8_t val;
    bool_t high;
    if (_input)
    {
        val = digitalRead(_pin_num);
        high = (val == HIGH ? true : false);
        return (high && !_active_low) || (!high && _active_low);
    }
    else
    {
        return _active;
    }
}

void Pin::activate(void)
{
    if (_input) return;
    digitalWrite(_pin_num, (_active_low) ? LOW : HIGH);
    _active = 1;
}

void Pin::deactivate(void)
{
    if (_input) return;
    digitalWrite(_pin_num, (_active_low) ? HIGH : LOW);
    _active = 0;
}

void Pin::toggle(void)
{
    if (_input) return;
    if (_active)
    {
        deactivate();
    }
    else
    {
        activate();
    }
}
