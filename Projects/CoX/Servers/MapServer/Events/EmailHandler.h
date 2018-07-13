/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef EMAILHANDLER_H
#define EMAILHANDLER_H

#include "EventProcessor.h"
#include "EmailEvents.h"

class EmailHandler : public EventProcessor
{
private:
    // EventProcessor interface
    void dispatch(SEGSEvent *ev) override;

    void on_email_header(EmailHeaderMessage* msg);
    void on_email_sent();
    void on_email_read();
    void on_email_delete();
public:
    EmailHandler();
};

#endif // EMAILHANDLER_H
