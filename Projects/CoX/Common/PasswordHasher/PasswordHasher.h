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
