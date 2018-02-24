#include <Arduino.h>

#include "help_button.h"
#include "led_display.h"
#include "utils.h"

#define DEFAULT_SERIAL_BAUD 115200

#define ONE_SECOND          1000
#define FIVE_SECONDS        (ONE_SECOND * 5)

typedef enum {
    MODE_IDLE,
    MODE_ACTIVE
} pendant_mode_t;

typedef enum {
    MODE_ACTIVE_SENT,
    MODE_ACTIVE_ACK
} active_mode_t;


static pendant_mode_t pendant_mode;
static active_mode_t active_mode;

void trigger_active_sent(void);

void trigger_active(void)
{
    pendant_mode = MODE_ACTIVE;
    Serial.println("Active");
    trigger_active_sent();
}

void trigger_idle(void)
{
    pendant_mode = MODE_IDLE;
    Serial.println("Idle");
    led_display_request_clear();
}

void trigger_active_sent(void)
{
    active_mode = MODE_ACTIVE_SENT;
    led_display_request_sent();
    Serial.println("Active Sent");
}

void trigger_active_ack(void)
{
    active_mode = MODE_ACTIVE_ACK;
    led_display_request_acknowledged();
    Serial.println("Active Ack");
}

void do_idle(void)
{
    if (help_button_read())
    {
        trigger_active();
    }
}

void do_active_ack(void)
{

}

void do_active_sent(void)
{
    if (help_button_read())
    {
        trigger_active_ack();
    }
}

void do_active(void)
{
    if (help_button_read_reset())
    {
        trigger_idle();
    }

    switch (active_mode)
    {
        case MODE_ACTIVE_ACK:
            do_active_ack();
            break;
        case MODE_ACTIVE_SENT:
            do_active_sent();
            break;
    }
}

void interface_test_setup()
{
    led_display_setup();
    help_button_setup();
    Serial.begin(DEFAULT_SERIAL_BAUD);
    trigger_idle();
}

void interface_test_loop()
{
    switch (pendant_mode)
    {
        case MODE_IDLE:
            do_idle();
            break;
        case MODE_ACTIVE:
            do_active();
            break;
    }

    led_display_do_loop();
    delay(100);
}
