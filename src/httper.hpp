/*
 *  Module: HTTPer
 *
 *  An HTTP request and response handler.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2018 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _HTTPER_HPP_
#define _HTTPER_HPP_

#include "bufstr.hpp"
#include "utils.h"

#define HTTPER_PARAMETER_MAX    5

class HTTPer {

    typedef struct {
        kstring_t key;
        kstring_t value;
    } parameter_t;

    typedef struct {
        uint16_t n;
        parameter_t elems[HTTPER_PARAMETER_MAX];
    } parameter_list_t;

public:
    typedef enum {
        /* HTTP Related */
        STATUS_OK,
        STATUS_BAD_AUTH,
        STATUS_BAD_REQUEST,
        STATUS_REMOTE_ERROR,

        /* System Related */
        STATUS_DISCONNECT,
        STATUS_INTERNAL_ERROR,
        STATUS_PAYLOAD_TOO_SMALL,
        STATUS_UNKNOWN
    } status_t;

private:
    parameter_list_t _parameter_list;
    kstring_t _host;
    uint16_t _port;
    kstring_t _path;

public:
    HTTPer(kstring_t host, uint16_t _port, kstring_t path);

    bool_t push_parameter(kstring_t key, kstring_t value);
    bool_t remove_parameter(kstring_t key);

    status_t send_get(char_t * payload, uint16_t payload_length);
    status_t send_get(void);
    status_t send_post(char_t * payload, uint16_t payload_length);
    status_t send_post(void);

private:
    bool_t write_parameters(BufStr * bstr);
    bool_t write_query_parameters(BufStr * uri);
    bool_t write_url(BufStr * url);
};

#endif /* _HTTPER_HPP_ */
