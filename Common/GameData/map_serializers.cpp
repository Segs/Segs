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

#include "map_serializers.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

#include "map_definitions.h"
#include "DataStorage.h"

bool loadFrom(BinStore *s,Map_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->read(target.Name);
    ok &= s->read(target.Icon);
    ok &= s->read(target.Location);
    ok &= s->read(target.TextLocation);
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    return ok && s->end_encountered();
}

bool loadFrom(BinStore *s, AllMaps_Data &target)
{
    bool ok = true;
    s->prepare();
    ok &= s->prepare_nested(); // will update the file size left
    assert(ok);
    if(s->end_encountered())
        return ok;
    QByteArray _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if("Zone"==_name) {
            Map_Data nt;
            ok &= loadFrom(s,nt);
            target.push_back(nt);
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    return ok;
}

template<class Archive>
static void serialize(Archive & archive, Map_Data & m)
{
    archive(cereal::make_nvp("Name",m.Name));
    archive(cereal::make_nvp("Icon",m.Icon));
    archive(cereal::make_nvp("Location",m.Location));
    archive(cereal::make_nvp("TextLocation",m.TextLocation));
}

void saveTo(const AllMaps_Data &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"AllZones",baseName,text_format);
}

//! @}
