/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "PasswordHasher.h"
#include <QTime>

PasswordHasher::PasswordHasher() : m_hasher(QCryptographicHash::Sha256)
{

}

QString PasswordHasher::getRandomString(int length) const
{
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const int randomStringLength = length;
    qsrand(static_cast<quint64>(QTime::currentTime().msecsSinceStartOfDay()));

    QString randomString;
    for(int i = 0; i < randomStringLength; ++i)
    {
        int index = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }
    return randomString;
}

/*!
 * \brief Generates a random salt of length 16.
 * \return A QByteArray of length 16 containing the generated salt.
 */
QByteArray PasswordHasher::generateSalt()
{
    QString salt = getRandomString(16);
    return salt.toUtf8();
}

/*!
 * \brief Hashes a password with the given salt using the Sha256 algorithm.
 * \param pass The password to be hashed.
 * \param salt The salt to be appended to the password.
 * \return A QByteArray containing the salted and hashed password.
 */
QByteArray PasswordHasher::hashPassword(const QByteArray &pass, const QByteArray &salt)
{
    QByteArray pass_array(pass+salt);
    QByteArray hashed_pass_array = QCryptographicHash::hash(pass_array, QCryptographicHash::Sha256);
    return hashed_pass_array;
}

//! @}
