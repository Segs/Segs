/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "DataStorage.h"
#include "Components/Colors.h"

#include <QtCore/QString>
#include <QtCore/QFileInfo>

bool BinStore::check_bin_version_and_crc(uint32_t req_crc)
{
    QString tgt;
    uint32_t crc_from_file;
    char magic_contents[8];
    m_device->read(magic_contents,8);
    read(crc_from_file);
    tgt=read_pstr(4096);

    if( 0!=strncmp(magic_contents,"CrypticS",8) || tgt.midRef(0,5)!="Parse" || (req_crc!=0 && crc_from_file != req_crc) ) //
    {
        m_device->close();
        return false;
    }
    int parse_version = tgt.midRef(5,1).toInt();
    if(parse_version!=4 && parse_version!=6)  {
        qDebug() << "Unsupported bin file version"<<parse_version;
        m_device->close();
        return false;
    }
    m_version = parse_version;
    return true;
}

const QByteArray &BinStore::read_pstr( size_t maxlen )
{
    static QByteArray buf;
    uint16_t len=0;
    buf.resize(0);
    if(read_internal(len)!=2)
        return buf;
    if(len<=maxlen)
    {
        buf.resize(len);
        m_device->read(buf.data(),len);
        if(isI24Data()) {
            const int len_based_fixup =  (4 - (len + sizeof(uint16_t)) % 4) % 4;
            m_device->seek(len_based_fixup+m_device->pos());
            bytes_read+=len+len_based_fixup;
            return buf;
        }
        if(m_file_sizes.size()>0)
        {
            (*m_file_sizes.rbegin())-=len;
            bytes_read+=len;
        }
        fixup();
        return buf;
    }
    return buf;
}

void BinStore::skip_pstr()
{
    uint16_t len=0;
    read(len);
    m_device->seek(len+m_device->pos());
}
bool BinStore::read_data_blocks( bool file_data_blocks )
{
    if(!file_data_blocks)
    {
        skip_pstr();
        uint32_t v;
        read(v);
        if(v)
            m_device->seek(v+m_device->pos());
        return true;
    }
    const QByteArray &hdr(read_pstr(20));
    uint32_t sz;
    read_internal(sz);

    quint64 read_start = m_device->pos();
    if(!hdr.startsWith("Files1")||sz<=0)
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
    quint64 read_end = m_device->pos();
    m_file_sizes.push_back(m_device->size()-read_end);
    return (sz==(read_end-read_start));
}

bool BinStore::open(FSWrapper& fs, const QString &name,uint32_t required_crc )
{
    if(m_device && m_device->isOpen())
    {
        m_device->close();
        delete m_device;
        m_device = nullptr;
    }
    m_device=fs.open(name,true);
    if(!m_device) {
            return false;
    }
    bool result = check_bin_version_and_crc(required_crc);
    return result && read_data_blocks(true);
}

BinStore::~BinStore()
{
    if(m_device && m_device->isOpen())
    {
        m_device->close();
    }
    delete m_device;
    m_device = nullptr;
}

bool BinStore::read( uint32_t &v )
{
    size_t res = read_internal(v);
    return res==4;
}

bool BinStore::read( int32_t &v )
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
    if(isI24Data()) {
        uint32_t val;
        size_t res = read_internal(val);
        v=(uint16_t)val;
        return res==4;
    }

    size_t res = read_internal(v);
    return res==2;
}

bool BinStore::read( uint8_t &v )
{
    if(isI24Data()) {
        uint32_t val;
        size_t res = read_internal(val);
        v=(uint8_t)val;
        return res==4;
    }
    size_t res = read_internal(v);
    return res==1;
}

bool BinStore::readU( uint8_t &v )
{
    size_t res = read_internal(v);
    uint8_t skipover;
    return res+read_internal(skipover) + read_internal(skipover) + read_internal(skipover)==4;
}
bool BinStore::read(Vec2 &val)
{
    bool parse_ok=true;
    parse_ok &= read(val[0]);
    parse_ok &= read(val[1]);
    return parse_ok;
}

bool BinStore::read(Vec3 &val)
{
    bool parse_ok=true;
    parse_ok &= read(val[0]);
    parse_ok &= read(val[1]);
    parse_ok &= read(val[2]);
    return parse_ok;
}

