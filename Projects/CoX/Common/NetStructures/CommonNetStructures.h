/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <glm/vec3.hpp>
#include "BitStream.h"
class QString;
typedef struct
{
        glm::vec3 row1;
        glm::vec3 row2;
        glm::vec3 row3;
        glm::vec3 row4;
} Matrix4x3;
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

class NetStructure // this represents an interface all structures that are traversing the network should implement
{
static const    uint32_t    stringcachecount_bitlength=12;
static const    uint32_t    colorcachecount_bitlength=10;
public:
                            NetStructure()
                            {
                            }
    virtual                 ~NetStructure(){}
    virtual void            serializeto(BitStream &bs) const =0;
    virtual void            serializefrom(BitStream &bs)=0;
            size_t          bits_used; // this reflects how many bits given operation affected
    static  void            storeBitsConditional(BitStream &bs,int numbits,int bits);
    static  int             getBitsConditional(BitStream &bs,int numbits);
    static  void            storePackedBitsConditional(BitStream &bs,int numbits,int bits);
    static  void            storeFloatConditional(BitStream &bs,float val);
    static  void            storeFloatPacked(BitStream &bs,float val);
    static  int             getPackedBitsConditional(BitStream &bs,int numbits);
    static  void            storeStringConditional(BitStream &bs, const QString &str);
    static  void            storeVector(BitStream &bs, glm::vec3 &vec);
    static  void            storeVectorConditional(BitStream &bs, glm::vec3 &vec);
    static  void            storeTransformMatrix(BitStream &tgt,const Matrix4x3 &src);
    static  void            storeTransformMatrix(BitStream &tgt,const TransformStruct &src);

    static  void            getTransformMatrix(BitStream &bs,Matrix4x3 &src);
    static  void            storeCached_Color(BitStream &bs,uint32_t col);
    static  void            storeCached_String(BitStream &bs, const QString &str);
    static  uint32_t        getCached_Color(BitStream &bs);
    static  QString         getCached_String(BitStream &bs);
};
