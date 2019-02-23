/*
 * SEGS - Super Entity Game Server - dbtool
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "DBConnection.h"

#include "PasswordHasher.h"
#include "Logging.h"
#include "Settings.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

// Cereal adds a wrapper around the entire object
// with a key of `value0`. We simulate that here.
QJsonObject DBConnection::prepareBlob(QJsonObject &obj)
{
    QJsonObject wrapped_obj;
    wrapped_obj.insert("value0", obj);

    return wrapped_obj;
}
