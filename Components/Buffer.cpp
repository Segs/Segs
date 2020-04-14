/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "Buffer.h"
#include <cassert>
#include <string>
#include <algorithm> // for std::min/max

GrowingBuffer::GrowingBuffer(uint8_t *buf, uint32_t size, bool take_ownership)
{
    m_buf       = nullptr;
    m_size      = 0;
    m_safe_area = 0;
    m_last_err  = 0;
    m_write_off = m_read_off = 0;
    m_max_size  = size>DEFAULT_MAX_SIZE ? size:DEFAULT_MAX_SIZE;
    if(take_ownership)
    {
        m_buf  = buf;
        m_size = size;
        Reset();
    }
    else
    {
        int alloc_result=resize(size);
        assert(alloc_result==0);
        uPutBytes(buf,size);
    }
}

GrowingBuffer::GrowingBuffer(uint32_t max_size,uint8_t safe_area, uint32_t pre_alloc_size)
{
    m_buf = nullptr;
    m_safe_area = safe_area;
    m_size = (pre_alloc_size+7)&(~7U);
    m_max_size = max_size;
    m_last_err = 0;
    m_write_off=m_read_off=0;
    if(pre_alloc_size)
    {
        m_buf = new uint8_t[m_size+m_safe_area];
        memset(m_buf,0,m_size);
        assert(m_buf!=nullptr);
    }
    Reset();
}
GrowingBuffer::GrowingBuffer(const GrowingBuffer &from)
{
    m_size      = from.m_size;
    m_buf       = new uint8_t[m_size];
    m_last_err  = 0;
    m_write_off = from.m_write_off;
    m_safe_area = from.m_safe_area;
    m_read_off  = from.m_read_off;
    m_max_size  = from.m_max_size;
    if(m_buf&&from.m_buf)
        memcpy(m_buf,from.m_buf,m_write_off); // copy up to write point
    
}

GrowingBuffer::GrowingBuffer(GrowingBuffer &&from) noexcept
{
    std::swap(m_size,from.m_size);
    std::swap(m_buf,from.m_buf);
    std::swap(m_last_err,from.m_last_err);
    std::swap(m_write_off,from.m_write_off);
    std::swap(m_safe_area,from.m_safe_area);
    std::swap(m_read_off,from.m_read_off);
    std::swap(m_max_size,from.m_max_size);
}
GrowingBuffer::~GrowingBuffer()
{
    delete []m_buf;
    m_write_off=m_read_off=0;
    m_buf = nullptr;
}
void GrowingBuffer::PutString(const char *t)
{
    uint32_t len = uint32_t(strlen(t)+1);
    PutBytes(reinterpret_cast<const uint8_t *>(t),len);
}
void GrowingBuffer::uPutString(const char *t)
{
    uint32_t len = uint32_t(strlen(t));
    uPutBytes(reinterpret_cast<const uint8_t *>(t),len);
}

void GrowingBuffer::PutBytes(const uint8_t *t, uint32_t len)
{
    if(m_write_off+len>m_size) {
        if(resize(m_write_off+len)==-1) // space exhausted
        {
            m_last_err = 1;
            return;
        }
    }
    uPutBytes(t,len);
}
void GrowingBuffer::uPutBytes(const uint8_t *t, uint32_t len)
{
    if(!(m_buf&&t))
        return;
    memcpy(&m_buf[m_write_off],t,len);
    m_write_off+=len;
}

void GrowingBuffer::GetString(char *t)
{
    if(GetReadableDataSize()==0)
    {
        m_last_err = 1;
        return;
    }
    uint32_t len = uint32_t(strlen((char *)&m_buf[m_read_off]));
    if((0==len) || len>GetReadableDataSize())
    {
        m_last_err = 1;
        return;
    }
    uGetBytes(reinterpret_cast<uint8_t *>(t),len);
}
void GrowingBuffer::uGetString(char *t)
{
    uint32_t len= uint32_t(strlen((char *)&m_buf[m_read_off]));
    uGetBytes(reinterpret_cast<uint8_t *>(t),len);
}

bool GrowingBuffer::GetBytes(uint8_t *t, uint32_t len)
{
    if(len>GetReadableDataSize())
        return false;
    uGetBytes(t,len);
    return true;
}
void GrowingBuffer::uGetBytes(uint8_t *t, uint32_t len)
{
    memcpy(t,&m_buf[m_read_off],len);
    m_read_off += len;
}
void GrowingBuffer::PopFront(uint32_t pop_count)
{
    if(pop_count>m_size)
    {
        m_write_off=0;
        m_read_off=0;
        return;
    }
    if(m_write_off < pop_count) // if there is any reason to memmove
    {
        m_write_off = m_read_off = 0;
        return;
    }
    memmove(m_buf,&m_buf[pop_count],m_write_off-pop_count); // shift buffer contents to the left
    m_write_off-=pop_count;
    if(m_read_off<pop_count)
        m_read_off=0;
    else
        m_read_off-=pop_count;

}

/** this method will try to resize GrowingBuffer to accommodate_size elements (in reality it preallocates a 'few' more )
 if the new size is 0, then internal buffer object is deleted, freeing all memory
 Warning: when buffer is growing, only it's part that contains any valid data is copied (i.e. from start, to write_off )
 returns -2 if there were problems allocating new block of memory for the internal storage
 returns -1 if new size exceeds maximum size allowed for this buffer
 returns 0 if everything went ok
*/
int GrowingBuffer::resize(uint32_t accommodate_size)
{
    uint32_t new_size = accommodate_size ? 2*accommodate_size+1 : 0;
    if(accommodate_size>m_max_size)
        return -1;
    if(accommodate_size<m_size)
        return 0;
    assert(accommodate_size<0x100000);
    new_size = std::min<uint32_t>(new_size,m_max_size);
    // fix read/write indexers ( it'll happen only if new size is less then current size)
    if(m_read_off>new_size)
        m_read_off  = new_size;
    if(m_write_off>new_size)
        m_write_off = new_size;

    if(0==new_size) // requested freeing of internal buffer
    {
        delete [] m_buf;
        m_buf = nullptr; // this allows us to catch calls through Unchecked methods quickly
        m_size= new_size;
        return 0;
    }
    if(new_size>m_size)
    {
        uint8_t *tmp = new uint8_t[new_size+m_safe_area];
        if(nullptr==tmp)
            return -2;
        assert(m_write_off<=m_size); // just to be sure
        if(m_write_off>1)
            memcpy(tmp,m_buf,m_size); // copying old contents, up to actual m_write_off
        memset(&tmp[m_size],0,new_size+m_safe_area-m_size);
        delete [] m_buf;
        m_buf = tmp;
        m_size = new_size;
    }
    return 0;
}

//! @}
