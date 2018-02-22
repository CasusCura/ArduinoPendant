/*
 *  Module: Pins
 *
 *  A set of macros which define the different pin values.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _PINS_H_
#define _PINS_H_

/* NodeMCU Pin Mapping */
#define D0      16
#define D1      5
#define D2      4
#define D3      0
#define D4      2
#define D5      14
#define D6      16
#define D7      13
#define D8      15
#define D9      3
#define D10     1

/*
 *  Pin Assignments
 */
/* Buttons */
#define PINS_HELP_BUTTON    D0
#define PINS_RESET_BUTTON   D1

/* LEDs */
#define PINS_POWER_LED      D2
#define PINS_ERROR_LED      D3
#define PINS_REQUEST_LED    D4

#endif /* _PINS_H_ */
