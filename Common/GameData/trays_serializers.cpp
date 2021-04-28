/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "trays_serializers.h"

#include "GameData/Powers.h"
#include "power_serializers.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

#include "Components/Logging.h"

void saveTo(const PowerTrayGroup &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"PowerTrayGroups",baseName,text_format);
}
void serializeToDb(const PowerTrayGroup &data, QString &tgt)
{
    std::ostringstream ostr;
    {
        cereal::JSONOutputArchive ar(ostr);
        ar(data);
    }
    tgt = QString::fromStdString(ostr.str());
}

void serializeFromDb(PowerTrayGroup &data, const QString &src)
{
    if(src.isEmpty())
        return;
    std::istringstream istr;
    istr.str(src.toStdString());
    {
        cereal::JSONInputArchive ar(istr);
        ar(data);
    }
}
//! @}
