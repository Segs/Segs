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
    int ContactBonusLength;
};
typedef std::vector<Parse_Origin> Parse_AllOrigins;

#endif // PARSE_ORIGIN_H
