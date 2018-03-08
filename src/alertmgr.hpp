/*
 *  Module: Alert Manager
 *
 *  Contains the main system logic for the Patient Pendant.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _ALERT_MANAGER_H_
#define _ALERT_MANAGER_H_

#include "utils.h"

/*
 *  Expected Indicator Interface
 *      void alert_on(void);
 *      void alert_off(void);
 *      void alert_flash(void);
 *
 *  Expected Messenger Interface
 *      void request_help(kstring_t message)
 */


template<class Indicator, class Messenger>
class AlertManager {
    /*
     *  State Type Definitions
     */
    typedef enum {
        MODE_DISABLED,
        MODE_ENABLED,
        MODE_DISCONNECTED
    } mode_t;

    typedef enum {
        ENABLED_MODE_NONE,
        ENABLED_MODE_IDLE,
        ENABLED_MODE_ACTIVE
    } enabled_mode_t;

    typedef enum {
        ENABLED_ACTIVE_MODE_NONE,
        ENABLED_ACTIVE_MODE_SENT,
        ENABLED_ACTIVE_MODE_WAIT
    } enabled_active_mode_t;

    /* Singleton Instance */
    static AlertManager s_instance;

    /*
     *  Instance Vaiables
     */

    Indicator *_indicator;
    Messenger *_messenger;

    mode_t _mode;
    enabled_mode_t _enabled_mode;
    enabled_active_mode_t _enabled_active_mode;

    uint32_t _request_id;

    /*
     *  Constructor
     */

    AlertManager():
        _indicator(Indicator::get_instance()),
        _messenger(Messenger::get_instance()),
        _mode(MODE_DISABLED),
        _enabled_mode(ENABLED_MODE_IDLE),
        _enabled_active_mode(ENABLED_ACTIVE_MODE_SENT),
        _request_id(0)
    {
        _indicator->alert_off();
    }

    /*
     *  State Transition Handlers
     */

    void do_mode_on_exit(mode_t mode)
    {
        switch (mode)
        {
            case MODE_DISABLED:
                break;
            case MODE_ENABLED:
                do_enabled_mode_on_exit(_enabled_mode);
                break;
            case MODE_DISCONNECTED:
                break;
        }
    }

    void do_mode_on_enter(mode_t mode)
    {
        switch (mode)
        {
            case MODE_DISABLED:
                break;
            case MODE_ENABLED:
                do_enabled_mode_on_enter(_enabled_mode);
                break;
            case MODE_DISCONNECTED:
                break;
        }
    }

    void do_enabled_mode_on_exit(enabled_mode_t enabled_mode)
    {
        switch (enabled_mode)
        {
            case ENABLED_MODE_IDLE:
                break;
            case ENABLED_MODE_ACTIVE:
                break;
        }
    }

    void set_mode(mode_t mode)
    {
        /* On Exit */
        do_mode_on_exit(_mode);

        /* Transition */
        _mode = mode;

        /* On Enter */
        do_mode_on_enter(_mode);
    }

    void set_enabled_mode(enabled_mode_t enabled_mode)
    {
        if (_mode != MODE_ENABLED)
        {
            set_mode(MODE_ENABLED);
        }
        else
        {
            /* On Exit */
            do_enabled_mode_on_exit(_enabled_mode);
        }

        _enabled_mode = enabled_mode;

        /* On Enter */
        do_enabled_mode_on_enter(_enabled_mode);
    }

    void set_enabled_active_mode(enabled_active_mode_t enabled_active_mode)
    {
        if (_enabled_mode != ENABLED_MODE_ACTIVE)
        {

        }
    }

    void trigger_enabled(void)
    {

    }

public:
    AlertManager * get_instance();

    /* Mode Getters */

    bool_t is_enabled(void)
    {
        return (_mode == MODE_ENABLED);
    }

    bool_t is_disabled(void)
    {
        return (_mode == MODE_DISABLED);
    }

    bool_t is_connected(void)
    {
        return (_mode != MODE_DISCONNECTED) && (_mode != MODE_DISABLED);
    }


    /* Event Triggers */

    void enable(void)
    {
        if (!is_disabled()) return;

    }

    void help_button_push(void)
    {

    }



};

#endif /* _ALERT_MANAGER_H_ */