/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once
#include <string>
#include <vector>
#include <ace/Singleton.h>
#include <ace/Thread_Mutex.h>
#include <fstream>
#include <map>
#include "types.h"
#include "CoXHash.h"
#include "DataStorage.h"
class BinReadable;
class CrcVisitor;
class ClassSchema;
struct Field
{
    std::string m_name;
    u32 m_type;
    u32 m_offset;
    u32 m_param;
    ClassSchema *m_sub_ref;
    union // this is how the Field knows how to access the member variable in target
    {
        u32                         BinReadable::*pu32;
        u16                         BinReadable::*pu16;
        std::string                 BinReadable::*pstr;
        float                       BinReadable::*pfloat;
        Vec3                        BinReadable::*pvec3;
        Vec2                        BinReadable::*pvec2;
        Color3ub                    BinReadable::*pub3;
        std::vector<u32>            BinReadable::*pvec32;
        std::vector<float>          BinReadable::*pvec_flt;
        std::vector<std::string>    BinReadable::*pvec_str;
        u8 *                        BinReadable::*pbytes;
        std::vector<BinReadable *>  BinReadable::*pvec_entries;
    };
    Field(const std::string &name,u32 type,u32 offset,u32 param,ClassSchema *sub) : m_name(name),m_type(type),m_offset(offset),m_param(param),m_sub_ref(sub)
    {}
    Field(const std::string &name,u32 type,u32 offset,u32 param,ClassSchema *sub,std::string BinReadable::*str) : m_name(name),
                m_type(type),
                m_offset(offset),
                m_param(param),
                m_sub_ref(sub),
                pstr(str)
    {}
    Field(const std::string &name,u32 type,u32 offset,u32 param,ClassSchema *sub,std::vector<BinReadable *>  BinReadable::*sub_entries) : m_name(name),
        m_type(type),
        m_offset(offset),
        m_param(param),
        m_sub_ref(sub),
        pvec_entries(sub_entries)
    {}
    Field(const std::string &name,u32 type,u32 offset,u32 param,ClassSchema *sub,std::vector<std::string>  BinReadable::*sub_entries) : m_name(name),
        m_type(type),
        m_offset(offset),
        m_param(param),
        m_sub_ref(sub),
        pvec_str(sub_entries)
    {}
    Field(const std::string &name,u32 type,u32 offset,u32 param,ClassSchema *sub,u32  BinReadable::*val) : m_name(name),
        m_type(type),
        m_offset(offset),
        m_param(param),
        m_sub_ref(sub),
        pu32(val)
    {}
    Field(const std::string &name,u32 type,u32 offset,u32 param,ClassSchema *sub,Vec3  BinReadable::*vec) : m_name(name),
        m_type(type),
        m_offset(offset),
        m_param(param),
        m_sub_ref(sub),
        pvec3(vec)
    {}

    Field(const std::string &name,u32 type,u32 offset,u32 param,ClassSchema *sub,float  BinReadable::*flt) : m_name(name),
        m_type(type),
        m_offset(offset),
        m_param(param),
        m_sub_ref(sub),
        pfloat(flt)
    {}

    void calc_crc(CrcVisitor &v) const;
    bool read_non_nested(BinReadable &tgt,Store *s) const;
    bool read_nested(BinReadable &tgt,Store *s) const;
};

class ClassSchema
{
    std::vector<Field> m_fields;
    std::map<std::string,size_t> m_map_names; // this is map from name to m_fields index +1, 0 is special idx meaning:  no such field
    BinReadable * (*m_constructor)(void);
public:
    ClassSchema(BinReadable * (*constructor)(void)) : m_constructor(constructor)
    {

    }
    BinReadable * create_instance() {return m_constructor();}
    Field &operator[](size_t idx) {return m_fields[idx];}
    void add_field(const std::string &name,u32 type,u32 offset,u32 param,ClassSchema *sub);
    void add_field(const Field &fld);
    bool read(BinReadable &tgt,Store *s) const;
    void calc_crc(CrcVisitor &v) const;
};

class BinReadable
{
public:
    virtual const ClassSchema *my_schema()=0;
    bool read(const std::string &name);
    bool read(Store *s);
};
#define DECL_READABLE(classname) \
    static ClassSchema m_schema;\
    static bool schema_initialized;\
    \
    static BinReadable * create() {return new classname;}\
    const ClassSchema *my_schema() {return &m_schema;}

#define DEF_SCHEMA(classname) \
    ClassSchema classname::m_schema(classname::create);\
    bool classname::schema_initialized=false;
#define STR_REF(classname,variable)  ((std::string BinReadable::*)&classname::variable)
#define ARR_REF(classname,variable)  ((std::vector<BinReadable *>  BinReadable::*)&classname::variable)
#define VEC3_REF(classname,variable) ((Vec3 BinReadable::*)&classname::variable)
#define U32_REF(classname,variable) ((u32 BinReadable::*)&classname::variable)
#define FLT_REF(classname,variable) ((float BinReadable::*)&classname::variable)
#define ADD_FIELD(a,b,c,d,e,f) \
{\
    Field fld(a,b,c,d,e,f);\
    m_schema.add_field(fld);\
}
#define ADD_BASIC_FIELD(a,b,c,d,e) \
{\
    Field fld(a,b,c,d,e);\
    m_schema.add_field(fld);\
}
#define ADD_END(a) \
{\
    Field fld(a,2,0,0,0);\
    m_schema.add_field(fld);\
}

struct ColorEntry : public BinReadable
{
    DECL_READABLE(ColorEntry);

    static void build_schema();

    Vec3 rgb;
};
struct ColorStorage : public BinReadable
{
    DECL_READABLE(ColorStorage);

    static void build_schema();

    std::vector<BinReadable *> m_colors;

    static u32 color_to_4ub(const ColorEntry *e)
    {
        const Vec3 &rgb(e->rgb);
        return ((u32)rgb.v[0]) | (((u32)rgb.v[1])<<8) | (((u32)rgb.v[2])<<16) | (0xFF<<24);
    }
};
