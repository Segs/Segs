/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <cereal/archives/json.hpp>
#include <cereal/archives/memory_binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>
#include <cereal/cereal.hpp>

#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDebug>

 // A simple file access wrapper to allow re-locating/packing files
struct FSWrapper
{
    virtual ~FSWrapper() = default;

    virtual QIODevice* open(const QString &path, bool read_only = true, bool text_only = false)=0;
    virtual bool exists(const QString& path)=0;
    virtual QStringList dir_entries(const QString& path) = 0;
};

struct QFSWrapper : public FSWrapper
{
    ~QFSWrapper() override = default;

    QIODevice *open(const QString &path, bool read_only = true, bool text_only = false) override;

    bool exists(const QString &path) override;

    QStringList dir_entries(const QString &path) override;
};

template<class T>
void commonSaveTo(const T & target, const char *classname, const QString & baseName, bool text_format)
{
    QString target_fname;
    if(text_format)
        target_fname = baseName + ".crl.json";
    else
        target_fname = baseName + ".crl.bin";
    QFile  tgt_fle(target_fname);
    try
    {
        if(text_format) {
            std::ostringstream tgt;
            {
                cereal::JSONOutputArchive ar( tgt );
                ar(cereal::make_nvp(classname,target));
            }
            if(!tgt_fle.open(QFile::WriteOnly|QFile::Text)) {
                qCritical() << "Failed to open"<<target_fname<<"in write mode";
                return;
            }
            tgt_fle.write(tgt.str().c_str(),tgt.str().size());
        }
        else {
            std::vector<uint8_t> tgt;
            cereal::VectorOutputArchive ar( tgt );
            ar(cereal::make_nvp(classname,target));
            if(!tgt_fle.open(QFile::WriteOnly)) {
                qCritical() << "Failed to open"<<target_fname<<"in write mode";
                return;
            }
            tgt_fle.write((const char *)tgt.data(),tgt.size());
        }
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

template<class T>
bool commonReadFrom(FSWrapper &fs,const QString &crl_path,const char *classname, T &target)
{
    QIODevice *ifl=nullptr;
    if(crl_path.endsWith("json") || crl_path.endsWith("crl_json"))
    {
        ifl = fs.open(crl_path,true,true);
        if(!ifl)
        {
            qWarning() << "Failed to open" << crl_path;
            return false;
        }

        std::istringstream istr(ifl->readAll().toStdString());
        delete ifl;

        try
        {
            cereal::JSONInputArchive arc(istr);
            arc(cereal::make_nvp(classname,target));
        }
        catch(cereal::RapidJSONException &e)
        {
            qWarning() << e.what();
        }
        catch (std::exception &e)
        {
            qCritical() << e.what();
        }
    }
    else if(crl_path.endsWith(".crl.bin"))
    {
        ifl = fs.open(qPrintable(crl_path), true, false);
        if(!ifl)
        {
            qWarning() << "Failed to open" << crl_path;
            return false;
        }

        std::vector<uint8_t> istr;
        istr.resize(ifl->size());
        ifl->read((char *)istr.data(),ifl->size());
        delete ifl;
        try
        {
            cereal::VectorInputArchive arc(istr);
            arc(cereal::make_nvp(classname,target));
        }
        catch(cereal::RapidJSONException &e)
        {
            qWarning() << e.what();
        }
        catch (std::exception &e)
        {
            qCritical() << e.what();
        }
    }
    else {
        qWarning() << "Invalid serialized data extension in" <<crl_path;
    }
    return true;
}

template<class T>
void serializeToQString(const T &data, QString &tgt)
{
    std::ostringstream ostr;
    {
        cereal::JSONOutputArchive ar(ostr);
        ar(data);
    }
    tgt = QString::fromStdString(ostr.str());
}

template<class T>
void serializeFromQString(T &data,const QString &src)
{
    if(src.isEmpty())
        return;
    std::istringstream istr;
    istr.str(src.toStdString());
    {
        cereal::JSONInputArchive ar(istr);
        ar(data);
    }
}
#define SPECIALIZE_SERIALIZATIONS(type)\
template \
void type::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive);\
template \
void type::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive);\
template \
void type::serialize<cereal::VectorInputArchive>(cereal::VectorInputArchive & archive);\
template \
void type::serialize<cereal::VectorOutputArchive>(cereal::VectorOutputArchive & archive);
#define SPECIALIZE_VERSIONED_SERIALIZATIONS(type)\
template \
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, type & m, uint32_t const version);\
template \
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, type & m, uint32_t const version);\
template \
void serialize<cereal::VectorInputArchive>(cereal::VectorInputArchive & archive, type & m, uint32_t const version);\
template \
void serialize<cereal::VectorOutputArchive>(cereal::VectorOutputArchive & archive, type & m, uint32_t const version);
#define SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(type)\
template \
void type::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, uint32_t const version);\
template \
void type::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, uint32_t const version);\
template \
void type::serialize<cereal::VectorInputArchive>(cereal::VectorInputArchive & archive, uint32_t const version);\
template \
void type::serialize<cereal::VectorOutputArchive>(cereal::VectorOutputArchive & archive, uint32_t const version);

#define SPECIALIZE_SPLIT_SERIALIZATIONS(type)\
template \
void CEREAL_SAVE_FUNCTION_NAME<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, const type & m);\
template \
void CEREAL_SAVE_FUNCTION_NAME<cereal::VectorOutputArchive>(cereal::VectorOutputArchive & archive, const type & m);\
template \
void CEREAL_LOAD_FUNCTION_NAME<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, type & m);\
template \
void CEREAL_LOAD_FUNCTION_NAME<cereal::VectorInputArchive>(cereal::VectorInputArchive & archive, type & m);

