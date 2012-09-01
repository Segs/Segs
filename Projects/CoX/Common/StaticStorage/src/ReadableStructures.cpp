/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#include <ace/ACE.h>
#include "ReadableStructures.h"
using namespace std;
DEF_SCHEMA(ColorEntry);
DEF_SCHEMA(ColorStorage);

void Field::calc_crc( CrcVisitor &v ) const
{
    v.digest(m_name.c_str(),m_name.size());
    v.digest((char *)&m_type,12); // also crc of type,offset,param
    if(m_sub_ref)
        m_sub_ref->calc_crc(v);
}
template<>
bool TemplateField<std::string>::do_read(u8 type, BinReadable &tgt,Store *s ) const
{
    bool parse_ok=true;
    switch ( type )
    {
    case 0:
    case 3:
    case 4:
        {
            std::string ps = s->read_str(12000);
            //parse_ok &= (ps.size()!=0); TODO handle string read errors
            (tgt.*pval)=ps;
        }
        break;
    case 5:
        {
            std::string ps = s->read_str(128);
            //parse_ok &= (ps.size()!=0); TODO handle string read errors
            (tgt.*pval)=ps;
        }
        break;
    default:
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Unexpected field type:%d expected a string.\n"), type),false);
    }
    return parse_ok;
}

template <>
bool TemplateField<Vec3>::do_read(u8 type, BinReadable &tgt,Store *s ) const
{
    bool parse_ok=true;
    switch ( type )
    {
    case 9:
        {
            Vec3 val;
            parse_ok &= s->read(val.v[0]);
            parse_ok &= s->read(val.v[1]);
            parse_ok &= s->read(val.v[2]);
            (tgt.*pval) = val;
        }
        break;
    default:
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Unexpected field type:%d expected a Vec3.\n"), type),false);
        break;
    }
    return parse_ok;
}
template <>
bool TemplateField<u32>::do_read(u8 type, BinReadable &tgt,Store *s ) const
{
    bool parse_ok=true;
    switch ( type )
    {
    case 1:
    case 2:
    case 12:
        {
            u32 v;
            parse_ok &= s->read(v);
            (tgt.*pval)=v;
        }
        break;
    case 6:
        {
            u32 v;
            parse_ok &= s->read(v);
            (tgt.*pval)=v;
        }
        break;
    default:
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Unexpected field type:%d expected a u32.\n"), type),false);
        break;
    }
    return parse_ok;
}
template <>
bool TemplateField<float>::do_read(u8 type, BinReadable &tgt,Store *s ) const
{
    bool parse_ok=true;
    switch ( type )
    {
    case 7: // float
        {
            float v;
            parse_ok &= s->read(v);
            (tgt.*pval)=v;
        }
        break;
    default:
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Unexpected field type:%d expected a float.\n"), type),false);
        break;
    }
    return parse_ok;
}
template <>
bool TemplateField<Vec2>::do_read(u8 type, BinReadable &tgt,Store *s ) const
{
    bool parse_ok=true;
    switch ( type )
    {
    case 8:
        {
            Vec2 val;
            parse_ok &= s->read(val.v[0]);
            parse_ok &= s->read(val.v[1]);
            (tgt.*pval) = val;
        }
        break;
    default:
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Unexpected field type:%d expected a float.\n"), type),false);
        break;
    }
    return parse_ok;
}
template <>
bool TemplateField<Color3ub>::do_read(u8 type, BinReadable &tgt,Store *s ) const
{
    bool parse_ok=true;
    switch ( type )
    {
    case 10:
        {
            Color3ub rgb;
            parse_ok &= s->read(rgb.v[0]);
            parse_ok &= s->read(rgb.v[1]);
            parse_ok &= s->read(rgb.v[2]);
            (tgt.*pval) = rgb;
            s->fixup();
        }
        break;
    default:
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Unexpected field type:%d expected a 3 byte color.\n"), type),false);
        break;
    }
    return parse_ok;
}
template <>
bool TemplateField<u16>::do_read(u8 type, BinReadable &tgt,Store *s ) const
{
    bool parse_ok=true;
    switch ( type )
    {
    case 11:
        {
            u16 v;
            parse_ok &= s->read(v);
            (tgt.*pval) = v;
            s->fixup();
        }
        break;
    default:
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Unexpected field type:%d expected an unsigned 16 but value.\n"), type),false);
        break;
    }
    return parse_ok;
}
template <>
bool TemplateField< vector<u32> >::do_read(u8 type, BinReadable &tgt,Store *s ) const
{
    bool parse_ok=true;
    switch ( type )
    {
    case 13:
        {
            vector<u32> &res((tgt.*pval));
            u32 to_read = 0;
            parse_ok &= s->read(to_read);
            res.clear();
            if ( 0==to_read)
                break;
            for(size_t idx = 0; idx < to_read; ++idx)
            {
                res.push_back(0);
                parse_ok &= s->read(res[idx]);
            }
        }
        break;
    default:
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Unexpected field type:%d expected a vector of u32.\n"), type),false);
        break;
    }
    return parse_ok;
}


