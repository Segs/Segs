#include "CohTextureConverter.h"
#include "CoHSceneConverter.h"

#include "Common/GameData/trick_definitions.h"

#include <Lutefisk3D/Graphics/Texture.h>
#include <Lutefisk3D/Graphics/Texture2D.h>
#include <Lutefisk3D/Core/Context.h>
#include <Lutefisk3D/IO/VectorBuffer.h>
#include <Lutefisk3D/Resource/ResourceCache.h>
#include <Lutefisk3D/Resource/Image.h>

#include <QDirIterator>
#include <QDebug>

extern QString basepath;

using namespace Urho3D;

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

QHash<QString, QString> s_texture_paths;
QSet<QString> s_missing_textures;
QHash<QString, TextureWrapper > s_loaded_textures;

TextureModifiers *modFromTextureName(const QString &texpath)
{
    QVector<QStringRef> split = texpath.splitRef("/");
    while(!split.empty())
    {
        if(0==split.front().compare(QLatin1String("texture_library"))) {
            split.pop_front();
            break;
        }
        split.pop_front();
    }
    return nullptr;
}
void loadTexHeader(const QString &fname)
{
    TextureWrapper res;
    QFileInfo tex_path(fname);
    QString lookupstring=tex_path.baseName().toLower();
    const QString &actualPath(s_texture_paths[lookupstring]);
    if(actualPath.isEmpty()) {
        if(!s_missing_textures.contains(lookupstring)) {
            qDebug() << "Missing texture" << fname;
            s_missing_textures.insert(lookupstring);
        }
        return;
    }
    QFileInfo actualFile(actualPath);
    QFile src_tex(actualPath);
    if (src_tex.exists() && src_tex.open(QFile::ReadOnly)) {
        TexFileHdr hdr;
        src_tex.read((char *)&hdr, sizeof(TexFileHdr));
        if (0 == memcmp(hdr.magic, "TX2", 3)) {
            if(hdr.alpha)
                res.flags |= TextureWrapper::ALPHA;
        }
    }
    res.info = modFromTextureName(actualFile.path()+"/"+actualFile.baseName());
    uint32_t texopt_flags = 0;
    if (res.info)
        texopt_flags = res.info->Flags;
    if (fname.contains("PLAYERS/",Qt::CaseInsensitive) ||
            fname.contains("ENEMIES/",Qt::CaseInsensitive) ||
            fname.contains("NPCS/",Qt::CaseInsensitive))
        res.flags |= TextureWrapper::BUMPMAP_MIRROR | TextureWrapper::CLAMP;

    if (fname.contains("MAPS/",Qt::CaseInsensitive))
        res.flags |= TextureWrapper::CLAMP;

    if (texopt_flags & REPLACEABLE)
        res.flags |= TextureWrapper::REPLACEABLE;

    if (texopt_flags & BUMPMAP)
        res.flags |= TextureWrapper::BUMPMAP;

    res.scaleUV0 = {1,1};
    res.scaleUV1 = {1,1};

    if (res.info && !res.info->BumpMap.isEmpty())
        res.bumpmap = res.info->BumpMap;
    QString detailname;
    if (texopt_flags & DUAL)
    {
        res.detailname = res.info->Blend;
        if (!res.detailname.isEmpty())
        {
            res.flags |= TextureWrapper::DUAL;
            res.BlendType = CoHBlendMode(res.info->BlendType);
            res.scaleUV0 = {res.info->ScaleST0.x,res.info->ScaleST0.y};
            res.scaleUV1 = {res.info->ScaleST1.x,res.info->ScaleST1.y};

            if (res.BlendType == CoHBlendMode::ADDGLOW && 0==res.detailname.compare("grey",Qt::CaseInsensitive))
            {
                res.detailname = "black";
            }
            s_loaded_textures[lookupstring] = res;
            return;
        }
        else
        {
            qDebug() << "Detail texture "<<res.info->Blend<<" does not exist for texture mod"<<res.info->name;
            detailname = "grey";
        }
    }
    else if (lookupstring.compare("invisible")==0)
    {
        detailname = "invisible";
    }
    else
    {
        detailname = "grey";
    }
    res.detailname = detailname;
    if (res.BlendType == CoHBlendMode::ADDGLOW && 0==res.detailname.compare("grey",Qt::CaseInsensitive))
    {
        res.detailname = "black";
    }
    s_loaded_textures[lookupstring] = res;
}
}
void preloadTextureNames()
{
    QDirIterator iter(basepath+"texture_library", QDir::Files, QDirIterator::Subdirectories);
    while (iter.hasNext()) {
        QString fpath = iter.next();
        QFileInfo fi(iter.fileInfo());
        s_texture_paths[fi.baseName().toLower()] = fpath;
        loadTexHeader(fpath);
    }
}
