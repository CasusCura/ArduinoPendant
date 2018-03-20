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

#include <ArduinoJson.h>

#include "json.hpp"
#include "utils.h"

#define HTTPER_PARAMETER_MAX    5
#define HTTPER_REQUEST_BODY_SIZE 1024
#define HTTPER_RESPONSE_BODY_SIZE 1024

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
        STATUS_OK,
        STATUS_BAD_AUTH,
        STATUS_NOT_FOUND,

        STATUS_DISCONNECT,
        STATUS_UNKNONW
    } status_t;

private:

    static char_t request_body[HTTPER_REQUEST_BODY_SIZE];
    static char_t response_body[HTTPER_RESPONSE_BODY_SIZE];

    parameter_list_t _parameter_list;
    kstring_t _host;
    kstring_t _path;

public:
    HTTPer(kstring_t host, kstring_t path);

    bool_t push_parameter(kstring_t key, kstring_t value);
    bool_t remove_parameter(kstring_t key);

    status_t send_get(void);
    status_t send_get_expect_json(SafeJson * json_buf);
    status_t send_create(void);

private:
    /*
     *  Prepare Request Buffer
     */
    void clear_request_body(void);
    void prepare_get(void);
    void prepare_create(void);
    void set_host(void);
    void set_basic_auth(void);
    void set_accept_type_json(void);
    void push_form_data(void);
    void end_request(void);

    void send_recv(void);

    /*
     *  Read Response Buffer
     */
    void clear_response_body();
    status_t read_status_code(void);
    char_t * json_start();
    bool_t has_json();


};

#endif /* _HTTPER_HPP_ */