bool BinStore::read(RGBA & rgb)
{
    bool parse_ok=true;
    if(isI24Data()) {
        for(int i=0; i<3; ++i) {
            parse_ok &= read(rgb.v[i]);
        }
        rgb.v[3] = 0;
        return parse_ok;
    }
    for(int i=0; i<3; ++i)
        parse_ok &= read(rgb.v[i]);
    rgb.v[3] = 0;
    uint8_t skipped;
    read(skipped);
    return parse_ok;
}

bool BinStore::read(uint8_t *&val, uint32_t length)
{
    bool parse_ok=true;
    if(val)
        delete [] val;
    val = new uint8_t [length];
    parse_ok &= read_bytes((char *)val,length);
    fixup();
    return parse_ok;
}

bool BinStore::read(QByteArray &val)
{
    val=this->read_str(12000);
    return true;
}

bool BinStore::read(std::vector<QByteArray> &res)
{
    bool parse_ok=true;
    uint32_t to_read = 0;
    parse_ok &= read(to_read);
    if( 0==to_read)
        return parse_ok;
    for(size_t idx = 0; idx < to_read; ++idx)
    {
        res.push_back(read_str(12000));
        //parse_ok &= res[idx].size()>0; TODO handle string read errors
    }
    return parse_ok;
}

bool BinStore::read(std::vector<std::vector<QByteArray>> &res)
{
    bool parse_ok=true;
    uint32_t to_read = 0;
    parse_ok &= read(to_read);
    if( 0==to_read)
        return parse_ok;
    for(size_t idx = 0; idx < to_read; ++idx)
    {
        std::vector<QByteArray> entry;
        if(!read(entry)) {
            return false;
        }
        res.emplace_back(std::move(entry));
    }
    return parse_ok;
}

bool BinStore::read(std::vector<uint32_t> &res)
{
    bool parse_ok=true;
    uint32_t to_read = 0;
    parse_ok &= read(to_read);
    res.clear();
    if( 0==to_read)
        return parse_ok;
    for(size_t idx = 0; idx < to_read; ++idx)
    {
        res.push_back(0);
        parse_ok &= read(res[idx]);
    }
    return parse_ok;
}

bool BinStore::read(std::vector<int32_t> &res)
{
    bool parse_ok=true;
    uint32_t to_read = 0;
    parse_ok &= read(to_read);
    res.clear();
    if( 0==to_read)
        return parse_ok;
    for(size_t idx = 0; idx < to_read; ++idx)
    {
        res.push_back(0);
        parse_ok &= read(res[idx]);
    }
    return parse_ok;
}

bool BinStore::read(std::vector<float> &res)
{
    bool parse_ok=true;
    uint32_t to_read = 0;
    parse_ok &= read(to_read);
    res.clear();
    if( 0==to_read)
        return parse_ok;
    for(size_t idx = 0; idx < to_read; ++idx)
    {
        res.push_back(0);
        parse_ok &= read(res.back());
    }
    return parse_ok;
}

bool BinStore::read_bytes( char *tgt,size_t sz )
{
    m_device->read(tgt,sz);
    bytes_read+=sz;
    return true;
}

const QByteArray & BinStore::read_str( size_t maxlen )
{
    const QByteArray &result(read_pstr(maxlen));
    if(!isI24Data())
        fixup();
    return result;
}

void BinStore::prepare()
{
    read_internal(bytes_to_read);
    bytes_read=0;
}

uint32_t BinStore::read_header( QByteArray &name,size_t maxlen )
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

bool BinStore::nesting_name(QByteArray &name)
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

void BinStore::fixup()
{
    qint64 nonmult4 = ((m_device->pos() + 3) & ~3) - m_device->pos();
    if(nonmult4)
    {
        m_device->seek(nonmult4+m_device->pos());
        bytes_read+=nonmult4;
        if(m_file_sizes.size()>0)
            (*m_file_sizes.rbegin())-=nonmult4;

    }
}

bool BinStore::end_encountered() const
{
    return (*m_file_sizes.rbegin())==0;
}

//! @}
