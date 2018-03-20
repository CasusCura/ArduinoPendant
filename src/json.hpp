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

#ifndef _JSON_HPP_
#define _JSON_HPP_

#include <ArduinoJson.h>

#define JSON_MAX_SIZE 1024

typedef StaticJsonBuffer<JSON_MAX_SIZE> SafeJson;

#endif /* _JSON_HPP_ */
