/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "Clue.h"
#include "Components/Logging.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"


template<class Archive>
void Souvenir::serialize(Archive &archive, uint32_t const version)
 {
     if(version != Souvenir::class_version)
     {
         qCritical() << "Failed to serialize Souvenir, incompatible serialization format version " << version;
         return;
     }
     archive(cereal::make_nvp("idx",m_idx));
     archive(cereal::make_nvp("Name",m_name));
     archive(cereal::make_nvp("Icon",m_icon));
     archive(cereal::make_nvp("Description",m_description));
 }

 CEREAL_CLASS_VERSION(Souvenir, Souvenir::class_version)   // register Souvenir class version
 SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(Souvenir)

 template<class Archive>
 void Clue::serialize(Archive &archive, uint32_t const version)
 {
     if(version != Clue::class_version)
     {
         qCritical() << "Failed to serialize Clue, incompatible serialization format version " << version;
         return;
     }
     archive(cereal::make_nvp("Name",m_name));
     archive(cereal::make_nvp("DisplayName",m_display_name));
     archive(cereal::make_nvp("DetailText",m_detail_text));
     archive(cereal::make_nvp("IconFile",m_icon_file));
 }

 CEREAL_CLASS_VERSION(Clue, Clue::class_version)   // register Clue class version
 SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(Clue)
