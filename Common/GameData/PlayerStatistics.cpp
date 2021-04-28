/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "PlayerStatistics.h"
#include "Components/Logging.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"


template<class Archive>
void PlayerStatistics::serialize(Archive &archive, uint32_t const version)
 {
     if(version != PlayerStatistics::class_version)
     {
         qCritical() << "Failed to serialize Souvenir, incompatible serialization format version " << version;
         return;
     }
     archive(cereal::make_nvp("HideAndSeek",m_hide_seek));
     archive(cereal::make_nvp("RelayRaces",m_relay_races));
     archive(cereal::make_nvp("Hunts",m_hunts));
 }

 CEREAL_CLASS_VERSION(PlayerStatistics, PlayerStatistics::class_version)
 SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(PlayerStatistics)

 template<class Archive>
 void HideAndSeek::serialize(Archive &archive, uint32_t const version)
  {
      if(version != HideAndSeek::class_version)
      {
          qCritical() << "Failed to serialize Souvenir, incompatible serialization format version " << version;
          return;
      }
      archive(cereal::make_nvp("Count",m_found_count));
  }

  CEREAL_CLASS_VERSION(HideAndSeek, HideAndSeek::class_version)
  SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(HideAndSeek)

  template<class Archive>
  void RelayRaceResult::serialize(Archive &archive, uint32_t const version)
   {
       if(version != RelayRaceResult::class_version)
       {
           qCritical() << "Failed to serialize Souvenir, incompatible serialization format version " << version;
           return;
       }
       archive(cereal::make_nvp("Segment",m_segment));
       archive(cereal::make_nvp("LastTime",m_last_time));
       archive(cereal::make_nvp("BestTime",m_best_time));
   }

   CEREAL_CLASS_VERSION(RelayRaceResult, RelayRaceResult::class_version)
   SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(RelayRaceResult)


   template<class Archive>
   void Hunt::serialize(Archive &archive, uint32_t const version)
    {
        if(version != Hunt::class_version)
        {
            qCritical() << "Failed to serialize Souvenir, incompatible serialization format version " << version;
            return;
        }
        archive(cereal::make_nvp("Type",m_type));
        archive(cereal::make_nvp("Count",m_count));
    }

    CEREAL_CLASS_VERSION(Hunt, Hunt::class_version)
    SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(Hunt)
