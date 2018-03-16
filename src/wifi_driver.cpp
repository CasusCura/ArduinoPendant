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

#include "utils.h"

/*
 *  Standard Library
 */

#include <string.h>

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
        wifi_station_connect();
    }
}

C_FUNCTION void wifi_driver_disconnect(void)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        wifi_station_connect();
    }
}

C_FUNCTION bool_t wifi_driver_is_connected(void)
{
    return (WiFi.status() == WL_CONNECTED);
}

C_FUNCTION void wifi_driver_init(void)
{
#ifdef WIFI_ENTEPRISE
    struct station_config wifi_config;
    wifi_station_disconnect();

    memset(&wifi_config, 0, sizeof(wifi_config));
    strcpy((string_t) wifi_config.ssid, kWifiSSID);
    strcpy((string_t) wifi_config.password, kWifiPass);
    wifi_station_set_config(&wifi_config);

    /* Enabled Enterprise Authentication. */
    wifi_station_set_wpa2_enterprise_auth(1);

    wifi_station_set_enterprise_username((byte_t *) kWifiUser, strlen(kWifiUser));
    wifi_station_set_enterprise_password((byte_t *) kWifiPass, strlen(kWifiPass));

    wifi_station_connect();

#else /* not enterrpise */
    WiFi.begin(kWifiSSID, kWifiPass);
#endif
}

C_FUNCTION void wifi_driver_loop(void)
{
    /* Nothing to do... yet. */
}
