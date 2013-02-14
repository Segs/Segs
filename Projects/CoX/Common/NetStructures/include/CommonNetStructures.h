/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once

#include <ace/Log_Msg.h>
#include <string>
#include <sstream>
#include "BitStream.h"
class Quaternion
{
public:
        Quaternion &FromNormalized(float a,float b,float c)
        {
                q[0]=a;
                q[1]=b;
                q[2]=c;
                float res = (a*a+b*b+c*c)-1;
                q[3]= std::max<>((float)0,res);
                return *this;
        }
        Quaternion()
        {
                q[0]=q[1]=q[2]=q[3]=0.0;
        }
        union
        {
                struct
                {
                        float x,y,z,amount;
                } vals;
                float q[4];
        };
};
class Vector3
{
public:
        Vector3(float x=0.0f,float y=0.0f,float z=0.0f)
        {
                v[0]=x;
                v[1]=y;
                v[2]=z;
        }
        union {
                float v[3];
                struct {
                        float x,y,z;
                };
        };
};
typedef struct
{
        Vector3 row1;
        Vector3 row2;
        Vector3 row3;
        Vector3 row4;
} Matrix4x3;
class TransformStruct
{
public:
        TransformStruct(const Vector3 &a,const Vector3 &b,const Vector3 &c,bool set1,bool set2,bool set3) :
          v1(a),v2(b),v3(c),v1_set(set1),v2_set(set2),v3_set(set3)
        {
        }
        TransformStruct() {v1_set=v2_set=v3_set=false;}
        Vector3 v1;
        Vector3 v2;
        Vector3 v3;
        bool v1_set,v2_set,v3_set;
};

class NetStructure // this represents an interface all structures that are traversing the network should implement
{
static const	uint32_t    stringcachecount_bitlength=12;
static const	uint32_t    colorcachecount_bitlength=10;
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
    static  void            storeStringConditional(BitStream &bs,const std::string &str);
    static  void            storeVector(BitStream &bs, Vector3 &vec);
    static  void            storeVectorConditional(BitStream &bs, Vector3 &vec);
    static  void            storeTransformMatrix(BitStream &tgt,const Matrix4x3 &src);
    static  void            storeTransformMatrix(BitStream &tgt,const TransformStruct &src);

    static  void            getTransformMatrix(BitStream &bs,Matrix4x3 &src);
    static  void            storeCached_Color(BitStream &bs,uint32_t col);
    static  void            storeCached_String(BitStream &bs,const std::string & str);
    static  uint32_t        getCached_Color(BitStream &bs);
    static  std::string     getCached_String(BitStream &bs);
};
