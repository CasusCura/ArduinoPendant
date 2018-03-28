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

#ifndef _FAKE_MESSENGER_HPP_
#define _FAKE_MESSENGER_HPP_

#include "uuid.h"
#include "utils.h"

class FakeMessenger {
    static FakeMessenger s_instance;

    bool_t _sent;

    FakeMessenger();
public:
    static FakeMessenger * get_instance(void);

    bool_t request_help(uuid_ref_t request_id);
    bool_t cancel_help(uuid_kref_t request_id);

    bool_t test(void);
};

#endif /* _FAKE_MESSENGER_HPP_ */
