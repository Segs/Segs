/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef PARSE_ORIGIN_H
#define PARSE_ORIGIN_H
#include <vector>
#include <QtCore/QString>

struct Parse_Origin
{
    QString Name;
    QString DisplayName;
    QString DisplayHelp;
    QString DisplayShortHelp;
    int NumBonusPowerSets;
    int NumBonusPowers;
    int NumBonusBoostSlots;
    int NumContacts;
    float ContactBonusLength;
};
typedef std::vector<Parse_Origin> Parse_AllOrigins;

#endif // PARSE_ORIGIN_H
