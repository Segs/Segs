/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include "GameCommandList.h"

#include <QtCore/QString>

class EmailMessageStatus final : public GameCommand
{
public:
    EmailMessageStatus(const int status, const QString &recipient) : GameCommand(MapEventTypes::evEmailMsgStatus),
        m_status(status),
        m_recipient(recipient)
    {
    }

    void    serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1, type()-MapEventTypes::evFirstServerToClient);
        bs.StorePackedBits(1, status);
        bs.StoreString(m_recipient);
    }

    void    serializefrom(BitStream &src);

protected:
    int m_status;
    QString m_recipient;
};
