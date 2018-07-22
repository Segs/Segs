/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "Servers/InternalEvents.h"

#ifndef EMAILEVENTS_H
#define EMAILEVENTS_H

enum EmailEventTypes : uint32_t
{
    evEmailHeaderRequest,
    evEmailHeaderResponse,
    evEmailRead,
    evEmailSend,
    evEmailDelete,
    evEmailWasReadByRecipient
};

// when tokens are brought up, use sess.link()->session_token()

#define ONE_WAY_MESSAGE(name)\
struct name ## Message final : public InternalEvent\
{\
    name ## Data m_data;\
    name ## Message(name ## Data &&d, uint64_t token) :  InternalEvent(EmailEventTypes::ev ## name),m_data(d) {session_token(token);}\
};

/// A message without Request having additional data
#define SIMPLE_TWO_WAY_MESSAGE(name)\
struct name ## Request final : public InternalEvent\
{\
    name ## Message() :  InternalEvent(EmailEventTypes::ev ## name ## Request) {}\
};\
struct name ## Response final : public InternalEvent\
{\
    name ## Data m_data;\
    name ## Response(name ## Data &&d) :  InternalEvent(EmailEventTypes::ev ## name ## Response),m_data(d) {}\
};

/// A message with Request having additional data
#define TWO_WAY_MESSAGE(name)\
struct name ## Request final : public InternalEvent\
{\
    name ## RequestData m_data;\
    name ## Request(name ## RequestData &&d,uint64_t token,EventProcessor *src = nullptr) :\
        InternalEvent(EmailEventTypes::ev ## name ## Request,src),m_data(d) {session_token(token);}\
};\
struct name ## Response final : public InternalEvent\
{\
    name ## ResponseData m_data;\
    name ## Response(name ## ResponseData &&d,uint64_t token) :  InternalEvent(EmailEventTypes::ev ## name ## Response),m_data(d) {session_token(token);}\
};

// 'id' below is the email id

struct EmailHeaderRequestData
{
    int id;
    QString sender;
    QString subject;
    int timestamp;
};

struct EmailHeaderResponseData
{
    int id;
    QString sender;
    QString subject;
    int timestamp;
};
TWO_WAY_MESSAGE(EmailHeader)

struct EmailReadData
{
    int id;
};
ONE_WAY_MESSAGE(EmailRead)

struct EmailSendData
{
    int id;
    QString sender;
    QString recipient;
    QString subject;
    QString message;
    int timestamp;
};
ONE_WAY_MESSAGE(EmailSend)

struct EmailDeleteData
{
    int id;
};
ONE_WAY_MESSAGE(EmailDelete)

struct EmailWasReadByRecipientData
{
    int id;
};
ONE_WAY_MESSAGE(EmailWasReadByRecipient)

#undef ONE_WAY_MESSAGE
#endif
