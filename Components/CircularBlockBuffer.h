/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <cstring>
#include <cstdint>
#include <cassert>

/**
 * @brief The block circular buffer class is meant to be used as a very fast serialization target 
 * for a bunch of 'blocks' of different sizes. In case of SEGS those blocks are serialized 'events'.
 * Expected usage when writing:
 * \code{.cpp}
 * // somewhere in class
 * block_circular_buffer m_buffer;
 * 
 * void to_buffer(Event *ev)
 * {
 *   m_buffer.begin_block();
 *   BinaryBufferOutputArchive ar(m_buffer);
 *   serialize_to(ar);
 *   m_buffer.end_block();
 * }
 * \endcode
 * When reading, read_block should be called with preallocated memory and it's size. 
 * If any blocks can be retrieved, the data will be stored in provided memory, and actual block size returned.
 * 
 * \sa BinaryBufferOutputArchive
 * 
 */

//TODO: this class needs more extensive testing.
class CircularBlockBuffer {
public:
    explicit CircularBlockBuffer(uint32_t size) :
        m_memory(std::unique_ptr<uint8_t[]>(new uint8_t[size])),
        m_max_size(size)
    {
    }
    
    void end_block()
    {
        // write the block length to allow reading the blocks from buffer.
        uint32_t count;
        if(m_wr_offset>m_rd_offset)
            count = m_wr_offset-m_rd_offset;
        else
            count = m_max_size + m_wr_offset -m_rd_offset;
        write((const uint8_t *)&count,sizeof(count));
    }
    void begin_block() {
        m_rd_offset = m_wr_offset;
    }
    void write(const uint8_t *data,uint32_t count)
    {
        assert(count<m_max_size);
        const uint32_t bytes_write1 = std::min(count, m_max_size - m_wr_offset);
        memcpy(m_memory.get() + m_wr_offset, data, bytes_write1);
        memcpy(m_memory.get(), data + bytes_write1, count - bytes_write1);
        m_wr_offset = (m_wr_offset + count) % m_max_size;
    }
    uint32_t read_block(uint8_t *data,uint32_t buf_size)
    {
        // read size before the wr_ptr
        uint32_t sz=0;
        uint32_t before=((m_max_size + m_rd_offset - 4)%m_max_size);
        // check if we would roll over wr_ptr
        if(m_rd_offset>m_wr_offset && before<m_wr_offset)
            return 0;
        read((uint8_t *)&sz,sizeof(sz));
        if(sz==0 || sz>=buf_size)
            return 0;
        before=((m_max_size + m_rd_offset - sz)%m_max_size);
        if(m_rd_offset>m_wr_offset && before<m_wr_offset)
            return 0;
        read(data,sz);
        return sz;
    }
private:
    void read(uint8_t *data, uint32_t count)
    {
        assert(count < m_max_size);
        uint32_t read_start  = (m_max_size + m_rd_offset - count) % m_max_size;
        uint32_t bytes_read1 = std::min(m_max_size - read_start, count);
        memcpy(data, m_memory.get() + read_start, bytes_read1);
        memcpy(data + bytes_read1, m_memory.get(), count - bytes_read1);
        m_rd_offset = read_start;
    }
    friend void emergency_store(CircularBlockBuffer &,int fd);
    std::unique_ptr<uint8_t[]> m_memory;
    uint32_t m_wr_offset = 0;
    uint32_t m_rd_offset = 0;
    const uint32_t m_max_size;
};
