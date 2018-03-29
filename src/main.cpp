/*
 *  Module: Main Program
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#include <Arduino.h>

#include "alertmgr.hpp"
#include "dlog.h"
#include "interface.hpp"
#include "fake_messenger.hpp"
#include "messenger.hpp"
#include "pin_values.h"
#include "scheduler.h"
#include "wifi_driver.h"

#define INTERFACE_LOOP_PERIOD_US    20000
#define MANAGER_LOOP_PERIOD_US      200000



Interface interface(
    PIN_POWER_LED, PIN_ALERT_LED, PIN_ERROR_LED,
    PIN_HELP_BUTTON, PIN_CANCEL_BUTTON);

typedef AlertManager<Interface, Messenger> Manager;

template<>
Manager Manager::s_instance = Manager();

Manager * manager;

/*
 *  Interface Loop Task
 *
 *  Called periodically to ensure pins are read and LEDs are
 *  flashed.
 */
uint8_t interface_loop_task(void *)
{
    static uint32_t counter = 0;
    if (++counter >= 250)
    {
        DLOG("Interface Loop...");
        counter = 0;
    }
    interface.loop();
    return TASK_EXIT_OK;
}

/*
 *  Alert Manager Loop Task
 *
 *  Called periodically to run the alert manager cycle.
 */
uint8_t manager_loop_task(void *)
{
    static bool_t has_printed = false;
    static uint32_t counter = 0;
    interface.loop();
    // if (!wifi_driver_is_connected())
    if (false)  /* For testing purposes */
    {
        DLOG("WiFi Connection Lost");
        manager->wifi_connection_lost();
    }
    else if (manager->is_disconnected())
    {
        DLOG("Wifi Connection Restored");
        manager->wifi_connection_restored();
    }

    if (wifi_driver_is_connected() && !has_printed)
    {
        wifi_driver_log_status();
        has_printed = true;
    }

    if (interface.is_help_pressed())
    {
        DLOG("Help Button Pressed");
        manager->help_button_push();
    }
    else if (interface.is_cancel_pressed())
    {
        DLOG("Cancel Button Pressed");
        manager->reset_button_push();
    }

    if (manager->is_sending())
    {
        DLOG("Trying to Send Help Request");
        manager->try_send();
        if (manager->is_sending())
        {
            counter++;

            if (counter == 4)
            {
                DLOG("Hard Reset of Manager");
                manager->hard_reset();
                manager->enable();
                counter = 0;
            }
        }
        else
        {
            DLOG("Done Send");
            counter = 0;
        }
    }
    else if (manager->is_cancelling())
    {
        DLOG("Trying to Cancel");
        manager->try_cancel();
    }

    return TASK_EXIT_OK;
}

void setup()
{
    DLOG_INIT();
    scheduler_init();
    // scheduler_periodic_callback(
    //     TASK_PRIORITY_LOWEST,
    //     INTERFACE_LOOP_PERIOD_US,
    //     interface_loop_task,
    //     NULL);
    scheduler_periodic_callback(
        TASK_PRIORITY_LOWEST,
        MANAGER_LOOP_PERIOD_US,
        manager_loop_task,
        NULL);
    wifi_driver_init();

    /* AlertManager Setup */
    manager = Manager::get_instance();
    DLOG("Setting manager interface");
    manager->set_messenger_interface(Manager::messenger_t::get_instance());
    DLOG("Setting indicator interface");
    manager->set_indicator_interface(&interface);
    DLOG("Enabling Manager");
    manager->enable();
}

void messenger_test_loop(void)
{
    static bool_t sent = false;
    static Messenger * msgr = NULL;

    if (!msgr)
    {
        msgr = Messenger::get_instance();
    }

    if (!wifi_driver_is_connected())
    {
        DLOG_ERR("WiFi down");
        delay(2500);
        wifi_driver_connect();
        delay(2500);
        return;
    }

    if (sent)
    {
        DLOG("Done loop...");
        delay(10000);
    }

    delay(1000);
    DLOG("Testing service connection and authorization");
    msgr->test();
    sent = true;
}

void loop()
{
    // put your main code here, to run repeatedly:
    // messenger_test_loop();
    scheduler_loop();
}
