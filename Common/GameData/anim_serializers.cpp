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

#include "anim_serializers.h"

#include "anim_definitions.h"
#include "DataStorage.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

bool loadFrom(BinStore * s, TextureAnim_Data & target)
{
    bool ok=true;
    s->prepare();
    ok &= s->read(target.speed);
    ok &= s->read(target.stScale);
    ok &= s->read(target.scrollType);
    ok &= s->read(target.flags);
    ok &= s->prepare_nested(); // will update the file size left
    assert(s->end_encountered());
    return ok;
}

template<class Archive>
void serialize(Archive & archive, TextureAnim_Data & m)
{
    archive(cereal::make_nvp("Speed",m.speed));
    archive(cereal::make_nvp("Scale",m.stScale));
    archive(cereal::make_nvp("Type",m.scrollType));
    archive(cereal::make_nvp("flags",m.flags));
}

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, TextureAnim_Data & m);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, TextureAnim_Data & m);
template
void serialize<cereal::VectorInputArchive>(cereal::VectorInputArchive & archive, TextureAnim_Data & m);
template
void serialize<cereal::VectorOutputArchive>(cereal::VectorOutputArchive & archive, TextureAnim_Data & m);

//! @}
