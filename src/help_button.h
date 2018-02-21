/*
 *  Module: Help Button
 *
 *  Help & reset button interface.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _HELP_BUTTON_H_
#define _HELP_BUTTON_H_

#include "utils.h"

START_C_SECTION

#define HELP_BUTTON_DEBOUNCE_DELAY  20  /* ms */

void help_button_setup(void);
bool_t help_button_read(void);
bool_t help_button_read_reset(void);

END_C_SECTION

#endif /* _HELP_BUTTON_H_ */
