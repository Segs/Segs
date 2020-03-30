/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapViewer Projects/CoX/Utilities/MapViewer
 * @{
 */

#include "CohTextureConverter.h"
#include "CoHSceneConverter.h"

#include "Common/GameData/trick_definitions.h"
#include "Common/Runtime/RuntimeData.h"
#include "Common/Runtime/Texture.h"

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

std::unordered_map<uint32_t,Urho3D::SharedPtr<Urho3D::Texture>> g_converted_textures;

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
}

SEGS::HTexture tryLoadTexture(Urho3D::Context *ctx,const QString &fname)
{
    SEGS::RuntimeData &rd(getRuntimeData());
    ResourceCache *rcache=ctx->m_ResourceCache.get();
    QFileInfo tex_path(fname);
    QString lookupstring = tex_path.baseName().toLower();
    QString actualPath   = rd.m_texture_paths.value(lookupstring);
    static SEGS::HTexture missing_tex_handle;
    if(actualPath.isEmpty())
    {
        if(!missing_tex_handle)
        {
            missing_tex_handle = SEGS::TextureStorage::instance().create();
            g_converted_textures[missing_tex_handle.idx] = rcache->GetResource<Texture2D>("Textures/Missing.dds");

        }
        if(!s_missing_textures.contains(lookupstring))
        {
            qDebug() << "Missing texture" << fname;
            s_missing_textures.insert(lookupstring);
        }
        return missing_tex_handle;
    }
    SEGS::HTexture &res(rd.m_loaded_textures[lookupstring]);
    if(g_converted_textures.find(res.idx)!=g_converted_textures.end())
        return res; // we have an Urho3D texture already, nothing to do.

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
    if (tgt.exists())
    {
        // a pre-converted texture file exists, load it instead
        g_converted_textures[res.idx] = rcache->GetResource<Texture2D>(converted_path);
        return res;
    }
    QByteArray data = src_tex.readAll();
    // save extracted texture into a local directory
    converted_dir.mkpath(QFileInfo(originalname).path());
    if(!tgt.open(QFile::WriteOnly))
    {
        qCritical() << "Cannot write:"<<converted_path;
    }
    else
    {
        tgt.write(data);
        tgt.close();
    }
    auto entry = new Texture2D(ctx);
    g_converted_textures[res.idx] = entry;
    VectorBuffer vbuf(data.data(),data.size());
    if (entry->BeginLoad(vbuf))
        entry->EndLoad();
    return res;
}

std::vector<SEGS::HTexture> getModelTextures(Urho3D::Context *ctx,std::vector<QString> &names)
{
    uint32_t name_count = std::max<uint32_t>(1,names.size());
    std::vector<SEGS::HTexture> res;
    res.reserve(name_count);
    SEGS::HTexture white_tex = tryLoadTexture(ctx,"white.tga");

    for(size_t tex_idx=0; tex_idx < names.size(); ++tex_idx )
    {
        QFileInfo fi(names[tex_idx]);
        QString baseName = fi.completeBaseName();
        if(baseName!=names[tex_idx])
        {
            if(fi.fileName() == names[tex_idx])
                names[tex_idx] = baseName;
            else
                names[tex_idx] = fi.path()+"/"+baseName;
        }
        if ( names[tex_idx].contains("PORTAL",Qt::CaseInsensitive) )
            res.emplace_back(tryLoadTexture(ctx,"invisible.tga"));
        else
            res.emplace_back(tryLoadTexture(ctx,names[tex_idx]));
        // replace missing texture with white
        // TODO: make missing textures much more visible ( high contrast + text ? )
        if ( g_converted_textures.end()==g_converted_textures.find(res[tex_idx].idx) )
        {
            res[tex_idx] = white_tex;
        }
    }
    if (names.empty())
        res.emplace_back(white_tex);
    return res;
}

//! @}
