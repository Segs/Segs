/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup TextureConverter Projects/CoX/Utilities/TextureConverter
 * @{
 */

#include "cereal/cereal.hpp"
#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QDir>
#include <QFileInfo>
#include <cstring>


#pragma pack(push, 1)
struct TexFileHdr
{
    int header_size;
    int file_size;
    int wdth;
    int hght;
    int flags;
    int fade[2];
    uint8_t alpha;
    char magic[3];
};

#pragma pack(pop)
struct CohTextureInfo {
    int file_size;
    int width;
    int height;
    int flags;
    std::vector<int> fade;
    uint8_t alpha;
};

template<class Archive>
static void serialize(Archive & archive, CohTextureInfo & m)
{
    archive(cereal::make_nvp("file_size",m.file_size));
    archive(cereal::make_nvp("width",m.width));
    archive(cereal::make_nvp("height",m.height));
    archive(cereal::make_nvp("flags",m.flags));
    archive(cereal::make_nvp("fade",m.fade));
    archive(cereal::make_nvp("alpha",m.alpha));
}

QString saveTo(const CohTextureInfo &tgt)
{
    std::ostringstream str_dat;
    try
    {
        cereal::JSONOutputArchive input_arc(str_dat);
        input_arc(cereal::make_nvp("CohTextureInfo",tgt));
    }
    catch(std::runtime_error &e) {
        qDebug() << "Exception throw while reading scenedef";
        return "";
    }
    return QString::fromStdString(str_dat.str());
}

int main(int c,char **argv)
{
    QCoreApplication app(c,argv);
    if(app.arguments().size()<2) {
        qDebug() << app.arguments()[0] << " texturename";
        return 0;
    }
    QFile src_tex(app.arguments()[1]);
    if(src_tex.exists() && src_tex.open(QFile::ReadOnly)) {
        TexFileHdr hdr;
        src_tex.read((char *)&hdr,sizeof(TexFileHdr));
        if(0==memcmp(hdr.magic,"TX2",3))
        {
            qDebug() << "hdr.file_size" << hdr.file_size;
            qDebug() << "image size " << hdr.wdth << 'x'<<hdr.hght;
            qDebug() << "flags" << QString::number(hdr.flags,16);
            qDebug() << "fade" << hdr.fade[0]<< hdr.fade[1];
            qDebug() << "alpha" << QString::number(hdr.alpha,16);
            QString originalname=QString(src_tex.read(hdr.header_size-sizeof(TexFileHdr)));
            qDebug() << "Original name was:" << originalname;
            QFileInfo fi(originalname);
            QDir here;
            here.mkpath(fi.path());
            QFile unpacked(originalname);
            unpacked.open(QFile::WriteOnly);
            unpacked.write(src_tex.readAll());
            unpacked.close();
            CohTextureInfo info {
                hdr.file_size,
                hdr.wdth,
                hdr.hght,
                hdr.flags,
                {hdr.fade[0],hdr.fade[1]},
                hdr.alpha
            };
            qDebug() << saveTo(info);
        }
    }
    return 0;
}

//! @}
