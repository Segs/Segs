/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "BitStream.h"
#include <set>
#include <list>
//#include "Opcodes.h"

class PacketCollector;
static const uint32_t maxPacketSize    = 0x5C0;
static const uint32_t packetHeaderSize = 8;
class CrudP_Packet
{
public:
    friend class PacketCollector;

    CrudP_Packet();
    explicit CrudP_Packet(const CrudP_Packet &);
    CrudP_Packet(BitStream *stream, bool hasDebugInfo);
    ~CrudP_Packet();

    uint32_t  GetBits(uint32_t nBits);
    void GetBitArray(uint32_t nBytes, uint8_t *array);
    uint32_t  GetPackedBits(uint32_t nBits);
    void GetString(QString &str);

    float  GetFloat();
    void StoreBits(uint32_t nBits, uint32_t dataBits)   { m_stream->StoreBits(nBits, dataBits); }
    void StoreBitArray(uint8_t *array, size_t nBits)    { m_stream->StoreBitArray(array,nBits); }
    void StorePackedBits(uint32_t nBits, uint32_t dataBits) { m_stream->StorePackedBits(nBits, dataBits); }
    void StoreString(const char *str)                   { m_stream->StoreString(str); }
    void CompressAndStoreString(const char *str);

    //  Sets the packet to the "finalized" state.
    //  This means that it's header is built, acks
    //  are stored in it, etc..
    void SetFinalized() { m_finalized = true; }

    //  Accessors
    //////////////////////////////////////////////////////////////////////////
    uint32_t    GetPackedBitsLength(uint32_t len, uint32_t dataBits) { return m_stream->GetPackedBitsLength(len, dataBits); }
    uint8_t  *  GetBuffer()         const   { return (uint8_t *)m_stream->GetBuffer(); }
    size_t      GetPacketLength()   const   { return m_stream->GetReadableDataSize();}
    BitStream * GetStream()                 { return m_stream;                      }
    bool        HasDebugInfo()      const   { return m_hasDebugInfo;                }
    bool        getIsCompressed()   const   { return m_compressed;                  }
    bool        HasSiblings()       const   { return m_numSibs > 0;                 }
    bool        IsFinalized()       const   { return m_finalized;                   }

    void SetBufferLength(uint32_t length)   { m_stream->SetByteLength(length); }

    uint32_t GetSequenceNumber()    const   { return m_seqNo;  }
    uint32_t GetSiblingPosition()   const   { return m_sibPos; }
    uint32_t getNumSibs()           const   { return m_numSibs;}
    uint32_t getSibId()             const   { return m_sibId;}
    uint32_t getSibPos()            const   { return m_sibPos;}
    void ByteAlign()                        { m_stream->ByteAlign(); }
    void SetStream(BitStream *stream)       { m_stream = stream; }
    void SetHasDebugInfo(bool hasDebugInfo) { m_hasDebugInfo = hasDebugInfo; }
    void SetIsCompressed(bool compressed)   { m_compressed = compressed; }
    void setSeqNo(uint32_t n)               { m_seqNo=n; }
    void setNumSibs(uint32_t n)             { m_numSibs=n; }
    void setSibId(uint32_t n)               { m_sibId=n; }
    void setSibPos(uint32_t n)              { m_sibPos=n; }
    void setContents(const BitStream &t);
    void addAck(uint32_t id)                { m_acks.insert(id); }
    size_t getNumAcks()                     { return m_acks.size(); }
    uint32_t getNextAck();
    void dump();
    uint32_t m_checksum;
protected:

    BitStream *m_stream;
    bool m_hasDebugInfo, m_compressed, m_finalized;
    uint32_t m_seqNo;
    uint32_t m_numSibs;
    uint32_t m_sibId;
    uint32_t m_sibPos;
    std::set<uint32_t> m_acks;
};
typedef std::list<CrudP_Packet *> lCrudP_Packet;
typedef std::vector<CrudP_Packet *> vCrudP_Packet;
typedef vCrudP_Packet::iterator ivCrudP_Packet;
