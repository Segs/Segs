/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
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
