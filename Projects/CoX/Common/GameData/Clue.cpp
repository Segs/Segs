/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "Clue.h"
#include "Logging.h"
#include "serialization_common.h"
#include "serialization_types.h"


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

 template<class Archive>
 void ClueSouvenirList::serialize(Archive &archive, uint32_t const version)
 {
     if(version != ClueSouvenirList::class_version)
     {
         qCritical() << "Failed to serialize ClueSouvenirList, incompatible serialization format version " << version;
         return;
     }
     archive(cereal::make_nvp("clueList",m_clue_list));
     archive(cereal::make_nvp("souvenirList",m_souvenir_list));
 }

 CEREAL_CLASS_VERSION(ClueSouvenirList, ClueSouvenirList::class_version)   // register Clue class version
 SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(ClueSouvenirList)
