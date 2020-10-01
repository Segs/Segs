/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Servers/InternalEvents.h"
#include "EmailDefinitions.h"

namespace SEGSEvents
{

enum EmailEventTypes : uint32_t
{
    evEmailHeadersToClientMessage = Internal_EventTypes::ID_LAST_Internal_EventTypes,
    evEmailHeaderToClientMessage,
    evEmailHeaderRequest,
    evEmailHeaderResponse,
    evEmailReadRequest,
    evEmailReadResponse,
    evEmailSendMessage,
    evEmailSendErrorMessage,
    evEmailCreateStatusMessage,
    evEmailDeleteMessage,
    evEmailWasReadByRecipientMessage
};

// when tokens are brought up, use sess.link()->session_token()
// 'id' below is the email id
struct EmailHeaderRequestData
{
    uint32_t m_user_id;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_user_id);
    }
};

struct EmailHeaderResponseData
{
    std::vector<EmailHeaderData> m_email_headers;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_headers);
    }
};
//[[ev_def:macro]]
TWO_WAY_MESSAGE(EmailEventTypes,EmailHeader)

struct EmailHeadersToClientData
{
    std::vector<EmailHeaderData> m_email_headers;
    int m_unread_emails_count;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_headers, m_unread_emails_count);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(EmailEventTypes,EmailHeadersToClient)

struct EmailHeaderToClientData
{
    uint32_t m_email_id;
    QString m_sender_name;
    QString m_subject;
    uint32_t m_timestamp;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_id, m_sender_name, m_subject, m_timestamp);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(EmailEventTypes,EmailHeaderToClient)

struct EmailReadRequestData
{
    uint32_t m_email_id;
    uint32_t m_reader_id;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_id, m_reader_id);
    }
};

struct EmailReadResponseData
{
    uint32_t m_email_id;
    QString m_message;
    QString m_sender_name;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_id, m_message, m_sender_name);
    }
};
//[[ev_def:macro]]
TWO_WAY_MESSAGE(EmailEventTypes,EmailRead)

struct EmailSendData
{
    uint32_t m_sender_id;
    QString m_sender_name;
    QString m_recipient_name;
    QString m_subject;
    QString m_message;
    uint32_t m_timestamp;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_sender_id, m_sender_name, m_recipient_name, m_subject, m_message, m_timestamp);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(EmailEventTypes,EmailSend)

struct EmailSendErrorData
{
    QString m_error_message;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar (m_error_message);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(EmailEventTypes,EmailSendError)

struct EmailDeleteData
{
    uint32_t m_email_id;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_id);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(EmailEventTypes,EmailDelete)

struct EmailWasReadByRecipientData
{
    QString m_message;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_message);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(EmailEventTypes,EmailWasReadByRecipient)

struct EmailCreateStatusData
{
    bool m_status;
    QString m_recipient_name;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_status, m_recipient_name);
    }
};
//[[ev_def:macro]]
ONE_WAY_MESSAGE(EmailEventTypes,EmailCreateStatus)

} // end of SEGSEvent namespace
