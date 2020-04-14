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

#include "ConfigExtension.h"
#include <ace/INET_Addr.h>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <ace/INET_Addr.h>

bool parseAddress(const QString &src,ACE_INET_Addr &tgt)
{
    QStringList parts = src.trimmed().split(':');
    if(parts.size()!=2)
        return false;
    tgt.set(parts[1].toUShort(),qPrintable(parts[0]));
    return true;
}

//! @}
