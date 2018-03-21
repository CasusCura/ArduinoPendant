/*
 *  Module: Buffer String
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#include <string.h>

#include "bufstr.hpp"

BufStr::BufStr(char_t * buffer, uint16_t size, bool_t clear):
    _start(buffer),
    _size(size),
    _length(0)
{
    if (clear)
    {
        this->clear();
    }
    else if (_start && size > 0)
    {
        while (_start[_length] && (_length+1) < _size)
        {
            _length++;
        }
        _start[_length] = 0;
    }
}

bool_t BufStr::clear(void)
{
    if (_start && _size)
    {
        memset(_start, 0, _size);
        return true;
    }
    return false;
}

bool_t BufStr::push_str(kstring_t s)
{
    char_t * ptr;
    uint8_t i;

    if (!_start || !s)
    {
        return false;
    }

    if ((_length + 1) >= _size)
    {
        return false;
    }
    ptr = &_start[_length];

    for (i = 0; s[i] && (_length+1) < _size; i++)
    {
        *ptr++ = s[i];
        _length++;
    }
    *ptr = 0;
    return !s[i];
}

bool_t BufStr::push_char(char_t c)
{
    if (!_start)
    {
        return false;
    }

    if ((_length + 1) >= _size)
    {
        return false;
    }

    _start[_length++] = c;
    _start[_length] = 0;

    return true;
}

char_t * BufStr::buffer(void)
{
    return _start;
}

uint16_t BufStr::size(void)
{
    return _size;
}

uint16_t BufStr::length(void)
{
    return _length;
}
