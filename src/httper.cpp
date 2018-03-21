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

/* Standard Library */
#include <string.h>

/* 3rd Party Library */
#include <ESP8266HTTPClient.h>

/* Project Library */
#include "dlog.h"
#include "smlstr.h"
#include "konstants.h"
#include "wifi_driver.h"

/* Self Header */
#include "httper.hpp"

#define URL_BUFFER_LENGTH 256
#define PORT_BUFFER_LENGTH 8


/* Konstants */
static kstring_t kAccept = "Accept";
static kstring_t kApplicationJson = "application/json";
static kstring_t kHttp = "http://";

static kstring_t http_code_to_string(int16_t http_code)
{
    switch (http_code)
    {
        /* 200 series */
        case HTTP_CODE_OK:
            return "OK";
        case HTTP_CODE_CREATED:
            return "CREATED";
        case HTTP_CODE_NO_CONTENT:
            return "No Content";
        /* 400 series */
        case HTTP_CODE_BAD_REQUEST:
            return "Bad Request";
        case HTTP_CODE_UNAUTHORIZED:
            return "Unauthorized";
        case HTTP_CODE_FORBIDDEN:
            return "Code Forbidden";
        case HTTP_CODE_NOT_FOUND:
            return "Not Found";
        case HTTP_CODE_METHOD_NOT_ALLOWED:
            return "Method Not Allowed";
        case HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED:
            return "Proxy Authentication Required";
        case HTTP_CODE_REQUEST_TIMEOUT:
            return "Request Timeout";
        /* 500 series */
        case HTTP_CODE_INTERNAL_SERVER_ERROR:
            return "Internal Server Error";
        case HTTP_CODE_NOT_IMPLEMENTED:
            return "Not Implemented";
        case HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED:
            return "Network Authentication Required";
        default:
            return "Unknown";
    }
}


HTTPer::HTTPer(kstring_t host, uint16_t port, kstring_t path):
    _host(host),
    _port(port),
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

HTTPer::status_t HTTPer::send_get(char_t * payload, uint16_t payload_length)
{
    HTTPClient client;
    String response_body;
    char_t url_buffer[URL_BUFFER_LENGTH];
    char_t port_buffer[PORT_BUFFER_LENGTH];
    BufStr url(url_buffer, URL_BUFFER_LENGTH);
    int16_t http_code;
    kstring_t http_code_str;

    /* Check for WiFi connection. */
    if (!wifi_driver_is_connected())
    {
        return STATUS_DISCONNECT;
    }

    /*
     * Prepare URI
     */
    DLOG("Preparing GET URL");

    /* Protocol */
    if (!url.push_str(kHttp))
    {
        DLOG_ERR("Could not write protocol to URL");
        return STATUS_INTERNAL_ERROR;
    }

    /* Host component */
    if (!url.push_str(_host))
    {
        DLOG_ERR2("Could not write host to URL", _host);
        return STATUS_INTERNAL_ERROR;
    }

    /* Port component */
    if (_port != 80)
    {
        url.push_char(':');
        if (smluintfmt(port_buffer, _port, PORT_BUFFER_LENGTH) >= PORT_BUFFER_LENGTH)
        {
            DLOG_ERR("Could not convert port to string.");
            return STATUS_INTERNAL_ERROR;
        }
        if (!url.push_str(port_buffer))
        {
            DLOG_ERR2("Could not write port to URL", port_buffer);
            return STATUS_INTERNAL_ERROR;
        }
    }

    /* Path component. */
    if (!url.push_str(_path))
    {
        DLOG_ERR2("Could not write path to URL", _path);
        return STATUS_INTERNAL_ERROR;
    }

    /* Parameter component */
    if (!write_query_parameters(&url))
    {
        DLOG_ERR("Could not write query parameters to URL");
        return STATUS_INTERNAL_ERROR;
    }

    DLOG2("Beginning HTTP Client", url.buffer());

    if (!client.begin(String(url.buffer())))
    {
        DLOG_ERR2("Failed to begin client with URL", url.buffer());
        return STATUS_INTERNAL_ERROR;
    }

    /* Set Expected Conent Type */
    client.addHeader(String(kAccept), String(kApplicationJson));

    /* Set Authorization of Device */
    client.setAuthorization(kWifiUser, kWifiPass);

    DLOG("Sending GET request...");
    http_code = client.GET();

    if (http_code < 0)
    {
        DLOG_ERR2("HTTP client returned error on GET", _path);
        return STATUS_INTERNAL_ERROR;
    }

    http_code_str = http_code_to_string(http_code);
    DLOG2("GET", http_code_str);

    switch (http_code)
    {
        case HTTP_CODE_OK:
            /* Parse Payload */
            if (payload)
            {
                response_body = client.getString();
                DLOG2("Got data", response_body.c_str());
                if (strlcpy(payload, response_body.c_str(), payload_length) >= payload_length)
                {
                    DLOG_ERR("Provided payload buffer is too small");
                    return STATUS_PAYLOAD_TOO_SMALL;
                }
                return STATUS_OK;
            }
            return STATUS_OK;
        case HTTP_CODE_NO_CONTENT:
            if (payload)
            {
                DLOG("No data, clearing buffer");
                memset(payload, 0, payload_length);
            }
            return STATUS_OK;
        case HTTP_CODE_BAD_REQUEST:
        case HTTP_CODE_METHOD_NOT_ALLOWED:
            return STATUS_BAD_REQUEST;
        case HTTP_CODE_UNAUTHORIZED:
        case HTTP_CODE_FORBIDDEN:
        case HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED:
        case HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED:
            return STATUS_BAD_AUTH;
        case HTTP_CODE_INTERNAL_SERVER_ERROR:
        case HTTP_CODE_NOT_IMPLEMENTED:
        case HTTP_CODE_REQUEST_TIMEOUT:
            return STATUS_REMOTE_ERROR;
        default:
            return STATUS_UNKNOWN;
    }
}

/* Private Methods */

void prepare_parameter(char_t * buffer, uint16_t length)
{
    BufStr parameters(buffer, length);

    if (!buffer || !length) return;

}

bool_t HTTPer::write_parameters(BufStr * bstr)
{
    uint8_t i;

    if (!bstr)
    {
        DLOG_ERR("NULL pointer for BufStr");
        return false;
    }

    for (i = 0; i < _parameter_list.n; i++)
    {
        if (i > 0)
        {
            bstr->push_char('&');
        }
        bstr->push_str(_parameter_list.elems[i].key);
        bstr->push_char('=');
        if (!bstr->push_str(_parameter_list.elems[i].value))
        {
            DLOG_ERR("Failed to add parameter");
            DLOG_ERR2(" KEY", _parameter_list.elems[i].key);
            DLOG_ERR2(" VAL", _parameter_list.elems[i].value);
            return false;
        }
    }
    return true;
}

bool_t HTTPer::write_query_parameters(BufStr * url)
{
    if (!url)
    {
        DLOG_ERR("NULL pointer for BufStr uri");
        return false;
    }
    if (_parameter_list.n > 0)
    {
        url->push_char('?');
        return write_parameters(url);
    }
    return true;
}
