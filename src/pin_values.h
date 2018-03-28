/*
 *  Module: Pin Values
 *
 *  A set of macros which define the different pin values.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _PIN_VALUES_H_
#define _PIN_VALUES_H_

/* NodeMCU Pin Mapping */
#define D0      16
#define D1      5
#define D2      4
#define D3      0
#define D4      2
#define D5      14
#define D6      12
#define D7      13
#define D8      15
#define D9      3
#define D10     1

/*
 *  Pin Assignments
 */
/* Buttons */
#define PIN_HELP_BUTTON        D0
#define PIN_CANCEL_BUTTON      D1

/* LEDs */
#define PIN_POWER_LED      D4
#define PIN_ERROR_LED      D3
#define PIN_ALERT_LED      D2

#endif /* _PIN_VALUES_H_ */
