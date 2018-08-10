/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommandList.h"

#include <QtCore/QString>

namespace SEGSEvents
{

class EmailHeaders final : public GameCommand
{
public:
    struct EmailHeader {
        int id;
        QString sender;
        QString subject;
        int timestamp;
    };

    /*Send multiple emails*/
    EmailHeaders(QVector<EmailHeader> &email) : GameCommand(MapEventTypes::evEmailHeadersCmd),
        m_fullupdate(true),
        m_emails(email)
    {
    }

    /*Defines a single email header to send*/
    EmailHeaders(const int &id, const QString &sender,  const QString &subject, const int &timestamp) : GameCommand(MapEventTypes::evEmailHeadersCmd),
        m_fullupdate(false)
    {
        m_emails.push_back(EmailHeader{ id, sender, subject, timestamp });
    }

    void    serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1, type()-MapEventTypes::evFirstServerToClient);
        bs.StorePackedBits(1, m_fullupdate);
        bs.StorePackedBits(1, m_emails.size());

        for(const EmailHeader &hdr : m_emails){
            bs.StoreBits(1, true); //"valid" flag
            bs.StoreBits(32, hdr.id);
            bs.StoreString(hdr.sender);
            bs.StoreString(hdr.subject);
            bs.StoreBits(32, hdr.timestamp);
        }
    }
    void    serializefrom(BitStream &src);

    // [[ev_def:field]]
    bool    m_fullupdate; //Forces a refresh of the email window
    // [[ev_def:field]]
    QVector<EmailHeader> m_emails;
};
} // end of SEGSEvents namespace

