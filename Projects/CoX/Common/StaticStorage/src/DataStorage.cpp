/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#include <ace/ACE.h>
#include "DataStorage.h"
#include "ReadableStructures.h"
bool BinStore::check_bin_version_and_crc(const ClassSchema *)
{
    std::string tgt;
    uint32_t crc_from_file;
    char magic_contents[8];
    m_str.read(magic_contents,8);
    read(crc_from_file);
    tgt=read_pstr(4096);
    if ( strncmp(magic_contents,"CrypticS",8) || strncmp(tgt.c_str(),"Parse4",6) ) //|| crc_from_file != m_required_crc
    {
        m_str.close();
        return false;
    }
    return true;
}
std::string BinStore::read_pstr( size_t maxlen )
{
    uint16_t len=0;
    bool res=read(len);
    if(res!=true)
        return "";
    if(len<=maxlen)
    {
        char *buf=new char[len+1];
        m_str.read(buf,len);
        buf[len]=0;
        std::string res=buf;
        delete [] buf;
        if(m_file_sizes.size()>0)
        {
            (*m_file_sizes.rbegin())-=len;
            bytes_read+=len;
        }
        fixup();
        return res;
    }
    return "";
}

void BinStore::skip_pstr()
{
    uint16_t len=0;
    read(len);
    m_str.seekg(len,std::ios_base::cur);
}

bool BinStore::read_data_blocks( bool file_data_blocks )
{
    if(!file_data_blocks)
    {
        skip_pstr();
        uint32_t v;
        read(v);
        if(v)
            m_str.seekg(v,std::ios_base::cur);
        return true;
    }
    std::string hdr=read_pstr(20);
    int sz;
    read_internal(sz);

    std::ifstream::pos_type read_start = m_str.tellg();
    if(hdr.compare("Files1")||sz<=0)
        return false;
    int num_data_blocks;
    read_internal(num_data_blocks);
    for (int blk_idx=0; blk_idx<num_data_blocks; ++blk_idx)
    {
        FileEntry fe;
        fe.name = read_pstr(260);
        read_internal(fe.date);
        m_entries.push_back(fe);
    }
    std::ifstream::pos_type read_end = m_str.tellg();
    m_str.seekg(0,std::ios_base::end);
    m_file_sizes.push_back(m_str.tellg()-read_end);
    m_str.seekg(read_end,std::ios_base::beg);

    return (sz==(read_end-read_start));
}

bool BinStore::open( const ClassSchema *s,const std::string &name )
{
    if(!m_str.is_open())
    {
        m_str.open(name.c_str(),std::ios_base::binary|std::ios_base::in);
    }
    if(!m_str.is_open())
        return false;
    CrcVisitor v;
    s->calc_crc(v);
    m_required_crc=v.result();
    bool result = check_bin_version_and_crc(s);
    return result && read_data_blocks(true);
}

bool BinStore::read( uint32_t &v )
{
    size_t res = read_internal(v);
    return res==4;
}

bool BinStore::read( float &v )
{
    size_t res = read_internal(v);
    return res==4;
}

bool BinStore::read( uint16_t &v )
{
    size_t res = read_internal(v);
    return res==2;
}

bool BinStore::read( uint8_t &v )
{
    size_t res = read_internal(v);
    return res==1;
}

bool BinStore::read(Vec2 &val)
{
    bool parse_ok=true;
    parse_ok &= read(val.v[0]);
    parse_ok &= read(val.v[1]);
    return parse_ok;
}
bool BinStore::read(std::vector<std::string> &res)
{
    bool parse_ok=true;
    uint32_t to_read = 0;
    parse_ok &= read(to_read);
    res.clear();
    if ( 0==to_read)
        return parse_ok;
    for(size_t idx = 0; idx < to_read; ++idx)
    {
        res.push_back(read_str(12000));
        //parse_ok &= res[idx].size()>0; TODO handle string read errors
    }
    return parse_ok;
}
bool BinStore::read(std::vector<float> &res)
{
    bool parse_ok=true;
    uint32_t to_read = 0;
    parse_ok &= read(to_read);
    res.clear();
    if ( 0==to_read)
        return parse_ok;
    for(size_t idx = 0; idx < to_read; ++idx)
    {
        res.push_back(0);
        parse_ok &= read(res[idx]);
    }
    return parse_ok;
}
bool BinStore::read_bytes( char *tgt,size_t sz )
{
    m_str.read(tgt,sz);
    bytes_read+=sz;
    return true;
}

std::string BinStore::read_str( size_t maxlen )
{
    std::string result(read_pstr(maxlen));
    fixup();
    return result;
}

void BinStore::prepare()
{
    read_internal(bytes_to_read);
    bytes_read=0;
}

uint32_t BinStore::read_header( std::string &name,size_t maxlen )
{
    name = read_pstr(maxlen);
    uint32_t res;
    if(4!=read_internal(res) || res==0)
        return ~0U;
    return res;
}

bool BinStore::prepare_nested()
{
    bool result= bytes_to_read==bytes_read;
    assert(bytes_to_read==bytes_read);
    bytes_to_read = *m_file_sizes.rbegin();
    return result;
}

bool BinStore::nesting_name( std::string &name )
{
    uint32_t expected_size = read_header(name,12000);
    if(expected_size == uint32_t(~0))
        return false;
    bytes_to_read = expected_size;
    if(m_file_sizes.size()>0)
        (*m_file_sizes.rbegin())-=bytes_to_read;
    m_file_sizes.push_back(bytes_to_read); // the size of structure being read. + sizeof(uint32_t)
    return true;
}

void BinStore::nest_in( const Field * )
{

}

void BinStore::nest_out( const Field * )
{
    //size_t strctsz = 4+bytes_read;
    m_file_sizes.pop_back();
    //(*m_file_sizes.rbegin()) -= strctsz;
}

void BinStore::fixup()
{
    std::ifstream::pos_type nonmult4 = ((m_str.tellg() + std::ifstream::pos_type(3)) & ~3) - m_str.tellg();
    if(nonmult4)
    {
        m_str.seekg(nonmult4,std::ios_base::cur);
        bytes_read+=nonmult4;
        if(m_file_sizes.size()>0)
            (*m_file_sizes.rbegin())-=nonmult4;

    }
}

bool BinStore::end_encountered()
{
    return (*m_file_sizes.rbegin())==0;
}
Store * StoreFactory::store_for_file( const ClassSchema *schema,const std::string &str )
{
    BinStore *res =new BinStore();
    if(res->open(schema,str))
        return res;
    delete res;
    return 0;
}

void StoreFactory::release( Store *r )
{
    delete r;
}
