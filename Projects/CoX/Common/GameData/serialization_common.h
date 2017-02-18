#pragma once
#include "Colors.h"
#include <glm/vec3.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>
#include <cereal/cereal.hpp>

#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QFile>

template<class T>
void commonSaveTo(const T & target, const char *classname, const QString & baseName, bool text_format)
{
    QString target_fname;
    if(text_format)
        target_fname = baseName + ".crl.json";
    else
        target_fname = baseName + ".crl.bin";
    std::ostringstream tgt;
    QFile  tgt_fle(target_fname);
    if(text_format) {
        cereal::JSONOutputArchive ar( tgt );
        ar(cereal::make_nvp(classname,target));
        if(!tgt_fle.open(QFile::WriteOnly|QFile::Text)) {
            qCritical() << "Failed to open"<<target_fname<<"in write mode";
            return;
        }
    }
    else {
        cereal::BinaryOutputArchive ar( tgt );
        ar(cereal::make_nvp(classname,target));
        if(!tgt_fle.open(QFile::WriteOnly)) {
            qCritical() << "Failed to open"<<target_fname<<"in write mode";
            return;
        }
    }
    tgt_fle.write(tgt.str().c_str(),tgt.str().size());
}

namespace cereal {
inline void epilogue(BinaryOutputArchive &, QString const &) { }
inline void epilogue(BinaryInputArchive &, QString const &) { }
inline void epilogue(JSONOutputArchive &, QString const &) { }
inline void epilogue(JSONInputArchive &, QString const &) { }

inline void prologue(JSONOutputArchive &, QString const &) { }
inline void prologue(JSONInputArchive &, QString const &) { }
inline void prologue(BinaryOutputArchive &, QString const &) { }
inline void prologue(BinaryInputArchive &, QString const &) { }

template<class Archive> inline void CEREAL_SAVE_FUNCTION_NAME(Archive & ar, ::QString const & str)
{
    ar( str.toStdString() );
}
//! Serialization for basic_string types, if binary data is supported
template<class Archive> inline void CEREAL_LOAD_FUNCTION_NAME(Archive & ar, ::QString & str)
{
    std::string rd;
    ar( rd );
    str = QString::fromStdString(rd);
}
template<class Archive>
void serialize(Archive & archive, glm::vec3 & m)
{
    int size=3;
    archive( make_size_tag( size ) ); // this is new
    for( int i=0; i<3; ++i )
      archive( m[i] );
}
template<class Archive>
void serialize(Archive & archive, RGBA & m)
{
    archive(cereal::make_nvp("rgba",m.val));
}

} // namespace cereal
