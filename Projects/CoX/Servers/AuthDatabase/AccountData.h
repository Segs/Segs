#pragma once

#include <QDateTime>
#include <QString>
#include <stdint.h>

struct AuthAccountData
{
    QString   m_login;
    uint8_t   m_access_level;
    uint64_t  m_acc_server_acc_id=0;
    QDateTime m_creation_date;
    bool valid() const { return m_acc_server_acc_id!=0; }
    bool isBlocked() const { return m_access_level == 0; }
};
