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
    uint32_t m_type;
    uint32_t m_offset;
    uint32_t m_param;
    ClassSchema *m_sub_ref;
    Field(const std::string &name,uint32_t type,uint32_t offset,uint32_t param,ClassSchema *sub) : m_name(name),m_type(type),m_offset(offset),m_param(param),m_sub_ref(sub)
    {}
    void calc_crc(CrcVisitor &v) const;
    virtual bool read_non_nested(BinReadable &tgt,Store *s) const;
    virtual bool read_nested(BinReadable &tgt,Store *s) const;
protected:
    virtual bool do_read(uint8_t type,BinReadable &tgt,Store *s) const=0;
    virtual bool do_read_nested(BinReadable &tgt,Store *s) const=0;
};
//! MiscField's are used to mark end of structures
struct MiscField : public Field
{
    MiscField(const std::string &name,uint32_t type,uint32_t offset,uint32_t param,ClassSchema *sub) : Field(name,type,offset,param,sub)
    {}
protected:
    bool do_read(uint8_t ,BinReadable &,Store *) const {assert("Misc field read attempted"); return false;}
    bool do_read_nested(BinReadable &,Store *) const {assert("Nested read attempt on non nested value!"); return false;}

};
struct NonNestableField : public Field
{
    NonNestableField(const std::string &name,uint32_t type,uint32_t offset,uint32_t param,ClassSchema *sub) :
        Field(name,type,offset,param,sub)
    {}
protected:
    virtual bool do_read_nested(BinReadable &,Store *) const {assert("Nested read attempt on non nested value!");return false;}
    virtual bool do_read(uint8_t type,BinReadable &tgt,Store *s) const=0;
};
template<class T>
struct TemplateField : public NonNestableField
{
    T                 BinReadable::*pval;
    TemplateField(const std::string &name,uint32_t type,uint32_t offset,uint32_t param,ClassSchema *sub,T BinReadable::*val) :
    NonNestableField(name,type,offset,param,sub),
        pval(val)
    {}
protected:
    bool do_read(uint8_t type,BinReadable &tgt,Store *s) const;
    bool do_read_nested(BinReadable &,Store *) const {assert("Nested read attempt on non nested value!"); return false;}
};
template<class T>
struct NestedTemplateField : public Field
{
    std::vector<T *> BinReadable::*pval;
    NestedTemplateField(const std::string &name,uint32_t type,uint32_t offset,uint32_t param,ClassSchema *sub,std::vector<T *> BinReadable::*val) :
        Field(name,type,offset,param,sub),
        pval(val)
    {}
protected:
    bool do_read(uint8_t ,BinReadable &,Store *) const {assert("Plain field read attempted on nested field."); return false;}
    bool do_read_nested(BinReadable &tgt,Store *s) const;

};
class ClassSchema
{
    std::vector<Field *> m_fields;
    std::map<std::string,size_t> m_map_names; // this is map from name to m_fields index +1, 0 is special idx meaning:  no such field
    BinReadable * (*m_constructor)(void);
    void internal_add_field(Field *);
public:
    ClassSchema(BinReadable * (*constructor)(void)) : m_constructor(constructor)
    {

    }
    BinReadable * create_instance() {return m_constructor();}
    Field &operator[](size_t idx) {return *m_fields[idx];}
    template<class T>
    void add_field_nested(const std::string &name,uint32_t type,uint32_t offset,uint32_t param,ClassSchema *sub,std::vector<T *> BinReadable::*val)
    {
        internal_add_field(new NestedTemplateField<T>(name,type,offset,param,sub,val));
    }
    template<class T>
    void add_field(const std::string &name,uint32_t type,uint32_t offset,uint32_t param,T BinReadable::*val)
    {
        internal_add_field(new TemplateField<T>(name,type,offset,param,0,val));
    }
    void add_end(const std::string &val="\n")
    {
        internal_add_field(new MiscField(val,2,0,0,0));
    }
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
#define ARR_OF_REF(classname,storedclass,variable)  ((std::vector<storedclass *>  BinReadable::*)&classname::variable)
#define TARGETED_ARR_OF_REF(classname,storedclass,variable)  (&storedclass::m_schema),((std::vector<storedclass *>  BinReadable::*)&classname::variable)
#define VEC3_REF(classname,variable) ((Vec3 BinReadable::*)&classname::variable)
#define U32_REF(classname,variable) ((uint32_t BinReadable::*)&classname::variable)
#define FLT_REF(classname,variable) ((float BinReadable::*)&classname::variable)
#define ADD_STR_FIELD(a,b,c,d,e,f) m_schema.add_field(new StringField(a,b,c,d,e,f));

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

    static uint32_t color_to_4ub(const ColorEntry *e)
    {
        const Vec3 &rgb(e->rgb);
        return ((uint32_t)rgb.v[0]) | (((uint32_t)rgb.v[1])<<8) | (((uint32_t)rgb.v[2])<<16) | (0xFF<<24);
    }
};


template<class T>
bool NestedTemplateField<T>::do_read_nested(BinReadable &tgt,Store *s) const
{
    assert(m_type==0x15);
    bool res;
    T *t = static_cast<T *>(m_sub_ref->create_instance());//cast constructed to stored type
    s->nest_in(this);
    res=m_sub_ref->read(*t,s); // read in created value
    (tgt.*pval).push_back(t);
    s->nest_out(this);
    return res;
}
