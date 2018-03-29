/*
 *  Module: WiFi Driver
 *
 *  Wrappers around the poorly documented ESP WiFi module.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2017 Alex Dale
 *  See LICENSE for information.
 */

#include "dlog.h"
#include "utils.h"

/*
 *  Standard Library
 */

#include <string.h>
#include "smlstr.h"

/*
 * Native Drivers
 */

#ifdef WIFI_ENTEPRISE

/*
 *  The following header files are finiky.  The order of import
 *  matters.  user_interface.h declares type definitions which
 *  are required by wpa2_enterprise.h.  They also must like to
 *  a C library.
 *
 *  Summary:  Leave these as is.
 */
START_C_SECTION
#include <user_interface.h>
#include <wpa2_enterprise.h>
END_C_SECTION
#endif /* WIFI_ENTEPRISE */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

/*
 *  Project Files
 */
#include "wifi_driver.h"

/* WiFi credentials. */
#include "konstants.h"

/*
 *  Type Definitions
 */

/*
 *  Public Module Interface
 */

C_FUNCTION void wifi_driver_connect(void)
{
    if (WiFi.status() != WL_CONNECTED)
    {
#ifdef WIFI_ENTEPRISE
        DLOG("Attempting connecting using WPA2-Enterprise");
        wifi_station_connect();
#else
        DLOG("Attempting connecting using WPA2-Personal");
        WiFi.begin(kWifiSSID, kWifiPass);
#endif
    }
}

C_FUNCTION void wifi_driver_disconnect(void)
{
    if (WiFi.status() != WL_CONNECTED)
    {
#ifdef WIFI_ENTEPRISE
        DLOG("Disconnecting from WPA2-Enterprise WiFi");
        wifi_station_disconnect();
#else
        DLOG("Disconnecting from WPA2-Personal WiFi");
        WiFi.disconnect();
#endif
    }
}

C_FUNCTION bool_t wifi_driver_is_connected(void)
{
    return (WiFi.status() == WL_CONNECTED);
}

C_FUNCTION void wifi_driver_log_status(void)
{
    if (wifi_driver_is_connected())
    {
        DLOG2("IP Address", WiFi.localIP().toString().c_str());
        DLOG2("Gateway", WiFi.gatewayIP().toString().c_str());
        DLOG2("MAC Address", WiFi.macAddress().c_str());
    }
}

C_FUNCTION void wifi_driver_init(void)
{
#ifdef WIFI_ENTEPRISE
    struct station_config wifi_config;
    wifi_station_disconnect();

    DLOG("Initializing WiFi for WPA2-Enterprise");

    /* Setup Configuration */
    memset(&wifi_config, 0, sizeof(wifi_config));
    smlstrcpy((string_t) wifi_config.ssid, kEntWifiSSID, sizeof(wifi_config.ssid));
    smlstrcpy((string_t) wifi_config.password, kEntWifiPass, sizeof(wifi_config.password));

    if (!wifi_station_set_config(&wifi_config))
    {
        DLOG_ERR("Failed to set WiFi config.");
    }

    /* Enabled Enterprise Authentication. */
    DLOG("Enabling WPA2-Enterprise");
    if (wifi_station_set_wpa2_enterprise_auth(1))
    {
        DLOG_ERR("Failed to enable WPA2-Enterprise");
    }

    wifi_station_set_enterprise_identity((byte_t *) kEntWifiUser, strlen(kEntWifiUser));
    wifi_station_set_enterprise_username((byte_t *) kEntWifiUser, strlen(kEntWifiUser));
    wifi_station_set_enterprise_password((byte_t *) kEntWifiPass, strlen(kEntWifiPass));

    wifi_station_set_reconnect_policy(true);

    wifi_station_connect();

#else /* not enterrpise */
    DLOG("Initializing WiFi for WPA2-Personal");
    WiFi.begin(kWifiSSID, kWifiPass);
#endif
}

C_FUNCTION void wifi_driver_loop(void)
{
    /* Nothing to do... yet. */
}
