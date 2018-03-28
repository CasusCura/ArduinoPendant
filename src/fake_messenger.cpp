/*
 *  Module: Fake HTTP Messenger
 *
 *  Like the Messenger module, but does not use network.
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2017 Alex Dale
 *  See LICENSE for information.
 */

#include "fake_messenger.hpp"


FakeMessenger FakeMessenger::s_instance;

FakeMessenger::FakeMessenger(void):
    _sent(false) {}

FakeMessenger * FakeMessenger::get_instance(void)
{
    return &s_instance;
}

bool_t FakeMessenger::request_help(uuid_ref_t request_id)
{
    if (!request_id) return false;

    uuid_set_zero(request_id);

    if (_sent) return false;

    _sent = true;
    return true;
}

bool_t FakeMessenger::cancel_help(uuid_kref_t request_id)
{
    if (!request_id) return false;

    if (!_sent) return false;

    _sent = false;
    return true;
}

bool_t FakeMessenger::test(void)
{
    return true;
}
