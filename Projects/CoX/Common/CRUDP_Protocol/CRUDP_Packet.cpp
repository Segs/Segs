/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup CRUDP_Protocol Projects/CoX/Common/CRUDP_Protocol
 * @{
 */

#include "CRUDP_Packet.h"

#include <QtCore/QDebug>

CrudP_Packet::CrudP_Packet()
{
    m_stream        = new BitStream(maxPacketSize);
    m_finalized     = false;
    m_compressed    = false;
    m_seqNo         = 0;
    m_numSibs       = 0;
    m_sibId         = 0;
    m_sibPos        = 0;
    m_reliable      = true;
    m_retransmit_count = 0;
}

CrudP_Packet::CrudP_Packet(BitStream *stream, bool /*hasDebugInfo*/)
{
    m_stream        = stream;
    m_finalized     = false;
    m_compressed    = false;
    m_seqNo         = 0;
    m_numSibs       = 0;
    m_sibId         = 0;
    m_sibPos        = 0;
    m_reliable      = true;
    m_retransmit_count = 0;

}

CrudP_Packet::CrudP_Packet(const CrudP_Packet &from)
{
    m_stream        = nullptr; // we're not copying streams !!!
    m_finalized     = from.m_finalized;
    m_compressed    = from.m_compressed;
    m_seqNo         = from.m_seqNo;
    m_numSibs       = from.m_numSibs;
    m_sibId         = from.m_sibId;
    m_sibPos        = from.m_sibPos;
    m_reliable      = from.m_reliable;
    m_retransmit_count = from.m_retransmit_count;
    m_xfer_time     = from.m_xfer_time;

}

CrudP_Packet::~CrudP_Packet()
{
    delete m_stream;
}

uint32_t CrudP_Packet::GetBits(uint32_t nBits)
{
    return m_stream->GetBits(nBits);
}

void CrudP_Packet::GetBitArray(uint32_t nBytes, uint8_t *array)
{
    return m_stream->GetBitArray(array,nBytes);
}

uint32_t CrudP_Packet::GetPackedBits(uint32_t nBits)
{
    return m_stream->GetPackedBits(nBits);
}

void CrudP_Packet::GetString(QString &tgt)
{
    m_stream->GetString(tgt);
}

float CrudP_Packet::GetFloat()
{
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
    qDebug() << "CrudP_Packet debug dump:";
    qDebug().nospace() << "\tSeqence Number 0x" << QString::number(m_seqNo,16);
    qDebug() << "\tSiblings "<<m_numSibs;
    qDebug() << "\tContains "<<getNumAcks()<<"acks";
    qDebug() << "\tCompressed"<<m_compressed;
}

uint32_t CrudP_Packet::getNextAck()
{
    uint32_t res = *m_acks.begin();
    m_acks.erase(m_acks.begin());
    return res;
}

//! @}
