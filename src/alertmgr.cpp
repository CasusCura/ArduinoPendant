/*
 *  Module: Alert Manager

 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#include "alertmgr.hpp"

kstring_t kAlertManagerUnknown = "UNKNOWN";

kstring_t kAlertManagerModeDisabled = "MODE_DISABLED";
kstring_t kAlertManagerModeEnabled = "MODE_ENABLED";
kstring_t kAlertManagerModeDisconnected = "MODE_DISCONNECTED";

kstring_t kAlertManagerEnabledModeNone = "ENABLED_MODE_NONE";
kstring_t kAlertManagerEnabledModeIdle = "ENABLED_MODE_IDLE";
kstring_t kAlertManagerEnabledModeActive = "ENABLED_MODE_ACTIVE";

kstring_t kAlertManagerEnabledActiveModeNone = "ENABLED_ACTIVE_MODE_NONE";
kstring_t kAlertManagerEnabledActiveModeSending = "ENABLED_ACTIVE_MODE_SENDING";
kstring_t kAlertManagerEnabledActiveModeSent = "ENABLED_ACTIVE_MODE_SENT";
kstring_t kAlertManagerEnabledActiveModeAcknowledged = "ENABLED_ACTIVE_MODE_ACKNOWLEDGED";
kstring_t kAlertManagerEnabledActiveModeCancelling = "ENABLED_ACTIVE_MODE_CANCELLING";
