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

#include "clientoptions_serializers.h"

#include "clientoptions_definitions.h"
#include "DataStorage.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

// register Client Options class version

void saveTo(const ClientOptions &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"ClientOptions",baseName,text_format);
}

SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(ClientOptions)

//! @}
