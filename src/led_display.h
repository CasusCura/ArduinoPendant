/*
 *  Module: LED Display
 *
 *  LED Display system for the Pendant.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _LED_DISPLAY_H_
#define _LED_DISPLAY_H_

#include "utils.h"

START_C_SECTION

void led_display_setup(void);

void led_display_power_low(void);
void led_display_power_good(void);

void led_display_error_activate(void);
void led_display_error_deactivate(void);

void led_display_request_clear(void);
void led_display_request_sent(void);
void led_display_request_acknowledged(void);

void led_display_do_loop(void);

END_C_SECTION

#endif /* _LED_DISPLAY_H_ */
