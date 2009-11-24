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
class BinReadable;
class CrcVisitor
{
public:
    void digest(const char *,size_t s)
    {

    }
    u32 result()
    {
        return 0;
    }
};

struct Vec3
{
    float v[3];
};
struct Vec2
{
    float v[2];
};
struct Color3ub
{
    u8 v[3];
};
struct Field;
class Store
{
public:
    virtual void    prepare()=0;
    virtual std::string read_str(size_t maxlen)=0;
    virtual bool    read_bytes(char *buffer,size_t sz)=0;
    virtual bool    read(u32 &v)=0;
    virtual bool    read(u16 &v)=0;
    virtual bool    read(u8 &v)=0;
    virtual bool    read(float &v)=0;
    virtual void    fixup()=0;
    virtual bool    prepare_nested()=0;
    virtual bool    nesting_name(std::string &name)=0;
    virtual void    nest_in(const Field *f)=0;
    virtual void    nest_out(const Field *f)=0;
    virtual bool    end_encountered()=0; // returns true if we should nest-out
};
class ClassSchema;
class BinStore : public Store
{
    u32 m_required_crc;
    std::ifstream m_str;
    template<class V>
    size_t read_internal(V &res)
    {
        if(m_file_sizes.size()>0 && current_fsize()<sizeof(V))
            return 0;
        m_str.read((char *)&res,sizeof(V));
        if(m_file_sizes.size()>0)
        {
            bytes_read+=sizeof(V);
            (*m_file_sizes.rbegin())-=sizeof(V);
        }
        return sizeof(V);
    }
    struct FileEntry {
        std::string name;
        u32 date;
    };
    std::vector<FileEntry> m_entries;
    size_t bytes_read;
    u32 bytes_to_read;
    std::vector<u32> m_file_sizes; // implicit stack

    std::string read_pstr(size_t maxlen);
    void skip_pstr();
    bool read_data_blocks(bool file_data_blocks);
    bool check_bin_version_and_crc(const ClassSchema *s);
    u32 current_fsize() {return *m_file_sizes.rbegin();}
    u32 read_header(std::string &name,size_t maxlen);
    void        fixup();
public:
                BinStore(){}
    bool        read(u32 &v);
    bool        read(float &v);
    bool        read_bytes(char *tgt,size_t sz);
    bool        read(u16 &v);
    bool        read(u8 &v);
    std::string read_str(size_t maxlen);
    void        prepare();
    bool        prepare_nested();
    bool        nesting_name(std::string &name);
    void        nest_in(const Field *f);
    void        nest_out(const Field *f);
    bool        end_encountered();
    bool        open(const ClassSchema *s,const std::string &name);
};
class StoreFactory
{
public:
    static Store *store_for_file(const ClassSchema *schema,const std::string &str);
    static void release(Store *r);
};
