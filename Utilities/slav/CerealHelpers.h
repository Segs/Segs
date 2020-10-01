/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#ifndef CEREALHELPERS_H
#define CEREALHELPERS_H
#include <QString>
#include <cereal/archives/json.hpp>

namespace cereal {
class VectorInputArchive;
class VectorOutputArchive;

inline void epilogue(VectorOutputArchive &, QString const &) { }
inline void epilogue(VectorInputArchive &, QString const &) { }
inline void epilogue(JSONOutputArchive &, QString const &) { }
inline void epilogue(JSONInputArchive &, QString const &) { }

inline void prologue(JSONOutputArchive &, QString const &) { }
inline void prologue(JSONInputArchive &, QString const &) { }
inline void prologue(VectorOutputArchive &, QString const &) { }
inline void prologue(VectorInputArchive &, QString const &) { }

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
}
#endif // CEREALHELPERS_H
