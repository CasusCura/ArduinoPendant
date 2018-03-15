/*
 *  Module: Alert Manager - Unit Test
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#ifdef UNIT_TEST

#include "unity.h"
#include "utils.h"

#include "alertmgr.hpp"

/*
 *  Test Interfaces
 */

class TestIndicator {
    typedef enum {
        LED_OFF,
        LED_ON,
        LED_FLASH
    } led_mode_t;

    led_mode_t _led_mode;
public:
    TestIndicator(): _led_mode(LED_OFF) {}


    void alert_on(void) {
        _led_mode = LED_ON;
    }

    void alert_off(void) {
        _led_mode = LED_OFF;
    }

    void alert_flash(void) {
        _led_mode = LED_FLASH;
    }

    bool_t is_alert_on(void) {
        return (_led_mode == LED_ON);
    }

    bool_t is_alert_off(void) {
        return (_led_mode == LED_OFF);
    }

    bool_t is_alert_flash(void) {
        return (_led_mode == LED_FLASH);
    }

    void reset(void) {
        _led_mode = LED_OFF;
    }
};

class TestMessenger {
    bool_t _requested_help;
    bool_t _cancelled_help;
    uint32_t _cancel_request_id;

public:
    TestMessenger():
        _requested_help(false),
        _cancelled_help(false),
        _cancel_request_id(0) {}

    void request_help(kstring_t message=NULL) {
        _requested_help = true;
    }

    void cancel_help(uint32_t request_id) {
        _cancelled_help == true;
        _cancel_request_id = request_id;
    }

    void reset(void) {
        _requested_help = false;
        _cancelled_help = false;
        _cancel_request_id = 0;
    }

    bool_t is_cancelling(void) {
        return _cancelled_help;
    }

    bool_t is_requesting_help(void) {
        return _requested_help;
    }

    uint32_t get_request_id(void) {
        return _cancel_request_id;
    }
};

/*
 *  Variable Declaration.
 */

typedef AlertManager<TestIndicator, TestMessenger> TestAlertManager;
template<>
TestAlertManager TestAlertManager::s_instance;

static TestIndicator indicator;
static TestMessenger messenger;
TestAlertManager * alert_manager;

/*
 *  Helper Functions
 */

void reset_all(void)
{
    indicator.reset();
    messenger.reset();
    alert_manager->hard_reset();
}

/*
 *  Test Cases
 */

void test_can_enable(void)
{
    reset_all();
    TEST_ASSERT(alert_manager->is_disabled());
    alert_manager->enable();
    TEST_ASSERT(alert_manager->is_enabled());
}

void test_can_enable_disable(void)
{
    reset_all();
    TEST_ASSERT(alert_manager->is_disabled());
    alert_manager->enable();
    TEST_ASSERT(alert_manager->is_enabled());
    alert_manager->disable();
    TEST_ASSERT(alert_manager->is_disabled());
}

void test_active_cycle_acknowledged(void)
{
    uint32_t request_id = 1234;
    reset_all();

    alert_manager->enable();
    TEST_ASSERT(alert_manager->is_enabled());
    TEST_ASSERT(indicator.is_alert_off());

    alert_manager->help_button_push();
    TEST_ASSERT(alert_manager->is_sending());
    TEST_ASSERT(indicator.is_alert_flash());
    TEST_ASSERT(messenger.is_requesting_help());

    alert_manager->help_request_received(request_id);
    TEST_ASSERT(alert_manager->is_sent());
    TEST_ASSERT(indicator.is_alert_flash());
    TEST_ASSERT_EQUAL(alert_manager->get_request_id(), request_id);

    alert_manager->alert_acknowledged();
    TEST_ASSERT(alert_manager->is_acknowledged());
    TEST_ASSERT(indicator.is_alert_on());

    alert_manager->issue_resolved();
    TEST_ASSERT(alert_manager->is_idle());
    TEST_ASSERT(indicator.is_alert_off());
    TEST_ASSERT_EQUAL(alert_manager->get_request_id(), 0);
}

void test_active_cycle_sent_cancelled(void)
{
    uint32_t request_id = 5312;
    reset_all();

    alert_manager->enable();
    TEST_ASSERT(alert_manager->is_enabled());
    TEST_ASSERT(indicator.is_alert_off());

    alert_manager->help_button_push();
    TEST_ASSERT(alert_manager->is_sending());
    TEST_ASSERT(indicator.is_alert_flash());
    TEST_ASSERT(messenger.is_requesting_help());

    alert_manager->help_request_received(request_id);
    TEST_ASSERT(alert_manager->is_sent());
    TEST_ASSERT(indicator.is_alert_flash());
    TEST_ASSERT_EQUAL(alert_manager->get_request_id(), request_id);

    alert_manager->reset_button_push();
    TEST_ASSERT(alert_manager->is_cancelling());
    TEST_ASSERT(indicator.is_alert_flash());
    TEST_ASSERT(messenger.is_cancelling());

    alert_manager->cancel_request_received();
    TEST_ASSERT(alert_manager->is_idle());
    TEST_ASSERT(indicator.is_alert_off());
    TEST_ASSERT_EQUAL(alert_manager->get_request_id(), 0);
}

void test_active_cycle_acknowledged_cancelled(void)
{
    uint32_t request_id = 5312;
    reset_all();

    alert_manager->enable();
    TEST_ASSERT(alert_manager->is_enabled());
    TEST_ASSERT(indicator.is_alert_off());

    alert_manager->help_button_push();
    TEST_ASSERT(alert_manager->is_sending());
    TEST_ASSERT(indicator.is_alert_flash());
    TEST_ASSERT(messenger.is_requesting_help());

    alert_manager->help_request_received(request_id);
    TEST_ASSERT(alert_manager->is_sent());
    TEST_ASSERT(indicator.is_alert_flash());
    TEST_ASSERT_EQUAL(alert_manager->get_request_id(), request_id);

    alert_manager->alert_acknowledged();
    TEST_ASSERT(alert_manager->is_acknowledged());
    TEST_ASSERT(indicator.is_alert_on());

    alert_manager->reset_button_push();
    TEST_ASSERT(alert_manager->is_cancelling());
    TEST_ASSERT(indicator.is_alert_flash());
    TEST_ASSERT(messenger.is_cancelling());

    alert_manager->cancel_request_received();
    TEST_ASSERT(alert_manager->is_idle());
    TEST_ASSERT(indicator.is_alert_off());
    TEST_ASSERT_EQUAL(alert_manager->get_request_id(), 0);
}

int main(int argc, char ** argv)
{
    /* Setup Alert Manager */
    alert_manager = TestAlertManager::get_instance();
    alert_manager->set_indicator_interface(&indicator);
    alert_manager->set_messenger_interface(&messenger);

    UNITY_BEGIN();



    return 0;
}

#endif /* UNIT_TEST */
