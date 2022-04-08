/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <vector>
#include <QtCore/QString>

struct Parse_Origin
{
    QByteArray Name;
    QByteArray DisplayName;
    QByteArray DisplayHelp;
    QByteArray DisplayShortHelp;
    // Those fields below, are missing in I24
    // probably since all origins are the same in regards to those settings.
    int NumBonusPowerSets=0;
    int NumBonusPowers=0;
    int NumBonusBoostSlots=0;
    int NumContacts=0;
    float ContactBonusLength=0.0f;
};
using Parse_AllOrigins = std::vector<Parse_Origin>;
