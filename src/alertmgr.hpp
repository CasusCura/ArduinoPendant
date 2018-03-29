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

#include <string.h>

#include "dlog.h"
#include "konstants.h"
#include "uuid.h"
#include "utils.h"

/*
 *  External String Constants
 */
extern kstring_t kAlertManagerUnknown;

extern kstring_t kAlertManagerModeDisabled;
extern kstring_t kAlertManagerModeEnabled;
extern kstring_t kAlertManagerModeDisconnected;

extern kstring_t kAlertManagerEnabledModeNone;
extern kstring_t kAlertManagerEnabledModeIdle;
extern kstring_t kAlertManagerEnabledModeActive;

extern kstring_t kAlertManagerEnabledActiveModeNone;
extern kstring_t kAlertManagerEnabledActiveModeSending;
extern kstring_t kAlertManagerEnabledActiveModeSent;
extern kstring_t kAlertManagerEnabledActiveModeAcknowledged;
extern kstring_t kAlertManagerEnabledActiveModeCancelling;

/*
 *  Expected Indicator Interface
 *      void alert_on(void);
 *      void alert_off(void);
 *      void alert_flash(void);
 *
 *  Expected Messenger Interface
 *      void request_help()
 *      void cancel_help(uint32_t request_id)
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
        ENABLED_ACTIVE_MODE_SENDING,
        ENABLED_ACTIVE_MODE_SENT,
        ENABLED_ACTIVE_MODE_ACKNOWLEDGED,
        ENABLED_ACTIVE_MODE_CANCELLING
    } enabled_active_mode_t;

    static kstring_t mode_to_kstring(mode_t mode)
    {
        switch (mode)
        {
            case MODE_ENABLED:
                return kAlertManagerModeEnabled;
            case MODE_DISABLED:
                return kAlertManagerModeDisabled;
            case MODE_DISCONNECTED:
                return kAlertManagerModeDisconnected;
            default:
                return kAlertManagerUnknown;
        }
    }

    static kstring_t enabled_mode_to_kstring(enabled_mode_t enabled_mode)
    {
        switch (enabled_mode)
        {
            case ENABLED_MODE_NONE:
                return kAlertManagerEnabledModeNone;
            case ENABLED_MODE_IDLE:
                return kAlertManagerEnabledModeIdle;
            case ENABLED_MODE_ACTIVE:
                return kAlertManagerEnabledModeActive;
            default:
                return kAlertManagerUnknown;
        }
    }

    static kstring_t enabled_active_mode_to_kstring(enabled_active_mode_t enabled_active_mode)
    {
        switch (enabled_active_mode)
        {
            case ENABLED_ACTIVE_MODE_NONE:
                return kAlertManagerEnabledActiveModeNone;
            case ENABLED_ACTIVE_MODE_SENDING:
                return kAlertManagerEnabledActiveModeSending;
            case ENABLED_ACTIVE_MODE_SENT:
                return kAlertManagerEnabledActiveModeSent;
            case ENABLED_ACTIVE_MODE_ACKNOWLEDGED:
                return kAlertManagerEnabledActiveModeAcknowledged;
            case ENABLED_ACTIVE_MODE_CANCELLING:
                return kAlertManagerEnabledActiveModeCancelling;
            default:
                return kAlertManagerUnknown;
        }
    }

    /* Singleton Instance */
    static AlertManager<Indicator, Messenger> s_instance;

    /*
     *  Instance Vaiables
     */

    /* Interface Handles */
    Indicator *_indicator;
    Messenger *_messenger;

    /* State Variables */
    mode_t _mode;
    enabled_mode_t _enabled_mode;
    enabled_active_mode_t _enabled_active_mode;

    /* Stored State Variables */
    mode_t _stored_mode;
    enabled_mode_t _stored_enabled_mode;
    enabled_active_mode_t _stored_enabled_active_mode;

    uuid_t _request_id;

    /*
     *  Constructor
     */

    AlertManager():
        _indicator(NULL),
        _messenger(NULL),
        _mode(MODE_DISABLED),
        _enabled_mode(ENABLED_MODE_IDLE),
        _enabled_active_mode(ENABLED_ACTIVE_MODE_SENT),
        _stored_mode(MODE_DISABLED),
        _stored_enabled_mode(ENABLED_MODE_IDLE),
        _stored_enabled_active_mode(ENABLED_ACTIVE_MODE_SENT)
    {
        memset(_request_id, 0, sizeof(_request_id));
    }

    /*
     * State Storage
     */

    void store_state(void)
    {
        _stored_mode = _mode;
        _stored_enabled_mode = _enabled_mode;
        _stored_enabled_active_mode = _enabled_active_mode;
    }

    void restore_state(void)
    {
        if (_stored_mode == MODE_ENABLED)
        {
            if (_stored_enabled_mode == ENABLED_MODE_ACTIVE)
            {
                set_enabled_active_mode(_stored_enabled_active_mode);
            }
            else
            {
                set_enabled_mode(_stored_enabled_mode);
            }
        }
        else
        {
            set_mode(_stored_mode);
        }
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
                _indicator->power_on();
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
            case ENABLED_MODE_NONE:
                break;
            case ENABLED_MODE_IDLE:
                break;
            case ENABLED_MODE_ACTIVE:
                break;
        }
    }

    void do_enabled_mode_on_enter(enabled_mode_t enabled_mode)
    {
        switch(enabled_mode)
        {
            case ENABLED_MODE_NONE:
                break;
            case ENABLED_MODE_IDLE:
                _indicator->alert_off();
                break;
            case ENABLED_MODE_ACTIVE:
                break;
        }
    }

    void do_enabled_active_mode_on_exit(enabled_active_mode_t enabled_active_mode)
    {
        switch (enabled_active_mode)
        {
            case ENABLED_ACTIVE_MODE_NONE:
                break;
            case ENABLED_ACTIVE_MODE_SENDING:
                break;
            case ENABLED_ACTIVE_MODE_SENT:
                break;
            case ENABLED_ACTIVE_MODE_ACKNOWLEDGED:
                break;
            case ENABLED_ACTIVE_MODE_CANCELLING:
                break;
        }
    }

    void do_enabled_active_mode_on_enter(enabled_active_mode_t enabled_active_mode)
    {
        switch (enabled_active_mode)
        {
            case ENABLED_ACTIVE_MODE_NONE:
                break;
            case ENABLED_ACTIVE_MODE_SENDING:
                _indicator->alert_flash();
                break;
            case ENABLED_ACTIVE_MODE_SENT:
                _indicator->alert_flash();
                break;
            case ENABLED_ACTIVE_MODE_ACKNOWLEDGED:
                _indicator->alert_on();
                break;
            case ENABLED_ACTIVE_MODE_CANCELLING:
                _indicator->alert_flash();
                break;
        }
    }

    void set_mode(mode_t mode, bool_t on_exit=true)
    {
        if (!is_init()) return;

        /* On Exit */
        if (on_exit)
        {
            do_mode_on_exit(_mode);
        }

        /* Transition */
        DLOG2("Alert Manager mode", mode_to_kstring(mode));
        _mode = mode;

        /* On Enter */
        do_mode_on_enter(_mode);
    }

    void set_enabled_mode(enabled_mode_t enabled_mode)
    {
        if (!is_init()) return;

        if (_mode != MODE_ENABLED)
        {
            set_mode(MODE_ENABLED);
        }
        else
        {
            /* On Exit */
            do_enabled_mode_on_exit(_enabled_mode);
        }

        DLOG2("Alert Manager enabled mode", enabled_mode_to_kstring(enabled_mode));
        _enabled_mode = enabled_mode;

        /* On Enter */
        do_enabled_mode_on_enter(_enabled_mode);
    }

    void set_enabled_active_mode(enabled_active_mode_t enabled_active_mode)
    {
        if (!is_init()) return;

        if (_enabled_mode != ENABLED_MODE_ACTIVE)
        {
            set_enabled_mode(ENABLED_MODE_ACTIVE);
        }
        else
        {
            /* On Exit */
            do_enabled_active_mode_on_exit(_enabled_active_mode);
        }

        _enabled_active_mode = enabled_active_mode;
        DLOG2("Alert Manager enabled active mode", enabled_active_mode_to_kstring(enabled_active_mode));

        /* On Enter */
        do_enabled_active_mode_on_enter(_enabled_active_mode);
    }

