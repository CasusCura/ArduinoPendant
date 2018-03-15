/*
 *  Module: Interface
 *
 *  Pendant device interface.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _INTERFACE_HPP_
#define _INTERFACE_HPP_

#include "button.hpp"
#include "led.hpp"
#include "pin.hpp"
#include "utils.h"

class Interface {
    /*
     *  Instance Variables
     */
    /* Pins */
    Pin _power_led_pin;
    Pin _alert_led_pin;
    Pin _error_led_pin;

    Pin _help_button_pin;
    Pin _cancel_button_pin;

    /* LEDs */
    Led _power_led;
    Led _alert_led;
    Led _error_led;

    /* Buttons */
    Button _help_button;
    Button _cancel_button;

public:
    Interface(
        pin_t power_pin_num, pin_t alert_pin_num, pin_t error_pin_num,
        pin_t help_pin_num, pin_t cancel_pin_num);

    void power_off(void);
    void power_on(void);
    void power_flash(void);

    void alert_off(void);
    void alert_on(void);
    void alert_flash(void);

    void error_off(void);
    void error_on(void);
    void error_flash(void);

    bool_t is_help_pressed(void);
    bool_t is_cancel_pressed(void);

    void loop(void);
};

#endif /* _INTERFACE_HPP_ */
