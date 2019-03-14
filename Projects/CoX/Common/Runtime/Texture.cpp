#include "Texture.h"

#include "RuntimeData.h"
#include "Logging.h"
#include "Common/GameData/GameDataStore.h"
#include "Common/GameData/trick_definitions.h"

#include <QFileInfo>
#include <QSet>
#include <QVector>
#include <QDebug>

using namespace SEGS;

namespace
{
#pragma pack(push, 1)
struct TexFileHdr
{
    int     header_size;
    int     file_size;
    int     wdth;
    int     hght;
    int     flags;
    int     fade[2];
    uint8_t alpha;
    char    magic[3];
};
#pragma pack(pop)

QSet<QString>                  s_missing_textures;
///
/// \brief Will split the \arg texpath into directories, and finds the closest TextureModifiers
/// that matches a directory
/// \param texpath contains a full path to the texture
/// \return texture modifier object, if any
///
TextureModifiers *modFromTextureName(const QString &texpath)
{
    RuntimeData &rd(getRuntimeData());
    QVector<QStringRef> split = texpath.splitRef("/");
    while(!split.empty())
    {
        if(0==split.front().compare(QLatin1String("texture_library"))) {
            split.pop_front();
            break;
        }
        split.pop_front();
    }
    SceneModifiers *mods = rd.m_modifiers;
    assert(mods);
    const QHash<QString,TextureModifiers *> &texmods(mods->m_texture_path_to_mod);
    // scan from the back of the texture path, until a modifier is found.
    while(!split.empty())
    {
        auto val = texmods.value(split.back().toString().toLower(),nullptr);
        if(val)
            return val;
        split.pop_back();
    }
    return nullptr;
}
}

namespace SEGS
{

void loadTexHeader(const QString &fname)
{
    RuntimeData &rd(getRuntimeData());
    TextureWrapper res;
    QFileInfo tex_path(fname);
    QString lookupstring=tex_path.baseName().toLower();
    const QString &actualPath(rd.m_texture_paths[lookupstring]);
    if(actualPath.isEmpty())
    {
        if(!s_missing_textures.contains(lookupstring))
        {
            qCDebug(logSceneGraph) << "Missing texture" << fname;
            s_missing_textures.insert(lookupstring);
        }
        return;
    }
    QFileInfo actualFile(actualPath);
    QFile src_tex(actualPath);
    if(src_tex.exists() && src_tex.open(QFile::ReadOnly))
    {
        TexFileHdr hdr;
        src_tex.read((char *)&hdr, sizeof(TexFileHdr));
        if(0 == memcmp(hdr.magic, "TX2", 3))
        {
            if(hdr.alpha)
                res.flags |= TextureWrapper::ALPHA;
        }
    }
    res.info = modFromTextureName(actualFile.path()+"/"+actualFile.baseName());
    uint32_t texopt_flags = 0;
    if(res.info)
        texopt_flags = res.info->Flags;
    if(fname.contains("PLAYERS/",Qt::CaseInsensitive) ||
            fname.contains("ENEMIES/",Qt::CaseInsensitive) ||
            fname.contains("NPCS/",Qt::CaseInsensitive))
        res.flags |= TextureWrapper::BUMPMAP_MIRROR | TextureWrapper::CLAMP;

    if(fname.contains("MAPS/",Qt::CaseInsensitive))
        res.flags |= TextureWrapper::CLAMP;

    if(texopt_flags & REPLACEABLE)
        res.flags |= TextureWrapper::REPLACEABLE;

    if(texopt_flags & BUMPMAP)
        res.flags |= TextureWrapper::BUMPMAP;

    res.scaleUV0 = {1,1};
    res.scaleUV1 = {1,1};

    if(res.info && !res.info->BumpMap.isEmpty())
        res.bumpmap = res.info->BumpMap;
    QString detailname;
    if(texopt_flags & DUAL)
    {
        if(!res.info->Blend.isEmpty())
        {
            res.flags |= TextureWrapper::DUAL;
            res.BlendType = CoHBlendMode(res.info->BlendType);
            res.scaleUV0 = {res.info->ScaleST0.x,res.info->ScaleST0.y};
            res.scaleUV1 = {res.info->ScaleST1.x,res.info->ScaleST1.y};
            res.detailname = res.info->Blend;

            if(res.BlendType == CoHBlendMode::ADDGLOW && 0==res.detailname.compare("grey",Qt::CaseInsensitive))
            {
                res.detailname = "black";
            }
            // copy the 'res' into the handle based storage, and record the handle
            rd.m_loaded_textures[lookupstring] = TextureStorage::instance().create(res);
            return;
        }
        qCDebug(logSceneGraph) << "Detail texture " << res.info->Blend << " does not exist for texture mod" << res.info->name;
        detailname = "grey";
    }
    else if(lookupstring.compare("invisible")==0)
    {
        detailname = "invisible";
    }
    else
    {
        detailname = "grey";
    }
    if(res.BlendType == CoHBlendMode::ADDGLOW && 0==detailname.compare("grey",Qt::CaseInsensitive))
    {
        detailname = "black";
    }
    res.detailname = detailname;
    // copy the 'res' into the handle based storage, and record the handle
    rd.m_loaded_textures[lookupstring] = TextureStorage::instance().create(res);
}
}
