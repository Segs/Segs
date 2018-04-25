/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QCryptographicHash>

class PasswordHasher
{
public:
    PasswordHasher();
    QByteArray generateSalt();
    QByteArray hashPassword(const QByteArray &pass, const QByteArray &salt);

protected:
    QString getRandomString(int length) const;

    QCryptographicHash m_hasher;
};
