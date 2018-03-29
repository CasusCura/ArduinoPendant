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

#ifndef _KONSTANTS_H_
#define _KONSTANTS_H_

#include "utils.h"

/* String Constants Prototypes */

/*
 *  Device Information
 */
extern kstring_t kDeviceUUID;
extern kstring_t kDeviceSerial;
extern kstring_t kDeviceUser;
extern kstring_t kDevicePass;

/*
 *  WiFi Information
 */
#ifdef WIFI_ENTEPRISE
/* Enterprise */
extern kstring_t kEntWifiSSID;
extern kstring_t kEntWifiUser;
extern kstring_t kEntWifiPass;
#else
/* Personal */
extern kstring_t kWifiSSID;
extern kstring_t kWifiPass;
#endif

/*
 *  Platform Information
 */
extern kstring_t kPlatformHost;

/*
 * Request Type
 */
extern kstring_t kHelpRequestType;

#endif /* _KONSTANTS_H_ */
