/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <set>
#include <deque>
#include <vector>
#include <stdint.h>
#include <chrono>
#include <memory>

class PacketCollector;
static const uint32_t maxPacketSize    = 1472;
static const uint32_t packetHeaderSize = 8;
class BitStream;
class QString;
class CrudP_Packet
{
    using time_point = std::chrono::steady_clock::time_point;
public:
    friend class PacketCollector;

    CrudP_Packet();
    explicit CrudP_Packet(const CrudP_Packet &);
    CrudP_Packet(BitStream *stream, bool hasDebugInfo);
    ~CrudP_Packet();

    uint32_t  GetBits(uint32_t nBits);
    uint32_t  GetPackedBits(uint32_t nBits);
    void GetString(QString &str);

    float GetFloat();
    void StoreBits(uint32_t nBits, uint32_t dataBits);
    void StoreBitArray(uint8_t *array, size_t nBits);
    void StorePackedBits(uint32_t nBits, uint32_t dataBits);
    void StoreString(const char *str);
    void CompressAndStoreString(const char *str);

    //  Sets the packet to the "finalized" state.
    //  This means that it's header is built, acks
    //  are stored in it, etc..
    void SetFinalized() { m_finalized = true; }

    //  Accessors
    //////////////////////////////////////////////////////////////////////////
    uint8_t  *  GetBuffer();
    size_t      GetPacketLength()   const;
    BitStream * GetStream()                 { return m_stream;                      }
    bool        getIsCompressed()   const   { return m_compressed;                  }
    bool        HasSiblings()       const   { return m_numSibs > 0;                 }
    bool        IsFinalized()       const   { return m_finalized;                   }
    bool        isReliable()        const   { return m_reliable;                    }
    bool        compressRequested() const   { return m_compress_on_send;            }


    uint32_t GetSequenceNumber()    const   { return m_seqNo;  }
    uint32_t GetSiblingPosition()   const   { return m_sibPos; }
    uint32_t getNumSibs()           const   { return m_numSibs;}
    uint32_t getSibId()             const   { return m_sibId;}
    uint32_t getSibPos()            const   { return m_sibPos;}
    size_t   getNumAcks()           const   { return m_acks.size(); }

    void ByteAlign();
    void SetStream(BitStream *stream)       { m_stream = stream; }
    void SetIsCompressed(bool compressed)   { m_compressed = compressed; }
    void SetReliabilty(bool r)              { m_reliable = r; }
    void setSeqNo(uint32_t n)               { m_seqNo=n; }
    void setNumSibs(uint32_t n)             { m_numSibs=n; }
    void setSibId(uint32_t n)               { m_sibId=n; }
    void setSibPos(uint32_t n)              { m_sibPos=n; }
    void setContents(const BitStream &t);
    void addAck(uint32_t id)                { m_acks.insert(id); }
    uint32_t getNextAck();
    void dump() const;
    time_point creationTime() const         { return m_creation_time; }
    time_point lastSend() const             { return m_xfer_time; }
    void setLastSend(time_point t)          { m_xfer_time=t; }
    void incRetransmits()                   { m_retransmit_count++;}
    uint32_t retransmitCount() const        { return m_retransmit_count; }
protected:

    BitStream *m_stream;
    bool m_compressed, m_finalized,m_reliable,m_compress_on_send=false;
    uint32_t m_seqNo;
    uint32_t m_numSibs;
    uint32_t m_sibId;
    uint32_t m_sibPos;
    time_point m_creation_time = std::chrono::steady_clock::now();
    time_point m_xfer_time;
    uint32_t m_retransmit_count;
    std::set<uint32_t> m_acks;
};
using lCrudP_Packet = std::deque<std::unique_ptr<CrudP_Packet>>;
using vCrudP_Packet = std::vector<CrudP_Packet *>;
using ivCrudP_Packet = vCrudP_Packet::iterator;
