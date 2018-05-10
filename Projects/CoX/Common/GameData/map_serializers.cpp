/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "map_serializers.h"
#include "serialization_common.h"

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
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Zone")==0) {
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
    try
    {
        archive(cereal::make_nvp("Name",m.Name));
        archive(cereal::make_nvp("Icon",m.Icon));
        archive(cereal::make_nvp("Location",m.Location));
        archive(cereal::make_nvp("TextLocation",m.TextLocation));
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << e.what();
    }
}

void saveTo(const AllMaps_Data &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"AllZones",baseName,text_format);
}

//! @}
