/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Components/BitStream.h"
#include "Common/GameData/CoXHash.h"

#include <glm/vec3.hpp>
#include <glm/mat4x3.hpp>

class QString;

class TransformStruct
{
public:
        TransformStruct(const glm::vec3 &a,const glm::vec3 &b,const glm::vec3 &c,bool set1,bool set2,bool set3) :
          v1(a),v2(b),v3(c),v1_set(set1),v2_set(set2),v3_set(set3)
        {
        }
        TransformStruct() {v1_set=v2_set=v3_set=false;}
        glm::vec3 v1;
        glm::vec3 v2;
        glm::vec3 v3;
        bool v1_set,v2_set,v3_set;
        template<class Archive>
        void serialize(Archive &ar)
        {
            ar(v1,v2,v3);
            ar(v1_set,v2_set,v3_set);
        }
};

///
/// \brief The ColorAndPartPacker class is responsible for packing/unpacking colors and part names from BitStream
///
class ColorAndPartPacker
{
public:
    virtual void packColor(uint32_t c,BitStream &into) const=0;
    virtual void unpackColor(BitStream &from,uint32_t &tgt) const =0;
    virtual void packPartname(const QString &c,BitStream &into) const =0;
    virtual void unpackPartname(BitStream &from,QString &tgt) const =0;
};
///
/// \brief The IndexedStringPacker class is responsible for storing a mapping from string to index
/// So when sending a new string to the client, the contents can be sent only once, and afterwards the index is used instead.
///
class IndexedStringPacker
{
public:
    virtual void addString(const QString &) = 0;
    /// return index of a string, or 0 if the string has not been added yet.
    virtual int getIndex(const QString &) const =0;
};
extern  void        storeBitsConditional(BitStream &bs, uint8_t numbits, int bits);
extern  int         getBitsConditional(BitStream &bs, uint32_t numbits);
extern  void        storePackedBitsConditional(BitStream &bs, uint8_t numbits, int bits);
extern  void        storeFloatConditional(BitStream &bs,float val);
extern  void        storeFloatPacked(BitStream &bs,float val);
extern  int         getPackedBitsConditional(BitStream &bs, uint8_t numbits);
extern  void        storeStringConditional(BitStream &bs, const QString &str);
extern  void        storeVector(BitStream &bs, glm::vec3 &vec);
extern  void        storeVectorConditional(BitStream &bs, glm::vec3 &vec);
extern  void        storeTransformMatrix(BitStream &tgt,const glm::mat4x3 &src);
extern  void        storeTransformMatrix(BitStream &tgt,const TransformStruct &src);
extern  void        getTransformMatrix(BitStream &bs,glm::mat4x3 &src);
extern  void        storeCached_Color(BitStream &bs, uint32_t col, ColorHash &color_hash, uint32_t bitcount);
extern  void        storeCached_String(BitStream &bs, const QString &str,const StringHash &string_hash, uint32_t bitc);
extern  uint32_t    getCached_Color(BitStream &bs,ColorHash &color_hash, uint32_t bitcount);
extern  QString     getCached_String(BitStream &bs, const StringHash &string_hash, uint32_t bitcount);
