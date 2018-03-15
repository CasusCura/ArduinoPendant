/*
 *  Module: Pin
 *
 *  An abstract Pin object.
 *
 *  Wraps the functionality of a microcontroller pin around an object
 *  to allow for better modularity.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _PIN_HPP_
#define _PIN_HPP_

#include "utils.h"

class Pin {

public:
    typedef enum {
        OUTPUT_MODE,
        INPUT_MODE
    } pin_mode_t;
private:
    pin_t _pin_num;
    struct {
        uint8_t _active_low : 1;
        uint8_t _input : 1;
        uint8_t _active: 1;
    };

public:
    Pin(pin_t pin_num, pin_mode_t pin_mode, bool_t active_low=false);

    operator bool_t(void) const;
    bool_t active(void) const;

    void activate(void);
    void deactivate(void);
    void toggle(void);
};

#endif /* _PIN_HPP_ */
