/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x3.hpp>
#include "BitStream.h"
#include "Common/GameData/CoXHash.h"

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
};

///
/// \brief The ColorAndPartPacker class is responsible for packing/unpacking colors and part names from BitStream
///
class ColorAndPartPacker
{
public:
    virtual void packColor(uint32_t c,BitStream &into)=0;
    virtual void unpackColor(BitStream &from,uint32_t &tgt)=0;
    virtual void packPartname(const QString &c,BitStream &into)=0;
    virtual void unpackPartname(BitStream &from,QString &tgt)=0;
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
