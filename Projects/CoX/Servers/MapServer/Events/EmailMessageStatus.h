#pragma once
#include "GameCommandList.h"

#include <QtCore/QString>

class EmailMessageStatus final : public GameCommand
{
public:
    EmailMessageStatus(const bool status, const QString &recipient) : GameCommand(MapEventTypes::evEmailMsgStatus),
        m_status(status),
        m_recipient(recipient)
    {
    }

    void    serializeto(BitStream &bs) const override {
        bs.StorePackedBits(1, type()-MapEventTypes::evFirstServerToClient);
        bs.StorePackedBits(1, m_status);
        if(!m_status)
            bs.StoreString(m_recipient);
    }

    void    serializefrom(BitStream &src);

protected:
    bool m_status;
    QString m_recipient;
};
