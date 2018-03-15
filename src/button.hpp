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

#ifndef _BUTTON_HPP_
#define _BUTTON_HPP_

#include "utils.h"
#include "pin.hpp"

class Button {
    typedef enum {
        IDLE_STATE,
        READING_STATE,
        READ_STATE
    } button_state_t;

    /*
     *  Instance Variables
     */
    Pin * _pin;

    button_state_t _button_state;
    uint8_t _hc;
    uint8_t _lc;
    time_ms_t _last_check;
    time_ms_t _next_check;

public:
    Button(Pin * pin);

    bool_t is_pressed(void);

    void loop(void);
};

#endif /* _BUTTON_HPP_ */
