/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Colors.h"
#include "Components/BitStream.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/memory_binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/cereal.hpp>
#include <ace/Time_Value.h>
#include <ace/INET_Addr.h>

#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>

// helper wrapper for serializing default-value fields
template <typename T>
struct wrap_optional {
    T &tgt;
    constexpr wrap_optional(T &s) : tgt(s) {}
    // returns true if the referenced value is non-default
    operator bool() const { return tgt!=T();}
};

namespace cereal {
inline void epilogue(VectorOutputArchive &, QString const &) { }
inline void epilogue(BinaryInputArchive &, QString const &) { }
inline void epilogue(JSONOutputArchive &, QString const &) { }
inline void epilogue(JSONInputArchive &, QString const &) { }

inline void prologue(JSONOutputArchive &, QString const &) { }
inline void prologue(JSONInputArchive &, QString const &) { }
inline void prologue(VectorOutputArchive &, QString const &) { }
inline void prologue(BinaryInputArchive &, QString const &) { }

inline void epilogue(VectorOutputArchive &, QByteArray const &) { }
inline void epilogue(BinaryInputArchive &, QByteArray const &) { }
inline void epilogue(JSONOutputArchive &, QByteArray const &) { }
inline void epilogue(JSONInputArchive &, QByteArray const &) { }

inline void prologue(JSONOutputArchive &, QByteArray const &) { }
inline void prologue(JSONInputArchive &, QByteArray const &) { }
inline void prologue(VectorOutputArchive &, QByteArray const &) { }
inline void prologue(BinaryInputArchive &, QByteArray const &) { }

template <typename T>
inline void prologue(JSONOutputArchive &, wrap_optional<T> const &) { }
template <typename T>
inline void prologue(JSONInputArchive &, wrap_optional<T> const &) { }
template <typename T>
inline void prologue(VectorOutputArchive &, wrap_optional<T> const &) { }
template <typename T>
inline void prologue(BinaryInputArchive &, wrap_optional<T> const &) { }

template <typename T>
inline void epilogue(VectorOutputArchive &, wrap_optional<T> const &) { }
template <typename T>
inline void epilogue(BinaryInputArchive &, wrap_optional<T> const &) { }
template <typename T>
inline void epilogue(JSONOutputArchive &, wrap_optional<T> const &) { }
template <typename T>
inline void epilogue(JSONInputArchive &, wrap_optional<T> const &) { }
template<class Archive>
inline void CEREAL_SAVE_FUNCTION_NAME(Archive & ar, ::QString const & str)
{
    ar( str.toStdString() );
}

template<class Archive>
inline void CEREAL_SAVE_FUNCTION_NAME(Archive & ar, ::QByteArray const & str)
{
    // make sure we actually have a latin1 string in str, and not something that has strange ascii chars
    assert(QString(str).toLatin1()==str);
    ar( str.toStdString() );
}

//! Serialization for utf8-like types, if binary data is supported
template<class Archive>
inline void CEREAL_LOAD_FUNCTION_NAME(Archive & ar, ::QString & str)
{
    std::string rd;
    ar( rd );
    str = QString::fromStdString(rd);
}
//! Serialization for latin1 string types, if binary data is supported
template<class Archive>
inline void CEREAL_LOAD_FUNCTION_NAME(Archive & ar, ::QByteArray & str)
{
    std::string rd;
    ar( rd );
    str = QByteArray::fromStdString(rd);
}
//! Serialization for std::map<uint32_t,std::vector<bool> >
template<class Archive, class C, class A,
    traits::EnableIf<traits::is_text_archive<Archive>::value> = traits::sfinae>
inline void CEREAL_SAVE_FUNCTION_NAME(Archive & ar, std::map<uint32_t, std::vector<bool>, C, A> map) // trying without &
{
    for (const auto & pair : map)
    {
        ar(cereal::make_nvp(QVariant(pair.first).toString().toStdString(), pair.second));
    }
}
//! Serialization for std::map<uint32_t,std::vector<bool> >
template<class Archive, class C, class A,
    traits::EnableIf<traits::is_text_archive<Archive>::value> = traits::sfinae>
inline void CEREAL_LOAD_FUNCTION_NAME(Archive & ar, std::map<uint32_t, std::vector<bool>, C, A> & map)
{
    map.clear();
    auto hint = map.begin();

    while (true)
    {
        const auto node_name_pointer = ar.getNodeName();

        if (!node_name_pointer)
        {
            break;
        }
        
        uint32_t key = QVariant(node_name_pointer).toUInt();
        std::vector<bool> values;
        ar(values);
        hint = map.emplace_hint(hint, std::move(key), std::move(values));
    }
}
template<class Archive>
void serialize(Archive & archive, glm::vec3 & m)
{
    size_type size=3;
    archive( make_size_tag( size ) ); // this is new
    for( int i=0; i<3; ++i )
      archive( m[i] );
}
template<class Archive>
void serialize(Archive & archive, glm::quat & m)
{
    size_type size=4;
    archive( make_size_tag( size ) ); // this is new
    for( int i=0; i<4; ++i )
      archive( m[i] );
}

template<class Archive>
void serialize(Archive & archive, glm::mat4x3 & m)
{
    size_type size=12;
    archive( make_size_tag( size ) ); // this is new
    for( int i=0; i<12; ++i )
      archive( m[i] );
}

template<class Archive>
void serialize(Archive & archive, glm::vec2 & m)
{
    size_type size=2;
    archive( make_size_tag( size ) ); // this is new
    for( int i=0; i<2; ++i )
      archive( m[i] );
}
//! Saving for wrap_optional
template <class Archive, typename T> inline
void CEREAL_SAVE_FUNCTION_NAME(Archive& ar, const wrap_optional<T>& optional)
{
    if(optional) {
        ar(optional.tgt);
    }
}

//! Loading for wrap_optional
template <class Archive, typename T> inline
void CEREAL_LOAD_FUNCTION_NAME(Archive& ar, wrap_optional<T>& optional)
{
    try
    {
        T value;
        ar(value);
        optional.tgt = std::move(value);
    }
    catch(cereal::Exception&)
    {
        ar.setNextName(nullptr);
        optional.tgt = T();
        // Loading a key that doesn't exist results in an exception
        // Since "Not Found" is a valid condition for us, we swallow
        // this exception and the archive will not load anything
    }
}
} // namespace cereal

