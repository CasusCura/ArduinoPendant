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

/* Standard Library */

/* 3rd-Party Library */
#include <Arduino.h>

/* Project Library */
#include "pins.h"

/* Help Button Header */
#include "led_display.h"

/*
 *  Module Macros
 */

#define POWER_FLASH_DELAY_MS 500
#define ERROR_FLASH_DELAY_MS 500
#define REQUEST_FLASH_DELAY_MS 500

/*
 *  Type Defintions
 */

typedef enum {
    LED_POWER_GOOD,
    LED_POWER_LOW
} led_power_mode_t;

typedef enum {
    LED_ERROR_DEACTIVATED,
    LED_ERROR_ACTIVATED
} led_error_mode_t;

typedef enum {
    LED_REQUEST_CLEAR,
    LED_REQUEST_SENT,
    LED_REQUEST_ACK
} led_request_mode_t;

typedef struct {
    led_power_mode_t power_mode;
    time_ms_t next_power_flash;
    pin_mode_t power_state;

    led_error_mode_t error_mode;
    time_ms_t next_error_flash;
    pin_mode_t error_state;

    led_request_mode_t request_mode;
    time_ms_t next_request_flash;
    pin_mode_t request_state;

    time_ms_t last_check;
} led_display_t;

/*
 *  Module Variables.
 */

static led_display_t gDisplay;

/*
 *  Internal Functions.
 */

bool_t need_flash(time_ms_t last_check, time_ms_t flash_time, time_ms_t current_time)
{
    /* No Overflow */
    if (last_check < current_time)
    {
        return (flash_time > last_check && flash_time <= current_time);
    }

    /* Overflow */
    return (flash_time > last_check || flash_time <= current_time);
}

pin_mode_t toggle_led(pin_t pin, pin_mode_t current)
{
    if (current == HIGH)
    {
        digitalWrite(pin, LOW);
        return LOW;
    }
    digitalWrite(pin, HIGH);
    return HIGH;
}


/*
 *  External API
 */

void led_display_setup(void)
{
    pinMode(PINS_POWER_LED, OUTPUT);
    pinMode(PINS_ERROR_LED, OUTPUT);
    pinMode(PINS_REQUEST_LED, OUTPUT);

    digitalWrite(PINS_POWER_LED, HIGH);
    digitalWrite(PINS_ERROR_LED, LOW);
    digitalWrite(PINS_REQUEST_LED, LOW);

    gDisplay.power_state = HIGH;
    gDisplay.error_state = LOW;
    gDisplay.request_state = LOW;

    gDisplay.power_mode = LED_POWER_GOOD;
    gDisplay.error_mode = LED_ERROR_DEACTIVATED;
    gDisplay.request_mode = LED_REQUEST_CLEAR;

    gDisplay.last_check = millis();
}

void led_display_power_low(void)
{
    if (gDisplay.power_mode == LED_POWER_LOW) return;
    gDisplay.power_mode = LED_POWER_LOW;
    gDisplay.next_power_flash = millis() + POWER_FLASH_DELAY_MS;
}

void led_display_power_good(void)
{
    if (gDisplay.power_mode == LED_POWER_GOOD) return;
    gDisplay.power_mode = LED_POWER_GOOD;
}

void led_display_error_activate(void)
{
    if (gDisplay.error_mode == LED_ERROR_ACTIVATED) return;
    gDisplay.error_mode = LED_ERROR_ACTIVATED;
    gDisplay.next_error_flash = millis() + ERROR_FLASH_DELAY_MS;
}

void led_display_error_deactivate(void)
{
    if (gDisplay.error_mode == LED_ERROR_DEACTIVATED) return;
    gDisplay.error_mode = LED_ERROR_DEACTIVATED;
}

void led_display_request_clear(void)
{
    if (gDisplay.request_mode == LED_REQUEST_CLEAR) return;
    gDisplay.request_mode = LED_REQUEST_CLEAR;
}

void led_display_request_sent(void)
{
    if (gDisplay.request_mode == LED_REQUEST_SENT) return;
    gDisplay.request_mode = LED_REQUEST_SENT;
    gDisplay.next_request_flash = millis() + REQUEST_FLASH_DELAY_MS;
}

void led_display_request_acknowledged(void)
{
    if (gDisplay.request_mode == LED_REQUEST_ACK) return;
    gDisplay.request_mode = LED_REQUEST_ACK;
}

void led_display_do_loop(void)
{
    time_ms_t current_time;

    current_time = millis();

    /* Validate Power */
    switch (gDisplay.power_mode)
    {
        case LED_POWER_GOOD:
            if (gDisplay.power_state == LOW)
            {
                digitalWrite(PINS_POWER_LED, HIGH);
                gDisplay.power_state = HIGH;
            }
            break;
        case LED_POWER_LOW:
            if (need_flash(gDisplay.last_check, gDisplay.next_power_flash, current_time))
            {
                gDisplay.power_state = toggle_led(
                    PINS_POWER_LED, gDisplay.power_state);
                gDisplay.next_power_flash = current_time + POWER_FLASH_DELAY_MS;
            }
            break;
    }

    /* Validate Error */
    switch (gDisplay.error_mode)
    {
        case LED_ERROR_ACTIVATED:
            if (need_flash(gDisplay.last_check, gDisplay.next_error_flash, current_time))
            {
                gDisplay.error_state = toggle_led(
                    PINS_ERROR_LED, gDisplay.error_state);
                gDisplay.next_error_flash = current_time + ERROR_FLASH_DELAY_MS;
            }
            break;
        case LED_ERROR_DEACTIVATED:
            if (gDisplay.error_state == HIGH)
            {
                digitalWrite(PINS_ERROR_LED, LOW);
                gDisplay.error_state = LOW;
            }
            break;
    }

    /* Validate Error */
    switch (gDisplay.request_mode)
    {
        case LED_REQUEST_CLEAR:
            if (gDisplay.request_state == HIGH)
            {
                digitalWrite(PINS_REQUEST_LED, LOW);
                gDisplay.request_state = LOW;
            }
            break;
        case LED_REQUEST_SENT:
            if (need_flash(gDisplay.last_check, gDisplay.next_request_flash, current_time))
            {
                gDisplay.request_state = toggle_led(
                    PINS_REQUEST_LED, gDisplay.request_state);
                gDisplay.next_request_flash = current_time + REQUEST_FLASH_DELAY_MS;
            }
            break;
        case LED_REQUEST_ACK:
            if (gDisplay.request_state == LOW)
            {
                digitalWrite(PINS_REQUEST_LED, HIGH);
                gDisplay.request_state = HIGH;
            }
            break;
    }

    gDisplay.last_check = current_time;
}
