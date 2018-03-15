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

#ifndef _LED_HPP_
#define _LED_HPP_

#include "pin.hpp"
#include "led.hpp"

#define LED_FLASH_RATE_MS       500

class Led {
    typedef enum {
        OFF_MODE,
        ON_MODE,
        FLASH_MODE
    } led_mode_t;

    /*
     *  Instance Variables
     */

    Pin * _pin;
    led_mode_t _led_mode;

public:
    Led(Pin * pin);

    void off(void);
    void on(void);
    void flash(void);

    bool_t is_on(void) const;
    bool_t is_off(void) const;
    bool_t is_flashing(void) const;

    void loop(void);
};

#endif /* _LED_HPP_ */