template <>
bool TemplateField< vector<float> >::do_read(u8 type, BinReadable &tgt,Store *s ) const
{
    bool parse_ok=true;
    switch ( type )
    {
    case 14:
        {
            vector<float> &res((tgt.*pval));
            u32 to_read = 0;
            parse_ok &= s->read(to_read);
            res.clear();
            if ( 0==to_read)
                break;
            for(size_t idx = 0; idx < to_read; ++idx)
            {
                res.push_back(0);
                parse_ok &= s->read(res[idx]);
            }
        }
        break;
    default:
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Unexpected field type:%d expected a vector of floats.\n"), type),false);
        break;
    }
    return parse_ok;
}
template <>
bool TemplateField< vector<string> >::do_read(u8 type, BinReadable &tgt,Store *s ) const
{
    bool parse_ok=true;
    switch ( type )
    {
    case 15:
        {
            vector<string> &res((tgt.*pval));
            u32 to_read = 0;
            parse_ok &= s->read(to_read);
            res.clear();
            if ( 0==to_read)
                break;
            for(size_t idx = 0; idx < to_read; ++idx)
            {
                res.push_back(s->read_str(12000));
                //parse_ok &= res[idx].size()>0; TODO handle string read errors
            }
        }
        break;
    default:
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Unexpected field type:%d expected a vector of strings.\n"), type),false);
        break;
    }
    return parse_ok;
}
template <>
bool TemplateField< u8 * >::do_read(u8 type, BinReadable &tgt,Store *s ) const
{
    bool parse_ok=true;
    switch ( type )
    {
    case 17:
        {
            u8 *v = (tgt.*pval);
            if(v)
                delete [] v;
            (tgt.*pval) = new u8 [m_param];
            parse_ok &= s->read_bytes((char *)(tgt.*pval),m_param);
            s->fixup();
        }
    default:
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) Unexpected field type:%d expected a raw character array.\n"), type),false);
        break;
    }
    return parse_ok;
}

bool Field::read_non_nested( BinReadable &tgt,Store *s ) const
{
    bool parse_ok=true;
     if((m_type&0x100) || ((m_type&0xFF) - 3>17))
        return parse_ok;
    u8 type=(m_type&0xFF) - 3;
    if(type==16)
        return parse_ok;
    return this->do_read(type,tgt,s);
 }
bool Field::read_nested( BinReadable &tgt,Store *s ) const
{

    switch(m_type)
    {
    case 0x15: // array of entries
        {
            return this->do_read_nested(tgt,s);
        }
    case 0x17:
        {
            assert(!"Not implemented nested read");
        }
        break;
    case 0x13:
        {
            assert(!"Not implemented nested read");
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
    m_schema.add_field("",0x20C,0x0,0x0,(Vec3 BinReadable::*)&ColorEntry::rgb);
    m_schema.add_end();
}

void ColorStorage::build_schema()
{
    if(schema_initialized)
        return;
    schema_initialized=true;

    ColorEntry::build_schema();
    m_schema.add_field_nested("Color",0x15,0x0,0xC,&ColorEntry::m_schema,(std::vector<BinReadable *>  BinReadable::*)&ColorStorage::m_colors);
    m_schema.add_end("EndPalette");
}

void ClassSchema::internal_add_field(Field *fld)
{
    m_fields.push_back(fld);
    m_map_names[fld->m_name] = 1+(m_fields.size()-1);

}

bool ClassSchema::read( BinReadable &tgt,Store *s ) const
{
    bool result=true;
    size_t field_idx;
    s->prepare();
    for(field_idx=0; field_idx<m_fields.size(); ++field_idx)
    {
        result &= m_fields[field_idx]->read_non_nested(tgt,s);
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
            assert(!"unknown field referenced.");
        // create instance of field value here ?
        result &= m_fields[field_idx-1]->read_nested(tgt,s);
    }
    return result;
}

void ClassSchema::calc_crc( CrcVisitor &v ) const
{
    for(size_t idx=0; idx<m_fields.size(); ++idx)
        m_fields[idx]->calc_crc(v);
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
