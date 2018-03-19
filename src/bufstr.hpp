/*
 *  Module: Buffer String
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _BUFSTR_HPP_
#define _BUFSTR_HPP_

#include "utils.h"

class BufStr {
    char_t * _start;
    uint16_t _size;
    uint16_t _length;

public:
    BufStr(char_t * buffer, uint16_t size, bool_t clear=false);

    bool_t clear(void);
    bool_t push_str(kstring_t s);
    bool_t push_char(char_t c);

    char_t * buffer(void);
    uint16_t size(void);
    uint16_t length(void);
};

#endif /* _BUFSTR_HPP_ */
