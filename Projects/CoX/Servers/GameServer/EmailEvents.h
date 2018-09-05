/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Servers/InternalEvents.h"

// #ifndef EMAILEVENTS_H
// #define EMAILEVENTS_H

namespace SEGSEvents
{

enum EmailEventTypes : uint32_t
{
    evEmailHeaderRequest,
    evEmailHeaderResponse,
    evEmailReadMessage,
    evEmailSendMessage,
    evEmailDeleteMessage,
    evEmailWasReadByRecipientMessage
};

// when tokens are brought up, use sess.link()->session_token()
// 'id' below is the email id

struct EmailHeaderRequestData
{
    int email_id;
    QString sender;
    QString subject;
    int timestamp;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(email_id, sender, subject, timestamp);
    }
};

struct EmailHeaderResponseData
{
    int email_id;
    QString sender;
    QString subject;
    int timestamp;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(email_id, sender, subject, timestamp);
    }
};
//[[ev_def:macro]]
TWO_WAY_MESSAGE(EmailEventTypes,EmailHeader)

struct EmailReadData
{
    int email_id;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(email_id);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(EmailEventTypes,EmailRead)

struct EmailSendData
{
    int email_id;
    QString sender;
    QString recipient;
    QString subject;
    QString message;
    int timestamp;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(email_id, sender, recipient, subject, message, timestamp);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(EmailEventTypes,EmailSend)

struct EmailDeleteData
{
    int email_id;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(email_id);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(EmailEventTypes,EmailDelete)

struct EmailWasReadByRecipientData
{
    int email_id;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(email_id);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(EmailEventTypes,EmailWasReadByRecipient)

} // end of SEGSEvent namespace