template <class Archive>
void CEREAL_SAVE_FUNCTION_NAME( Archive & ar, const BitStream &buf );
template <class Archive>
void CEREAL_LOAD_FUNCTION_NAME( Archive & ar, BitStream &buf );

template<class Archive>
inline void CEREAL_LOAD_FUNCTION_NAME(Archive & ar, ACE_INET_Addr & addr)
{
    uint16_t port;
    uint32_t ipv4;
    ar( port );
    ar( ipv4 );
    addr = ACE_INET_Addr(port,ipv4);
}
template<class Archive>
inline void CEREAL_SAVE_FUNCTION_NAME(Archive & ar, const ACE_INET_Addr & addr)
{
    ar( addr.get_port_number() );
    ar( addr.get_ip_address() );
}
template<class Archive>
void serialize(Archive & archive, RGBA & m)
{
    archive(cereal::make_nvp("rgba",m.val));
}
template<class Archive>
void CEREAL_LOAD_FUNCTION_NAME(Archive & archive, QDateTime & m)
{
    qint64 msec_since_epoch;
    archive(msec_since_epoch);
    m = QDateTime::fromMSecsSinceEpoch(msec_since_epoch);
}
template<class Archive>
void CEREAL_SAVE_FUNCTION_NAME(Archive & archive, const QDateTime & m)
{
    qint64 msec_since_epoch = m.toMSecsSinceEpoch();
    archive(msec_since_epoch);
}

template<class Archive>
inline void CEREAL_LOAD_FUNCTION_NAME(Archive & ar, ACE_Time_Value & str)
{
    uint64_t rd;
    ar( rd );
    str = ACE_Time_Value(rd);
}
template<class Archive>
inline void CEREAL_SAVE_FUNCTION_NAME(Archive & ar, const ACE_Time_Value & str)
{
    ar( uint64_t(str.usec()) );
}
template<class Archive,class Type>
static void serialize_as_optional(Archive & archive, const char *name, Type & m)
{
    wrap_optional<Type> value(m);
    archive(cereal::make_nvp(name,value));
}
