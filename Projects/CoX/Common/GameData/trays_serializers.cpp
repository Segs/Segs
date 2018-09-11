/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "trays_serializers.h"
//#include "trays_definitions.h"
#include "power_serializers.h"
#include "serialization_common.h"
#include "serialization_types.h"

#include "Logging.h"

const constexpr uint32_t PowerTrayItem::class_version;
const constexpr uint32_t PowerTray::class_version;
const constexpr uint32_t PowerTrayGroup::class_version;

template
void PowerTray::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, uint32_t const version);
template
void PowerTray::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, uint32_t const version);
template
void PowerTray::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive & archive,uint32_t const version);

void saveTo(const PowerTrayGroup &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"PowerTrayGroups",baseName,text_format);
}

template
void PowerTrayGroup::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, uint32_t const version);
template
void PowerTrayGroup::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, uint32_t const version);
template
void PowerTrayGroup::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive & archive, uint32_t const version);

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
