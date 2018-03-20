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

#include <string.h>

#include <Base64.h>

#include "bufstr.hpp"
#include "konstants.h"
#include "wifi_driver.h"

#include "httper.hpp"

#define AUTH_BUFFER_LENGTH 256

/* Konstants */

static kstring_t kGet = "GET";
static kstring_t kCreate = "CREATE";
static kstring_t kHttpVersion = "HTTP/1.1";
static kstring_t kHost = "Host:";
static kstring_t kAccept = "Accept:";
static kstring_t kAuth = "Authorization:";
static kstring_t kCRLF = "\r\n";

static kstring_t kBasicAuth = "Basic";
static kstring_t kApplicationJson = "application/json";


static char_t * str_push_str(char_t * start, char_t * pos, kstring_t src, uint16_t length)
{
    uint16_t cur;
    uint16_t i;

    /* Check current length */
    cur = (uint16_t) (pos - start);
    if (cur >= (length-1)) return pos;

    for (i = 0; src[i] && (cur+i+1) < length; i++)
    {
        *pos++ = src[i];
    }
    *pos = 0;

    return pos;
}

static char_t * str_push_char(char_t * start, char_t * pos, char_t src, uint16_t length)
{
    uint16_t cur;

    cur = (uint16_t) (pos - start);
    if (cur >= (length-1)) return pos;
    *pos++ = src;
    *pos = 0;
    return pos;
}


char_t HTTPer::request_body[HTTPER_REQUEST_BODY_SIZE];
char_t HTTPer::response_body[HTTPER_RESPONSE_BODY_SIZE];

HTTPer::HTTPer(kstring_t host, kstring_t path):
    _host(host),
    _path(path)
{
    memset(&_parameter_list, 0, sizeof(_parameter_list));
}

bool_t HTTPer::push_parameter(kstring_t key, kstring_t value)
{
    if (_parameter_list.n >= HTTPER_PARAMETER_MAX) return false;
    if (!key || !value) return false;

    _parameter_list.elems[_parameter_list.n].key = key;
    _parameter_list.elems[_parameter_list.n].value = value;
    _parameter_list.n++;
    return true;
}

bool_t HTTPer::remove_parameter(kstring_t key)
{
    int i, j;
    if (!_parameter_list.n || !key) return false;

    for (i = 0; i < _parameter_list.n; i++)
    {
        if (!strcmp(key, _parameter_list.elems[i].key)) break;
    }

    if (i == _parameter_list.n) return false;

    for (i = 0; i < (_parameter_list.n-1); i++)
    {
        _parameter_list.elems[i].key = _parameter_list.elems[i+1].key;
        _parameter_list.elems[i].value = _parameter_list.elems[i+1].value;
    }

    _parameter_list.elems[i].key = NULL;
    _parameter_list.elems[i].value = NULL;
    _parameter_list.n--;

    return true;
}

HTTPer::status_t HTTPer::send_get(void)
{
    if (!wifi_driver_is_connected()) return STATUS_DISCONNECT;

    clear_request_body();
    prepare_get();
    end_request();

    clear_response_body();
    send_recv();

    return read_status_code();
}

HTTPer::status_t HTTPer::send_get_expect_json(SafeJson * json_buf)
{
    if (!wifi_driver_is_connected()) return STATUS_DISCONNECT;

    clear_request_body();
    prepare_get();
    set_accept_type_json();
    end_request();

    clear_response_body();
    send_recv();

    if (has_json())
    {
        json_buf->parseObject(json_start());
    }

    return read_status_code();
}


/* Private Methods */

// TODO: check for errors and return internal error codes.

void HTTPer::clear_request_body(void)
{
    memset(response_body, 0, sizeof(response_body));
}

void HTTPer::prepare_get(void)
{
    BufStr body(request_body, sizeof(request_body));
    uint8_t i;

    body.push_str(kGet);
    body.push_char(' ');
    body.push_str(_path);

    if (_parameter_list.n > 0)
    {
        body.push_char('?');

        for (i = 0; i < _parameter_list.n; i++)
        {
            if (i > 0)
            {
                body.push_char('&');
            }
            body.push_str(_parameter_list.elems[i].key);
            body.push_char('=');
            body.push_str(_parameter_list.elems[i].value);
        }
    }

    body.push_char(' ');
    body.push_str(kHttpVersion);
    body.push_str(kCRLF);
}

void HTTPer::prepare_create(void)
{
    BufStr body(request_body, sizeof(request_body));

    body.push_str(kCreate);
    body.push_char(' ');
    body.push_str(_path);
    body.push_char(' ');
    body.push_str(kHttpVersion);
    body.push_str(kCRLF);
}

void HTTPer::set_host(void)
{
    BufStr body(request_body, sizeof(request_body));
    body.push_str(kHost);
    body.push_char(' ');
    body.push_str(_host);
    body.push_str(kCRLF);
}

void HTTPer::set_accept_type_json(void)
{
    BufStr body(request_body, sizeof(request_body));
    body.push_str(kAccept);
    body.push_char(' ');
    body.push_str(kApplicationJson);
    body.push_str(kCRLF);
}

void HTTPer::set_basic_auth(void)
{
    BufStr body(request_body, sizeof(request_body));
    char_t auth_out_buffer[AUTH_BUFFER_LENGTH];
    char_t auth_in_buffer[AUTH_BUFFER_LENGTH];
    BufStr auth_in(auth_in_buffer, AUTH_BUFFER_LENGTH);

    memset(auth_out_buffer, 0, AUTH_BUFFER_LENGTH);

    auth_in.push_str(kWifiUser);
    auth_in.push_char(':');
    auth_in.push_str(kWifiPass);

    base64_encode(auth_out_buffer, auth_in.buffer(), auth_in.length());

    body.push_str(kAuth);
    body.push_char(' ');
    body.push_str(kBasicAuth);
    body.push_char(' ');
    body.push_str(auth_out_buffer);
    body.push_str(kCRLF);
}

void HTTPer::push_form_data(void)
{
    BufStr body(request_body, sizeof(request_body));
    uint8_t i;

    if (_parameter_list.n > 0)
    {
        body.push_char('?');

        for (i = 0; i < _parameter_list.n; i++)
        {
            if (i > 0)
            {
                body.push_char('&');
            }
            body.push_str(_parameter_list.elems[i].key);
            body.push_char('=');
            body.push_str(_parameter_list.elems[i].value);
        }
    }
    body.push_str(kCRLF);
}

void HTTPer::end_request(void)
{
    BufStr body(request_body, sizeof(request_body));
    body.push_str(kCRLF);
}

void HTTPer::send_recv(void)
{

}
