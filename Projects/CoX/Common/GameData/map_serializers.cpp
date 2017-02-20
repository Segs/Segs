#include "map_serializers.h"
#include "serialization_common.h"

#include "map_definitions.h"
#include "DataStorage.h"

//static Serialization_Template Map_Tokens[6] = {
//{ "Name", TOKEN_STRING, 0x10},
//{ "Icon",  TOKEN_STRING, 0x14},
//{ "Location",  TOKEN_VEC2, 0},
//{ "TextLocation",  TOKEN_VEC2, 8},
//{ "End",  TOKEN_END},
//{ },
//};

//static const Serialization_Template Map_Token[2] = {
//{ "Zone", TOKEN_SUB_TABLE, 0, 0x18, Map_Tokens},
//{},
//};

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
    archive(cereal::make_nvp("Name",m.Name));
    archive(cereal::make_nvp("Icon",m.Icon));
    archive(cereal::make_nvp("Location",m.Location));
    archive(cereal::make_nvp("TextLocation",m.TextLocation));
}

void saveTo(const AllMaps_Data &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"AllZones",baseName,text_format);
}
