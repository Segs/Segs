#include "string_util.h"

#include <QString>

namespace jcon {

QString variantToString(const QVariant& v)
{
    if (v.type() == QVariant::List) {
        return QString("list (%1 elements)").arg(v.toList().size());
    }
    if (v.type() == QVariant::Map) {
        return QString("map (%1 elements)").arg(v.toMap().size());
    }
    if (v.canConvert<QString>()) {
        return v.toString();
    }

    return ("N/A");
}

QStringList variantListToStringList(const QVariantList& l)
{
    QStringList res;
    std::transform(l.begin(), l.end(), std::back_inserter(res),
                   [](const QVariant& v) {
                       return variantToString(v);
                   });
    return res;
}

}
