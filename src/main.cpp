/*
 *  Module: Main Program
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#include <Arduino.h>

#include "interface.hpp"
#include "pin_values.h"
#include "scheduler.h"

#define INTERFACE_LOOP_PERIOD_US    2000

Interface interface(
    PIN_POWER_LED, PIN_ALERT_LED, PIN_ERROR_LED,
    PIN_HELP_BUTTON, PIN_CANCEL_BUTTON);

/*
 *  Interface Loop Task
 *
 *  Called periodically to ensure pins are read and LEDs are
 *  flashed.
 */
uint8_t interface_loop_task(void *)
{
    interface.loop();
    return TASK_EXIT_OK;
}

void setup()
{
    scheduler_init();
    scheduler_periodic_callback(
        TASK_PRIORITY_LOWEST,
        INTERFACE_LOOP_PERIOD_US,
        interface_loop_task,
        NULL);
}

void loop()
{
    // put your main code here, to run repeatedly:
}
