/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Client.cpp 253 2006-08-31 22:00:14Z malign $
 */
#include <ace/OS.h>
#include "ReadableStructures.h"

DEF_SCHEMA(ColorEntry);
DEF_SCHEMA(ColorStorage);

void Field::calc_crc( CrcVisitor &v ) const
{
    v.digest(m_name.c_str(),m_name.size());
    v.digest((char *)&m_type,12); // also crc of type,offset,param
    if(m_sub_ref)
        m_sub_ref->calc_crc(v);
}

bool Field::read_non_nested( BinReadable &tgt,Store *s ) const
{
    bool parse_ok=true;
    if(m_type&0x100)
        return parse_ok;
    switch ( (m_type&0xFF) - 3 )
    {
    case 0:
    case 3:
    case 4:
        {
            std::string ps = s->read_str(12000);
            //parse_ok &= (ps.size()!=0); TODO handle string read errors
            (tgt.*pstr)=ps;
        }
        break;
    case 1:
    case 2:
    case 12:
        {
            u32 v;
            parse_ok &= s->read(v);
            (tgt.*pu32)=v;
        }
        break;
    case 5:
        {
            std::string ps = s->read_str(128);
            //parse_ok &= (ps.size()!=0); TODO handle string read errors
            (tgt.*pstr)=ps;
        }
        break;
    case 6:
        {
            u32 v;
            parse_ok &= s->read(v);
            (tgt.*pu32)=v;
        }
        break;
    case 7:
        {
            u32 v;
            parse_ok &= s->read(v);
            (tgt.*pu32)=v;
        }
        break;
    case 8:
        {
            Vec2 val;
            parse_ok &= s->read(val.v[0]);
            parse_ok &= s->read(val.v[1]);
            (tgt.*pvec2) = val;
        }
        break;
    case 9:
        {
            Vec3 val;
            parse_ok &= s->read(val.v[0]);
            parse_ok &= s->read(val.v[1]);
            parse_ok &= s->read(val.v[2]);
            (tgt.*pvec3) = val;
        }
        break;
    case 10:
        {
            Color3ub rgb;
            parse_ok &= s->read(rgb.v[0]);
            parse_ok &= s->read(rgb.v[1]);
            parse_ok &= s->read(rgb.v[2]);
            (tgt.*pub3) = rgb;
            s->fixup();
        }
        break;
    case 11:
        {
            u16 v;
            parse_ok &= s->read(v);
            (tgt.*pu16) = v;
            s->fixup();
        }
        break;
    case 13:
        {
            u32 to_read = 0;
            parse_ok &= s->read(to_read);
            (tgt.*pvec32).clear();
            if ( 0==to_read)
                break;
            for(size_t idx = 0; idx < to_read; ++idx)
            {
                (tgt.*pvec32).push_back(0);
                parse_ok &= s->read((tgt.*pvec32)[idx]);
            }
        }
        break;
    case 14:
        {
            u32 to_read = 0;
            parse_ok &= s->read(to_read);
            (tgt.*pvec_flt).clear();
            if ( 0==to_read)
                break;
            for(size_t idx = 0; idx < to_read; ++idx)
            {
                (tgt.*pvec_flt).push_back(0.0f);
                parse_ok &= s->read((tgt.*pvec_flt)[idx]);
            }
        }
        break;
    case 15: //array of strings
        {
            u32 to_read = 0;
            parse_ok &= s->read(to_read);
            (tgt.*pvec_str).clear();
            if ( 0==to_read)
                break;
            for(size_t idx = 0; idx < to_read; ++idx)
            {
                (tgt.*pvec_str).push_back(s->read_str(12000));
                //parse_ok &= (tgt.*pvec_str)[idx].size()>0; TODO handle string read errors
            }
        }
        break;
    case 16:
        break;
    case 17:
        {
            u8 *v = (tgt.*pbytes);
            if(v)
                delete v;
            (tgt.*pbytes) = new u8 [m_param];
            parse_ok &= s->read_bytes((char *)(tgt.*pbytes),m_param);
            s->fixup();
        }
    }
    return parse_ok;
}

bool Field::read_nested( BinReadable &tgt,Store *s ) const
{
    switch(m_type)
    {
    case 0x15: // array of entries
        {
            bool res;
            BinReadable *t = m_sub_ref->create_instance();
            s->nest_in(this);
            res=m_sub_ref->read(*t,s);
            (tgt.*pvec_entries).push_back(t);
            s->nest_out(this);
            return res;
        }
    case 0x17:
        {
            ACE_ASSERT(!"Not implemented nested read");
        }
        break;
    case 0x13:
        {
            ACE_ASSERT(!"Not implemented nested read");
        }
        break;
    }
    return false;
}

void ColorEntry::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;
    Field fld("",0x20C,0x0,0x0,0);
    fld.pvec3 = (Vec3 BinReadable::*)&ColorEntry::rgb;
    m_schema.add_field(fld);
    ADD_END("\n");
}

void ColorStorage::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    ColorEntry::build_schema();

    ADD_FIELD("Color"        ,0x15,0x0,0xC,&ColorEntry::m_schema,(std::vector<BinReadable *>  BinReadable::*)&ColorStorage::m_colors);
    ADD_END("EndPalette");
}


void ClassSchema::add_field( const std::string &name,u32 type,u32 offset,u32 param,ClassSchema *sub )
{
    m_fields.push_back(Field(name,type,offset,param,sub));
    m_map_names[name] = 1+(m_fields.size()-1);
}

void ClassSchema::add_field( const Field &fld )
{
    m_fields.push_back(fld);
    m_map_names[fld.m_name] = 1+(m_fields.size()-1);
}

bool ClassSchema::read( BinReadable &tgt,Store *s ) const
{
    bool result=true;
    size_t field_idx;
    s->prepare();
    for(field_idx=0; field_idx<m_fields.size(); ++field_idx)
    {
        result &= m_fields[field_idx].read_non_nested(tgt,s);
    }
    result &= s->prepare_nested(); // will update the file size left
    if(s->end_encountered())
        return result;
    std::string name;
    while(s->nesting_name(name))
    {
        std::pair<std::string,size_t> res=*m_map_names.find(name);
        field_idx=res.second;
        // find Field by name
        if(field_idx==0)
            ACE_ASSERT(!"unknown field referenced.");
        // create instance of field value here ?
        result &= m_fields[field_idx-1].read_nested(tgt,s);
    }
    return result;
}

void ClassSchema::calc_crc( CrcVisitor &v ) const
{
    for(size_t idx=0; idx<m_fields.size(); ++idx)
        m_fields[idx].calc_crc(v);
}

bool BinReadable::read( const std::string &name )
{
    Store * r=StoreFactory::store_for_file(my_schema(),name);
    if(r)
    {
        bool res=read(r);
        StoreFactory::release(r);         
        return res;
    }
    return false;
}

bool BinReadable::read( Store *s )
{
    return my_schema()->read(*this,s);
}
