/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "VisitLocation.h"
#include "serialization_common.h"
#include "serialization_types.h"
#include "Logging.h"

template<class Archive>
void VisitLocation::serialize(Archive &archive, uint32_t const version)
{
    if(version != VisitLocation::class_version)
    {
        qCritical() << "Failed to serialize VisitLocation, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("name",m_location_name));
    archive(cereal::make_nvp("position",m_pos));
}

CEREAL_CLASS_VERSION(VisitLocation, VisitLocation::class_version)   // register VisitLocation class version
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(VisitLocation)