public:
    static AlertManager * get_instance(void)
    {
        return &s_instance;
    }

    /* Interface Setters */

    void set_indicator_interface(Indicator * indicator)
    {
        _indicator = indicator;
    }

    void set_messenger_interface(Messenger * messenger)
    {
        _messenger = messenger;
    }

    /* Mode Getters */

    bool_t is_init(void)
    {
        return (_messenger && _indicator);
    }

    bool_t is_disabled(void)
    {
        return (_mode == MODE_DISABLED);
    }

    bool_t is_enabled(void)
    {
        return (_mode == MODE_ENABLED);
    }

    bool_t is_idle(void)
    {
        return is_enabled() && (_enabled_mode == ENABLED_MODE_IDLE);
    }

    bool_t is_active(void)
    {
        return is_enabled() && (_enabled_mode == ENABLED_MODE_ACTIVE);
    }

    bool_t is_sending(void)
    {
        return is_active() && (_enabled_active_mode == ENABLED_ACTIVE_MODE_SENDING);
    }

    bool_t is_sent(void)
    {
        return is_active() && (_enabled_active_mode == ENABLED_ACTIVE_MODE_SENT);
    }

    bool_t is_cancelling(void)
    {
        return is_active() && (_enabled_active_mode == ENABLED_ACTIVE_MODE_CANCELLING);
    }

    bool_t is_acknowledged(void)
    {
        return is_active() && (_enabled_active_mode == ENABLED_ACTIVE_MODE_ACKNOWLEDGED);
    }

    bool_t is_disconnected(void)
    {
        return (_mode == MODE_DISCONNECTED);
    }

    /* Request ID Getters */

    uuid_kref_t get_request_id(void)
    {
        if (!is_sent() && !is_acknowledged() && !is_cancelling()) return NULL;
        return _request_id;
    }

    /* Event Triggers */

    void enable(void)
    {
        if (!is_disabled()) return;
        if (!_indicator || !_messenger) return;
        set_mode(MODE_ENABLED);
    }

    void disable(void)
    {
        if (is_disabled()) return;
        set_mode(MODE_DISABLED);
    }

    void help_button_push(void)
    {
        if (!is_idle()) return;
        DLOG("Help Button Pushed Event");
        set_enabled_active_mode(ENABLED_ACTIVE_MODE_SENDING);
    }

    void reset_button_push(void)
    {
        if (!is_sent() && !is_acknowledged()) return;
        DLOG("Cancel Button Pushed Event");
        set_enabled_active_mode(ENABLED_ACTIVE_MODE_CANCELLING);
    }

    void alert_acknowledged(void)
    {
        if (!is_sent())
        DLOG("Alert Acknowledged Event");
        set_enabled_active_mode(ENABLED_ACTIVE_MODE_ACKNOWLEDGED);
    }

    void try_send(void)
    {
        if (!is_sending()) return;
        DLOG("Try Send Alert Event");
        if (_messenger->request_help(_request_id))
        {
            set_enabled_active_mode(ENABLED_ACTIVE_MODE_SENT);
        }
    }

    void try_cancel(void)
    {
        if (!is_cancelling()) return;
        DLOG("Try Cancel Alert Event");
        if (_messenger->cancel_help(_request_id))
        {
            set_enabled_mode(ENABLED_MODE_IDLE);
        }
    }

    void issue_resolved(void)
    {
        if (!is_active()) return;
        DLOG("Issue Resolved Event");
        set_enabled_mode(ENABLED_MODE_IDLE);
    }

    void wifi_connection_lost(void)
    {
        if (!is_enabled()) return;
        DLOG("Connection Lost Event");
        store_state();
        set_mode(MODE_DISCONNECTED, false);
    }

    void wifi_connection_restored(void)
    {
        if (!is_disconnected()) return;
        DLOG("Connection Restored Event");
        restore_state();
    }

    void hard_reset(void)
    {
        DLOG("Hard Reset Event");
        _mode = MODE_DISABLED;
        _enabled_mode = ENABLED_MODE_NONE;
        _enabled_active_mode = ENABLED_ACTIVE_MODE_NONE;
        _request_id = 0;
    }

    typedef Indicator indicator_t;
    typedef Messenger messenger_t;
};

#endif /* _ALERT_MANAGER_H_ */
