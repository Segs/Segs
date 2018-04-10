/*
 * Super Entity Game Server Project
 * http://github.com/Segs
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "GameCommandList.h"

#include <QtCore/QString>

class EmailHeaders final : public GameCommand
{
public:
    struct EmailHeader {
        int valid = true;
        int id;
        QString sender;
        QString subject;
        int timestamp;
    };

    /*Send multiple emails*/
    EmailHeaders(QVector<EmailHeader> &email) : GameCommand(MapEventTypes::evEmailHeadersCmd),
        m_fullupdate(true),
        m_count(email.size()),
        m_emails(email)
    {
    }

    /*Defines a single email header to send*/
    EmailHeaders(const int &id, const QString &sender,  const QString &subject, const int &timestamp) : GameCommand(MapEventTypes::evEmailHeadersCmd),
        m_fullupdate(false),
        m_count(1),
        m_id(id),
        m_sender(sender),
        m_subject(subject),
        m_timestamp(timestamp)
    {
    }

    void    serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1, type()-MapEventTypes::evFirstServerToClient);
        bs.StorePackedBits(1, m_fullupdate);
        bs.StorePackedBits(1, m_count);

        if(m_count > 1){
            for(int i = 0; i < m_count; ++i){
                bs.StoreBits(1, m_emails.at(i).valid);
                bs.StoreBits(32, m_emails.at(i).id);
                bs.StoreString(m_emails.at(i).sender);
                bs.StoreString(m_emails.at(i).subject);
                bs.StoreBits(32, m_emails.at(i).timestamp);
            }
        }else{
            bs.StoreBits(1, m_valid);
            bs.StoreBits(32, m_id);
            bs.StoreString(m_sender);
            bs.StoreString(m_subject);
            bs.StoreBits(32, m_timestamp);
        }
    };
    void    serializefrom(BitStream &src);

protected:
    int     m_fullupdate; //Basically forces a refresh of the email window
    int     m_count;
    int     m_valid = true; //Checked if the email exists, can probably use this to mark emails that are awaiting garbage collection
    int     m_id;
    QString m_sender;
    QString m_subject;
    int     m_timestamp; //# of seconds since Y2K, in relation to server defined timezone

    QVector<EmailHeader> m_emails;
};
