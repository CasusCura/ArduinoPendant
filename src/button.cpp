/*
 *  Module: Button
 *
 *  An abstract Button / Switch object.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#include <Arduino.h>
#include "button.hpp"

/* Number of consecutive consistance pin reads required. */
#define CONSEC_READ         5
#define DEBOUNCE_DELAY_MS   20


static bool_t time_between(time_ms_t early_time, time_ms_t test_time, time_ms_t later_time)
{
    if (early_time < later_time)
    {
        return (early_time <= test_time) && (test_time <= later_time);
    }
    /* Case of time overflow. */
    return (early_time <= test_time) || (test_time <= later_time);
}

Button::Button(Pin * pin):
    _pin(pin),
    _button_state(IDLE_STATE),
    _hc(0),
    _lc(0),
    _last_check(0),
    _next_check(0) {}


bool_t Button::is_pressed(void)
{
    if (_button_state != READ_STATE) return false;
    _button_state = IDLE_STATE;
    return true;
}

void Button::loop(void)
{
    time_ms_t now;
    switch (_button_state)
    {
        case READ_STATE:
            break;
        case IDLE_STATE:
            if (!_pin->active()) break;
            /* Begin reading / debouncing process. */
            _hc = 1;
            _lc = 0;
            _button_state = READING_STATE;
            _last_check = micros();
            _next_check = _last_check + DEBOUNCE_DELAY_MS;
            break;
        case READING_STATE:
            now = micros();
            /* Check if enough time has passed between checks. */
            if (!time_between(_last_check, _next_check, now)) break;
            /*
             *  Debounce Logic
             */
            if (_pin->active())
            {
                _lc = 0;
                _hc++;
            }
            else
            {
                _hc = 0;
                _lc++;
            }
            if (_hc >= CONSEC_READ)
            {
                _button_state = READ_STATE;
            }
            else if (_lc >= CONSEC_READ)
            {
                _button_state = IDLE_STATE;
            }
            else /* Still debouncing. */
            {
                _last_check = now;
                _next_check = now + DEBOUNCE_DELAY_MS;
            }
            break;
    }
}
