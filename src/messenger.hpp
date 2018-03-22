/*
 *  Module: HTTP Messenger
 *
 *  Author: Alex Dale @superoxigen
 *
 *  Copyright (c) 2017 Alex Dale
 *  See LICENSE for information.
 */

#ifndef _MESSENGER_HPP_
#define _MESSENGER_HPP_

#include "uuid.h"
#include "utils.h"

class Messenger {
    static Messenger s_instance;

    Messenger();
public:
    Messenger * get_instance(void);

    bool_t request_help(uuid_ref_t request_id);
    bool_t cancel_help(uuid_kref_t request_id);
};

#endif /* _MESSENGER_HPP_ */
