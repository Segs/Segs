/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
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
