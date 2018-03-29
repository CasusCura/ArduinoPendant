/*
 *  Module: HTTP Messenger
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2017 Alex Dale
 *  See LICENSE for information.
 */

#include "dlog.h"
#include "httper.hpp"
#include "json.hpp"
#include "konstants.h"
#include "smlstr.h"

#include "messenger.hpp"

#define PLATFORM_PORT 80

static kstring_t kHelpRequestPath = "/patient/request1";
static kstring_t kCancelRequestPath = "/patient/request/cancel";
static kstring_t kTestPath = "/patient/test";

static kstring_t kDeviceUUIDKey = "device_id";
static kstring_t kRequestUUIDKey = "issue_id";
static kstring_t kRequestTypeKey = "request_type_id";

#define RESPONSE_BODY_LENGTH  1024

Messenger Messenger::s_instance = Messenger();

Messenger::Messenger() {}

Messenger * Messenger::get_instance(void)
{
    return &s_instance;
}

bool_t Messenger::request_help(uuid_ref_t request_id)
{
    HTTPer client(kPlatformHost, PLATFORM_PORT, kHelpRequestPath);
    HTTPer::status_t status;
    SafeJson json_buf;
    char_t response_body[RESPONSE_BODY_LENGTH];

    if (!request_id)
    {
        DLOG_ERR("Cannot request help with out request ID buffer");
        return false;
    }

    /* Push Parameters */
    DLOG("Pushing parameters");
    client.push_parameter(kDeviceUUIDKey, kDeviceUUID);
    client.push_parameter(kRequestTypeKey, kHelpRequestType);

    /* Send Post */
    DLOG("Sending request for help");
    status = client.send_post(response_body, RESPONSE_BODY_LENGTH);

    if (status == HTTPer::STATUS_OK)
    {
        DLOG("Request successully sent and accepted");
        JsonObject& root = json_buf.parseObject(response_body);
        if (!root.success())
        {
            DLOG_WARN2("Failed to parse object as JSON", response_body);
            uuid_set_zero(request_id);
            return true;
        }
        else if (!root[kRequestUUIDKey].success())
        {
            DLOG_WARN2("Returned JSON does not have request ID key", response_body);
            uuid_set_zero(request_id);
            return true;
        }
        smlstrcpy(request_id, root[kRequestUUIDKey], UUID_BUFFER_LENGTH);
        if (!uuid_is_uuid(request_id))
        {
            DLOG_WARN2("Returned request ID is not UUID", request_id);
            uuid_set_zero(request_id);
            return true;
        }
        return true;
    }
    else if (status == HTTPer::STATUS_PAYLOAD_TOO_SMALL)
    {
        DLOG_WARN("Response body was too small");
        uuid_set_zero(request_id);
        return true;
    }
    else
    {
        DLOG_ERR("Request for help failed");
        return false;
    }

    return true;
}

bool_t Messenger::cancel_help(uuid_kref_t request_id)
{
    HTTPer client(kPlatformHost, PLATFORM_PORT, kCancelRequestPath);
    HTTPer::status_t status;

    if (!request_id)
    {
        DLOG_ERR("Cannot cancel help without request ID");
        return false;
    }

    /* Push Parameters */
    DLOG("Pushing parameters");
    client.push_parameter(kDeviceUUIDKey, kDeviceUUID);
    client.push_parameter(kRequestUUIDKey, request_id);

    /* Send Post */
    DLOG("Sending request to cancel help");
    status = client.send_post();

    if (status == HTTPer::STATUS_OK)
    {
        DLOG("Request successully cancelled");
        return true;
    }

    DLOG_ERR("Failed to cancel request");
    return false;
}

bool_t Messenger::test(void)
{
    HTTPer client(kPlatformHost, PLATFORM_PORT, kTestPath);
    HTTPer::status_t status;

    DLOG("Pushing parameters");
    client.push_parameter(kDeviceUUIDKey, kDeviceUUID);

    DLOG("Testing service");
    status = client.send_get();

    if (status == HTTPer::STATUS_OK)
    {
        DLOG("Test successful");
        return true;
    }
    DLOG_ERR("Test failed");
    return false;
}
