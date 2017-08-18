/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "CRUDP_Packet.h"

#include <ace/Log_Msg.h>
#include <QtCore/QDebug>

CrudP_Packet::CrudP_Packet()
{
    m_stream        = new BitStream(maxPacketSize);
    m_hasDebugInfo  = false;
    m_finalized     = false;
    m_compressed    = false;
    m_seqNo         = 0;
    m_numSibs       = 0;
    m_sibId         = 0;
    m_sibPos        = 0;
}

CrudP_Packet::CrudP_Packet(BitStream *stream, bool hasDebugInfo)
{
    m_stream        = stream;
    m_hasDebugInfo  = hasDebugInfo;
    m_finalized     = false;
    m_compressed    = false;
    m_seqNo         = 0;
    m_numSibs       = 0;
    m_sibId         = 0;
    m_sibPos        = 0;

}
CrudP_Packet::CrudP_Packet(const CrudP_Packet &from)
{
    m_stream        = nullptr; // we're not copying streams !!!
    m_hasDebugInfo  = from.m_hasDebugInfo;
    m_finalized     = from.m_finalized;
    m_compressed    = from.m_compressed;
    m_seqNo         = from.m_seqNo;
    m_numSibs       = from.m_numSibs;
    m_sibId         = from.m_sibId;
    m_sibPos        = from.m_sibPos;

}
CrudP_Packet::~CrudP_Packet()
{
    if(m_stream) delete m_stream;
}

uint32_t CrudP_Packet::GetBits(uint32_t nBits)
{
    if(m_hasDebugInfo)
        return m_stream->GetBitsWithDebugInfo(nBits);
    return m_stream->GetBits(nBits);
}

void CrudP_Packet::GetBitArray(uint32_t nBytes, uint8_t *array)
{
    if(m_hasDebugInfo)
        return m_stream->GetBitArrayWithDebugInfo(array,nBytes);
    return m_stream->GetBitArray(array,nBytes);
};

uint32_t CrudP_Packet::GetPackedBits(uint32_t nBits)
{
    if(m_hasDebugInfo)
        return m_stream->GetPackedBitsWithDebugInfo(nBits);
    return m_stream->GetPackedBits(nBits);
}

void CrudP_Packet::GetString(QString &tgt)
{
    if(m_hasDebugInfo)
        m_stream->GetStringWithDebugInfo(tgt);
    else
        m_stream->GetString(tgt);
}

float CrudP_Packet::GetFloat()
{
    if(m_hasDebugInfo)
        return m_stream->GetFloatWithDebugInfo();
    return m_stream->GetFloat();
}
void CrudP_Packet::setContents(const BitStream &t)
{
    if(m_stream)
        *m_stream = t;
    else
        m_stream = new BitStream(t);
}

void CrudP_Packet::dump() const
{
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("CrudP_Packet debug dump:\n")));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tSeqence Number 0x%08x\n"),m_seqNo));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tSiblings %d\n"),m_numSibs));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tDebug info : %d \n"),int(m_hasDebugInfo)));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tContains %d acks\n"),getNumAcks()));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("\tCompressed %d\n"),m_compressed));
    //ACE_HEX_DUMP((LM_DEBUG,(char *)m_stream->read_ptr(),m_stream->GetReadableDataSize(),"contents"));
    ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("CrudP_Packet debug complete\n")));
}

uint32_t CrudP_Packet::getNextAck()
{
    uint32_t res = *m_acks.begin();
    m_acks.erase(m_acks.begin());
    return res;
}
