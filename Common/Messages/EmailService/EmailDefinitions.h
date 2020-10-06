/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <QtCore/QHash>
#include <vector>
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

struct EmailData
{
    uint32_t m_sender_id;
    uint32_t m_recipient_id;
    QString m_sender_name;          // take this from db during runtime based on sender_id?
    QString m_subject;
    QString m_message;
    uint32_t m_timestamp;
    bool m_is_read_by_recipient = false;
};

template<class Archive>
static void serialize(Archive & archive, EmailData & src)
{
    // sender id and recipient id are outside the blob
    archive(cereal::make_nvp("SenderName",src.m_sender_name));
    archive(cereal::make_nvp("Subject",src.m_subject));
    archive(cereal::make_nvp("Message",src.m_message));
    archive(cereal::make_nvp("TimeStamp",src.m_timestamp));
    archive(cereal::make_nvp("IsRead",src.m_is_read_by_recipient));
}

struct EmailHeaderData
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

// used by GameDbSyncEvent
struct EmailResponseData
{
    uint32_t m_email_id;
    uint32_t m_sender_id;
    uint32_t m_recipient_id;
    QString m_cerealized_email_data;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_email_id, m_sender_id, m_recipient_id, m_cerealized_email_data);
    }
};
