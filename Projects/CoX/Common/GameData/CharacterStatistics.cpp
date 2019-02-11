/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "CharacterStatistics.h"
#include "Logging.h"
#include "serialization_common.h"
#include "serialization_types.h"


template<class Archive>
void CharacterStatistic::serialize(Archive &archive, uint32_t const version)
 {
     if(version != CharacterStatistic::class_version)
     {
         qCritical() << "Failed to serialize Souvenir, incompatible serialization format version " << version;
         return;
     }
      archive(cereal::make_nvp("Id",m_id));
     archive(cereal::make_nvp("Name",m_name));
     archive(cereal::make_nvp("Time",m_time));
     archive(cereal::make_nvp("count",m_count));
 }

 CEREAL_CLASS_VERSION(CharacterStatistic, CharacterStatistic::class_version)   // register Souvenir class version
 SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(CharacterStatistic)
