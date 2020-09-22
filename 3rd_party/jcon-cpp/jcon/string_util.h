#pragma once

#include "jcon.h"

#include <QVariantList>

namespace jcon {

/**
 * Convert a QVariant to a QString, with non-empty representations of
 * QVariantMaps and QVariantLists. Useful for logging.
 *
 * @param[in] v The QVariant to convert
 *
 * @return A string representation of the QVariant.
 */
JCON_API QString variantToString(const QVariant& v);

/**
 * Convert a QVariantList to a QStringList
 *
 * @param[in] l The QVariantList to convert
 *
 * @return A list with the string representations of each element in the list.
 */
JCON_API QStringList variantListToStringList(const QVariantList& l);

}
