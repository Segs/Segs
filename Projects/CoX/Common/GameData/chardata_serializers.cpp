/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "chardata_serializers.h"
#include "GameData/CharacterData.h"
#include "serialization_common.h"
#include "serialization_types.h"
#include "GameData/Powers.h"

#include "DataStorage.h"
#include "trays_serializers.h"
#include "attrib_serializers.h"
#include <QDebug>

#include "Logging.h"


void saveTo(const CharacterData &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"CharacterData",baseName,text_format);
}


//! @}
