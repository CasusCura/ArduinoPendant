/*
 *  Module: Konstants
 *
 *  Variable Constants
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#include "konstants.h"

#include "env_config.h"

/* Device Information */
kstring_t kDeviceUUID = DEVICE_UUID;
kstring_t kDeviceSerial = DEVICE_SERIAL;

#ifdef WIFI_ENTEPRISE
/* WiFi Information - Enterprise */
kstring_t kEntWifiSSID = ENT_WIFI_SSID;
kstring_t kEntWifiUser = ENT_WIFI_USERNAME;
kstring_t kEntWifiPass = ENT_WIFI_PASSWORD;

kstring_t kDeviceUser = ENT_WIFI_USERNAME;
kstring_t kDevicePass = ENT_WIFI_PASSWORD;
#else
/* WiFi Information - Personal */
kstring_t kWifiSSID = WIFI_SSID;
kstring_t kWifiPass = WIFI_PASSWORD;

kstring_t kDeviceUser = "test";
kstring_t kDevicePass = "test";
#endif

/* Platform Information */
kstring_t kPlatformHost = PLATFORM_HOST;
