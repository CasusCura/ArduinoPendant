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

#ifndef _WIFI_DRIVER_H_
#define _WIFI_DRIVER_H_

#include "utils.h"

START_C_SECTION

void wifi_driver_connect(void);
void wifi_driver_disconnect(void);
bool_t wifi_driver_is_connected(void);

void wifi_driver_init(void);
void wifi_driver_loop(void);

END_C_SECTION

#endif /* _WIFI_DRIVER_H_ */
