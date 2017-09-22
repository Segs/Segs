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

// for every directory in the texture's path we can hava a modifier.
QHash<QString,TextureModifiers *> g_texture_path_to_mod;
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
QHash<QString, QString>        s_texture_paths; // map from texture name to actual file path
QHash<QString, TextureWrapper> s_loaded_textures;

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
    // scan from the back of the texture path, until a modifier is found.
    while(!split.empty())
    {
        auto val = g_texture_path_to_mod.value(split.back().toString().toLower(),nullptr);
        if(val)
            return val;
        split.pop_back();
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
TextureWrapper tryLoadTexture(Urho3D::Context *ctx,const QString &fname)
{
    ResourceCache *rcache=ctx->m_ResourceCache.get();
    QFileInfo tex_path(fname);
    QString lookupstring=tex_path.baseName().toLower();
    QString actualPath = s_texture_paths.value(lookupstring);
    if(actualPath.isEmpty())
    {
        if(!s_missing_textures.contains(lookupstring))
        {
            qDebug() << "Missing texture" << fname;
            s_missing_textures.insert(lookupstring);
        }
        return TextureWrapper();
    }
    TextureWrapper &res(s_loaded_textures[lookupstring]);
    if(res.base) // we have an Urho3D texture already, nothing to do.
        return res;

    QFile src_tex(actualPath);
    if (!src_tex.exists() || !src_tex.open(QFile::ReadOnly))
    {
        qWarning() << actualPath<<" is not readable";
        return res;
    }
    TexFileHdr hdr;
    src_tex.read((char *)&hdr, sizeof(TexFileHdr));
    if (0 != memcmp(hdr.magic, "TX2", 3))
    {
        qWarning() << "Unrecognized texture format.";
        return res;
    }
    QString originalname = QString(src_tex.read(hdr.header_size - sizeof(TexFileHdr)));

    QDir converted_dir("./converted");
    QString converted_path(converted_dir.filePath(originalname));
    QFile tgt(converted_path);
    if (!tgt.exists() && tgt.open(QFile::WriteOnly))
    {
        // save extracted texture into a local directory
        converted_dir.mkpath(QFileInfo(originalname).path());
        QByteArray data = src_tex.readAll();
        VectorBuffer vbuf(data.data(),data.size());
        tgt.write(data);
        tgt.close();
        res.base = new Texture2D(ctx);
        if (res.base->BeginLoad(vbuf))
            res.base->EndLoad();
    }
    else // a pre-converted texture file exists, load it instead
        res.base = rcache->GetResource<Texture2D>(converted_path);
    return res;
}
void preloadTextureNames()
{
    //TODO: store texture headers into an array, and only rescan directories when forced ?
    QDirIterator iter(basepath+"texture_library", QDir::Files, QDirIterator::Subdirectories);
    while (iter.hasNext()) {
        QString fpath = iter.next();
        QFileInfo fi(iter.fileInfo());
        s_texture_paths[fi.baseName().toLower()] = fpath;
        loadTexHeader(fpath);
    }
}
