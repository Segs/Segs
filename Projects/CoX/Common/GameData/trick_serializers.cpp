#include "trick_serializers.h"

#include "anim_serializers.h"
#include "serialization_common.h"
#include "trick_definitions.h"
#include "DataStorage.h"

namespace {
bool loadFrom(BinStore * s, Parser_Texture & target)
{
    bool ok=true;
    s->prepare();
    ok &= s->read(target.name);
    ok &= s->read(target.src_file);
    ok &= s->read(target.Gloss);
    ok &= s->read(target.Surface);
    ok &= s->read(target.Fade);
    ok &= s->read(target.ScaleST0);
    ok &= s->read(target.ScaleST1);
    ok &= s->read(target.Blend);
    ok &= s->read(target.BumpMap);
    ok &= s->read(target.BlendType);
    ok &= s->read(target.Flags);
    ok &= s->prepare_nested(); // will update the file size left
    assert(s->end_encountered());
    return ok;
}
bool loadFrom(BinStore * s, GeomTrick & target)
{
    bool ok=true;
    s->prepare();
    ok &= s->read(target.name);
    ok &= s->read(target.src_name);
    ok &= s->read(target.LodFar);
    ok &= s->read(target.LodFarFade);
    ok &= s->read(target.LodNear);
    ok &= s->read(target.LodNearFade);
    ok &= s->read(target.node._TrickFlags);
    ok &= s->read(target.ObjFlags);
    ok &= s->read(target.GfxFlags);
    ok &= s->read(target.GroupFlags);
    ok &= s->read(target.Sway);
    ok &= s->read(target.Sway_Rotate);
    ok &= s->read(target.AlphaRef);
    ok &= s->read(target.FogDist);
    ok &= s->read(target.node.SortBias);
    ok &= s->read(target.node.ScrollST0);
    ok &= s->read(target.node.ScrollST1);
    ok &= s->read(target.ShadowDist);
    ok &= s->read(target.NightGlow);
    ok &= s->read(target.node.TintColor0);
    ok &= s->read(target.node.TintColor1);
    ok &= s->read(target.ObjTexBias);
    ok &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("StAnim")==0) {
            target.StAnim.emplace_back();
            ok &= loadFrom(s,target.StAnim.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(s->end_encountered());
    return ok;
}
}


bool loadFrom(BinStore * s, AllTricks_Data *target)
{
    s->prepare();
    bool ok = s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return ok;
    QString _name;
    while(s->nesting_name(_name))
    {
        s->nest_in();
        if(_name.compare("Trick")==0) {
            target->geom_tricks.emplace_back();
            ok &= loadFrom(s,target->geom_tricks.back());
        } else if(_name.compare("Texture")==0) {
            target->texture_trick.emplace_back();
            ok &= loadFrom(s,target->texture_trick.back());
        } else
            assert(!"unknown field referenced.");
        s->nest_out();
    }
    assert(ok);
    return ok;
}

template<class Archive>
static void serialize(Archive & archive, Parser_Texture & m)
{
    archive(cereal::make_nvp("name",m.name));
    archive(cereal::make_nvp("src_file",m.src_file));
    archive(cereal::make_nvp("Gloss",m.Gloss));
    archive(cereal::make_nvp("Surface",m.Surface));
    archive(cereal::make_nvp("Fade",m.Fade));
    archive(cereal::make_nvp("ScaleST0",m.ScaleST0));
    archive(cereal::make_nvp("ScaleST1",m.ScaleST1));
    archive(cereal::make_nvp("Blend",m.Blend));
    archive(cereal::make_nvp("BumpMap",m.BumpMap));
    archive(cereal::make_nvp("BlendType",m.BlendType));
    archive(cereal::make_nvp("Flags",m.Flags));
}
template<class Archive>
static void serialize(Archive & archive, TrickNode & m)
{
    archive(cereal::make_nvp("TrickFlags",m._TrickFlags));
    archive(cereal::make_nvp("SortBias",m.SortBias));
    archive(cereal::make_nvp("ScrollST0",m.ScrollST0));
    archive(cereal::make_nvp("ScrollST1",m.ScrollST1));
    archive(cereal::make_nvp("TintColor0",m.TintColor0));
    archive(cereal::make_nvp("TintColor1",m.TintColor1));
}
template<class Archive>
static void serialize(Archive & archive, GeomTrick & m)
{
    archive(cereal::make_nvp("Name",m.name));
    archive(cereal::make_nvp("SrcName",m.src_name));
    archive(cereal::make_nvp("LodFar",m.LodFar));
    archive(cereal::make_nvp("LodFarFade",m.LodFarFade));
    archive(cereal::make_nvp("LodNear",m.LodNear));
    archive(cereal::make_nvp("LodNearFade",m.LodNearFade));
    archive(cereal::make_nvp("ObjFlags",m.ObjFlags));
    archive(cereal::make_nvp("GfxFlags",m.GfxFlags));
    archive(cereal::make_nvp("GroupFlags",m.GroupFlags));
    archive(cereal::make_nvp("Sway",m.Sway));
    archive(cereal::make_nvp("Sway_Rotate",m.Sway_Rotate));
    archive(cereal::make_nvp("AlphaRef",m.AlphaRef));
    archive(cereal::make_nvp("FogDist",m.FogDist));
    archive(cereal::make_nvp("ShadowDist",m.ShadowDist));
    archive(cereal::make_nvp("NightGlow",m.NightGlow));
    archive(cereal::make_nvp("ObjTexBias",m.ObjTexBias));
    archive(cereal::make_nvp("Node",m.node));
    archive(cereal::make_nvp("TextureAnims",m.StAnim));
}

template<class Archive>
static void serialize(Archive & archive, AllTricks_Data & m)
{
    archive(cereal::make_nvp("Texture",m.texture_trick));
    archive(cereal::make_nvp("Geometry",m.geom_tricks));
}
void saveTo(const AllTricks_Data &target, const QString &baseName, bool text_format)
{
    commonSaveTo(target,"Tricks",baseName,text_format);
}
