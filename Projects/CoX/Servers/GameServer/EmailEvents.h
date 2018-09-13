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
    evEmailHeaderRequest = Internal_EventTypes::ID_LAST_Internal_EventTypes,
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
    uint32_t sender_id;
    QString sender_name;
    QString subject;
    int timestamp;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(sender_id, sender_name, subject, timestamp);
    }
};

struct EmailHeaderResponseData
{
    uint32_t email_id;
    QString sender_name;
    QString subject;
    int timestamp;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(email_id, sender_name, subject, timestamp);
    }
};
//[[ev_def:macro]]
TWO_WAY_MESSAGE(EmailEventTypes,EmailHeader)

struct EmailReadData
{
    uint32_t email_id;

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
    uint32_t sender_id;
    uint32_t recipient_id;
    QString sender_name;
    QString subject;
    QString message;
    int timestamp;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(sender_id, recipient_id, sender_name, subject, message, timestamp);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(EmailEventTypes,EmailSend)

struct EmailDeleteData
{
    uint32_t email_id;

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
    uint32_t email_id;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(email_id);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(EmailEventTypes,EmailWasReadByRecipient)

} // end of SEGSEvent namespace
