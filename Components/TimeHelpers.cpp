/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components Components
 * @{
 */

#include "TimeHelpers.h"
#include <QDateTime>

int64_t getSecsSince2000Epoch()
{
    QDateTime base_date(QDate(2000,1,1),QTime(0,0),Qt::UTC);
    return base_date.secsTo(QDateTime::currentDateTimeUtc());
}
//! @}
