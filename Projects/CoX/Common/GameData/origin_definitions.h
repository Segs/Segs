/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef PARSE_ORIGIN_H
#define PARSE_ORIGIN_H
#include <vector>
#include <QtCore/QString>

struct Parse_Origin
{
    QByteArray Name;
    QByteArray DisplayName;
    QByteArray DisplayHelp;
    QByteArray DisplayShortHelp;
    int NumBonusPowerSets;
    int NumBonusPowers;
    int NumBonusBoostSlots;
    int NumContacts;
    float ContactBonusLength;
};
using Parse_AllOrigins = std::vector<Parse_Origin>;

#endif // PARSE_ORIGIN_H
