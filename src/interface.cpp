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

#include "interface.hpp"

Interface::Interface(
        pin_t power_pin_num, pin_t alert_pin_num, pin_t error_pin_num,
        pin_t help_pin_num, pin_t cancel_pin_num):
    _power_led_pin(power_pin_num, Pin::OUTPUT_MODE),
    _alert_led_pin(alert_pin_num, Pin::OUTPUT_MODE),
    _error_led_pin(error_pin_num, Pin::OUTPUT_MODE),
    _help_button_pin(help_pin_num, Pin::INPUT_MODE, true),
    _cancel_button_pin(cancel_pin_num, Pin::INPUT_MODE, true),
    _power_led(&_power_led_pin),
    _alert_led(&_alert_led_pin),
    _error_led(&_error_led_pin),
    _help_button(&_help_button_pin),
    _cancel_button(&_cancel_button_pin) {}

void Interface::power_off(void)
{
    _power_led.off();
}

void Interface::power_on(void)
{
    _power_led.on();
}

void Interface::power_flash(void)
{
    _power_led.flash();
}

void Interface::alert_off(void)
{
    _alert_led.off();
}

void Interface::alert_on(void)
{
    _alert_led.on();
}

void Interface::alert_flash(void)
{
    _alert_led.flash();
}

void Interface::error_off(void)
{
    _error_led.off();
}

void Interface::error_on(void)
{
    _error_led.on();
}

void Interface::error_flash(void)
{
    _error_led.flash();
}

bool_t Interface::is_help_pressed(void)
{
    return _help_button.is_pressed();
}

bool_t Interface::is_cancel_pressed(void)
{
    return _cancel_button.is_pressed();
}

void Interface::loop(void)
{
    _power_led.loop();
    _alert_led.loop();
    _error_led.loop();
    _help_button.loop();
    _cancel_button.loop();
}
