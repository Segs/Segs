#include "RuntimeData.h"

#include "Prefab.h"
#include "Texture.h"
#include "Components/Logging.h"
#include "GameData/trick_definitions.h"
#include "GameData/trick_serializers.h"
#include "GameData/DataStorage.h"
#include "Components/serialization_common.h"

#include <QDirIterator>
#include <QString>

using namespace SEGS;
namespace {
static void setupTexOpt(SceneModifiers *mods,TextureModifiers *tmod)
{
    if(tmod->ScaleST0.x == 0.0f)
        tmod->ScaleST0.x = 1.0f;
    if(tmod->ScaleST0.y == 0.0f)
        tmod->ScaleST0.y = 1.0f;
    if(tmod->ScaleST1.x == 0.0f)
        tmod->ScaleST1.x = 1.0f;
    if(tmod->ScaleST1.y == 0.0f)
        tmod->ScaleST1.y = 1.0f;
    if(tmod->Fade.x != 0.0f || tmod->Fade.y != 0.0f)
        tmod->Flags |= uint32_t(TexOpt::FADE);
    if(!tmod->Blend.isEmpty())
        tmod->Flags |= uint32_t(TexOpt::DUAL);
    if(!tmod->Surface.isEmpty())
    {
        //qCDebug(logSceneGraph) << "Has surface" << tex->Surface;
    }

    tmod->name = tmod->name.mid(0,tmod->name.lastIndexOf('.')); // cut last extension part
    if(tmod->name.startsWith('/'))
        tmod->name.remove(0,1);
    QString lower_name = tmod->name.toLower();
    auto iter = mods->m_texture_path_to_mod.find(lower_name);
    if(iter!=mods->m_texture_path_to_mod.end())
    {
        qCDebug(logSceneGraph) << "Duplicate texture info: " << tmod->name;
        return;
    }
    mods->m_texture_path_to_mod[lower_name] = tmod;
}
static void setupTrick(SceneModifiers *mods,GeometryModifiers *gmod)
{
    if(gmod->node.TintColor0.rgb_are_zero())
        gmod->node.TintColor0 = RGBA(0xFFFFFFFF);
    if(gmod->node.TintColor1.rgb_are_zero())
        gmod->node.TintColor1 = RGBA(0xFFFFFFFF);
    gmod->AlphaRef /= 255.0f;
    if(gmod->ObjTexBias != 0.0f)
        gmod->node._TrickFlags |= TexBias;
    if(gmod->AlphaRef != 0.0f)
        gmod->node._TrickFlags |= AlphaRef;
    if(gmod->FogDist.x != 0.0f || gmod->FogDist.y != 0.0f)
        gmod->node._TrickFlags |= FogHasStartAndEnd;
    if(gmod->ShadowDist != 0.0f)
        gmod->node._TrickFlags |= CastShadow;
    if(gmod->NightGlow.x != 0.0f || gmod->NightGlow.y != 0.0f)
        gmod->node._TrickFlags |= NightGlow;
    if(gmod->node.ScrollST0.x != 0.0f || gmod->node.ScrollST0.y != 0.0f)
        gmod->node._TrickFlags |= ScrollST0;
    if(gmod->node.ScrollST1.x != 0.0f || gmod->node.ScrollST1.y != 0.0f)
        gmod->node._TrickFlags |= ScrollST1;
    if(!gmod->StAnim.empty())
    {
        //        if(setStAnim(&a1->StAnim.front()))
        //            a1->node._TrickFlags |= STAnimate;
    }
    if(gmod->GroupFlags & VisTray)
        gmod->ObjFlags |= 0x400;
    if(gmod->name.isEmpty())
        qCDebug(logSceneGraph) << "No name in trick";
    QString lower_name = gmod->name.toLower();
    auto iter = mods->g_tricks_string_hash_tab.find(lower_name);
    if(iter!=mods->g_tricks_string_hash_tab.end())
    {
        qCDebug(logSceneGraph) << "duplicate model trick!";
        return;
    }
    mods->g_tricks_string_hash_tab[lower_name]=gmod;
}

static void trickLoadPostProcess(SceneModifiers *mods)
{
    mods->m_texture_path_to_mod.clear();
    mods->g_tricks_string_hash_tab.clear();
    for(TextureModifiers &texopt : mods->texture_mods)
        setupTexOpt(mods,&texopt);
    for(GeometryModifiers &trickinfo : mods->geometry_mods)
        setupTrick(mods,&trickinfo);
}

template<class TARGET,unsigned int CRC>
bool read_data_to(FSWrapper &fs, const QString &directory_path, const QString &storage, TARGET &target)
{
    QDebug deb = qDebug().noquote().nospace();
    deb << "Reading " << directory_path << storage << " ... ";
    BinStore bin_store;
    if(!bin_store.open(fs,directory_path+storage,CRC))
    {
        deb << "failure";
        qWarning().noquote() << "Couldn't load" << storage << "from" << directory_path;
        qWarning().noquote() << "Using piggtool, ensure that bin.pigg has been extracted to ./data/";
        return false;
    }

    bool res=loadFrom(&bin_store,target);
    if(res)
        deb << "OK";
    else
    {
        deb << "failure";
        qWarning().noquote() << "Couldn't load" << directory_path<<storage<<": wrong file format?";
    }

    return res;
}

} // end of anonymous namespace


namespace SEGS
{

//void preloadTextureNames(FSWrapper *fs,const QByteArray &basepath)
//{
//    RuntimeData &rd(getRuntimeData());
//    //TODO: store texture headers into an array, and only rescan directories when forced ?
//    QDirIterator iter(basepath + "texture_library", QDir::Files, QDirIterator::Subdirectories);
//    while(iter.hasNext())
//    {
//        QByteArray fpath = iter.next().toUtf8();
//        QByteArray texture_key = iter.fileInfo().baseName().toLower().toUtf8();
//        rd.m_texture_paths[texture_key] = fpath;
//        loadTexHeader(fs,fpath);
//    }
//}

} //end of SEGS namespace

bool RuntimeData::read_model_modifiers(const QByteArray &directory_path)
{
    if(m_modifiers)
        return true;
    SceneModifiers tricks_store;
    assert(m_wrapper);
    if(!read_data_to<SceneModifiers,tricks_i0_requiredCrc>(*m_wrapper,directory_path,"bin/tricks.bin", tricks_store))
    {
        return false;
    }
    m_modifiers = new SceneModifiers;
    *m_modifiers = std::move(tricks_store);
    trickLoadPostProcess(m_modifiers);
    return true;
}

bool RuntimeData::prepare(FSWrapper* fs, const QByteArray &directory_path)
{
    m_wrapper = fs;
    m_ready = false;
    if(!read_prefab_definitions(directory_path))
        return false;
    if(!read_model_modifiers(directory_path))
        return false;

    m_ready = true;
    return true;
}

bool RuntimeData::read_prefab_definitions(const QByteArray &directory_path)
{
    if(!m_prefab_mapping)
        m_prefab_mapping = new SEGS::PrefabStore(m_wrapper,directory_path);
    return m_prefab_mapping->prepareGeoLookupArray(directory_path);
}

using namespace SEGS;
RuntimeData &getRuntimeData()
{
    static RuntimeData instance;
    return instance;
}
