/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup CRUDP_Protocol Projects/CoX/Common/CRUDP_Protocol
 * @{
 */

//#include "GameProtocol.h"
#include "CRUDP_Protocol.h"

#include "PacketCodec.h"
#include "Components/BitStream.h"


#include <cassert>

#include <QDebug>
using namespace std::chrono;
/**
 Cryptic Reliable UDP
 CrudP
 Incoming
  BitStream Block ->
   Parse Header [ Checksum + Decode ]
   Remove from reliable packet list those acknowledged by this header
   if seq_no < min_recv_seq, reject packet [out of order packet]
   Store/Join
   Record packet's seq_no in ack_list
   Sort according to sequence num
   if next packet in sequence is available, pass it to higher layer. and record min_recv_seq
 Outgoing
  BitStream ->
   Split stream
   Each block is Encoded, and Checksumed.
   Sibling packet id is recorded in the packet
   Packets are inserted into send queue.
   If this queue is not scheduled for send, it's scheduled now

*/
using namespace std::chrono;

namespace  {

void PacketDestroyer(CrudP_Packet *a)
{
    delete a;
}

int getPacketResendDelay(signed int attempts, int ping, int before_first)
{
    int delay;

    if( before_first )
        delay = 3 * ping / 2;
    else
        delay = 4 * ping;
    attempts = std::min(2,attempts);
    switch(attempts) {
        case 0: delay*=4; break;
        case 1: delay*=2; break;
        case 2:        break;
    }
    return std::min(std::max(delay,250),5000);
}

} // end of anonymous namespace

void PacketSibDestroyer(const std::pair<int, CrudP_Protocol::pPacketStorage> &a)
{
    for_each(a.second.begin(), a.second.end(), PacketDestroyer);
}

bool CrudP_Protocol::PacketSeqCompare(const CrudP_Packet *a,const CrudP_Packet *b)
{
    return a->GetSequenceNumber()<b->GetSequenceNumber();
}

bool CrudP_Protocol::PacketSibCompare(const CrudP_Packet *a,const CrudP_Packet *b)
{
    return a->GetSiblingPosition()<b->GetSiblingPosition();
}

CrudP_Protocol::~CrudP_Protocol()
{
    delete m_codec;
    m_codec= nullptr;
}

void CrudP_Protocol::clearQueues(bool recv_queue,bool clear_send_queue)
{
    //  seen_seq.clear();
    if(recv_queue)
    {
        for_each(avail_packets.begin(),avail_packets.end(),PacketDestroyer);
        for_each(sibling_map.begin(),sibling_map.end(),PacketSibDestroyer);
        sibling_map.clear();
        avail_packets.clear();
    }
    if(clear_send_queue)
    {
        std::lock_guard<std::mutex> grd(m_packets_mutex);
        for_each(send_queue.begin(),send_queue.end(),PacketDestroyer);
        for_each(reliable_packets.begin(),reliable_packets.end(),PacketDestroyer);
        retransmit_queue.clear();
        reliable_packets.clear();
        send_queue.clear();
    }
}

void CrudP_Protocol::ReceivedBlock(BitStream &src)
{
    uint32_t bitlength, checksum=0;
    //TODO: consider allowing codec-less operation ?
    if(!m_codec || src.GetReadableDataSize()<12)
        return;

    src.Get(bitlength);
    assert(src.GetReadableBits()>=bitlength-32);
    m_codec->Decrypt(src.read_ptr(),src.GetReadableDataSize());

    src.Get(checksum);

    uint32_t realcsum  = PacketCodecNull::Checksum(src.read_ptr(),src.GetReadableDataSize());
    if(realcsum!=checksum)
    {
        qWarning() << "Checksum error.";
        return;
    }
    m_last_activity = steady_clock::now();
    CrudP_Packet *res = new CrudP_Packet; //PacketFactory::newDataPacket;
    bool requested_debug_mode_bitstream = src.uGetBits(1);
    assert(false == requested_debug_mode_bitstream);
    res->setSeqNo(src.uGetBits(32));
    uint32_t sibcount = src.GetPackedBits(1);
    res->setNumSibs(sibcount);
    if(sibcount)
    {
        res->setSibPos(src.GetPackedBits(1));
        res->setSibId(src.uGetBits(32));
    }
    parseAcks(src,res);
    res->SetIsCompressed(src.GetBits(1));
    src.ByteAlign(true,false);
    // how much data did we actually read
    size_t bits_left=(bitlength-src.GetReadPos());
    res->StoreBitArray(src.read_ptr(),bits_left);
    PushRecvPacket(res);
}

void CrudP_Protocol::parseAcks(BitStream &src,CrudP_Packet *tgt)
{
    // FixMe: GetPackedBits() can return signed values which can cause the numUniqueAcks assignment to be extremely high.
    uint32_t numUniqueAcks = src.GetPackedBits(1);
    if(numUniqueAcks  == 0)
        return;
    uint32_t firstAck = src.GetBits(32);
    tgt->addAck(firstAck);
    for(uint32_t i = 1; i < numUniqueAcks; i++)
    {
        //  The first sequence number is sent in it's entirety.  Every subsequent
        //  number is sent as a delta between it, and it's predecessor.  This is
        //  a clever way of compressing the ack table, because if you have the
        //  first ack in the sequence, all you need is the delta to the next one
        //  to calculate it, and the Packed Bits format achieves much higher
        //  compression ratios with smaller values
        firstAck += src.GetPackedBits(1) + 1;
        tgt->addAck(firstAck);
    }
}

void CrudP_Protocol::storeAcks(BitStream &bs)
{
    //TODO: sort + binary search for id
    std::lock_guard<std::mutex> grd(m_packets_mutex);
    if(recv_acks.empty())
    {
        bs.StorePackedBits(1,0);
        return;
    }
    recv_acks.sort();
    recv_acks.unique();
    std::list<uint32_t>::iterator iter = recv_acks.begin();
    uint32_t num_acks = std::min<uint32_t>(recv_acks.size(),16); // store up to 16 acks
    bs.StorePackedBits(1,num_acks);

    uint32_t last_ack = *iter;
    bs.StoreBits(32,last_ack);
    --num_acks;
    ++iter;
    while(iter!=recv_acks.end())
    {
        bs.StorePackedBits(1,(*iter-last_ack)-1); // 0 means difference of 1
        last_ack = *iter;
        ++iter;
        if(0==(--num_acks))
            break;
    }
    recv_acks.erase(recv_acks.begin(),iter);
    m_unacked_count = recv_acks.size();
}

bool CrudP_Protocol::allSiblingsAvailable(uint32_t sibid)
{
    pPacketStorage &storage = sibling_map[sibid];
    size_t avail=0;
    for(CrudP_Packet *pak : storage)
    {
        avail+= (pak != nullptr);
    }
    return avail==storage.size();
}

void CrudP_Protocol::PushRecvPacket(CrudP_Packet *a)
{
    std::lock_guard<std::mutex> grd(m_packets_mutex);
    uint32_t ack_count_to_process=a->getNumAcks();
    for(size_t i=0; i<ack_count_to_process; i++)
    {
        PacketAck(a->getNextAck()); // endpoint acknowledged those packets
    }
    // clean up acked packets from reliable_packets
    auto first_invalid =
            std::remove_if(reliable_packets.begin(), reliable_packets.end(),
                           [](CrudP_Packet *p) -> bool { return p == nullptr; });
    reliable_packets.erase(first_invalid,reliable_packets.end());

    recv_acks.push_back(a->GetSequenceNumber());
    m_unacked_count = recv_acks.size();
    if(!a->HasSiblings())
    {
        avail_packets.push_back(a);
        return;
    }
    if(!insert_sibling(a))
    {
        delete a;
        return;
    }
    if(allSiblingsAvailable(a->getSibId()))
        avail_packets.push_back(mergeSiblings(a->getSibId()));

}

CrudP_Packet *CrudP_Protocol::mergeSiblings(uint32_t id)
{
    auto store_iter = sibling_map.find(id);
    pPacketStorage &storage(store_iter->second);
    assert(!storage.empty()); // wtf ??
    BitStream *bs=new BitStream(32);
    CrudP_Packet *res= new CrudP_Packet(*storage[0]); //copy packet info from first sibling
    for(CrudP_Packet *pak : storage)
    {
        //Skip duplicate siblings
        //if(i > 0 && storage[i]->getSibPos() == storage[i-1]->getSibPos()) continue;
        assert(pak->getSibId() == id);
        BitStream *pkt_bs = pak->GetStream();
        bs->StoreBitArray(pkt_bs->read_ptr(),pkt_bs->GetReadableBits());
        delete pak;
        //PacketFactory::Delete(storage[i])
    }
    res->SetStream(bs);

    sibling_map.erase(store_iter);
    return res;
}

bool CrudP_Protocol::insert_sibling(CrudP_Packet *pkt)
{
    pPacketStorage &storage=sibling_map[pkt->getSibId()];
    if(storage.size()==0)
        storage.resize(pkt->getNumSibs());

    if(storage[pkt->getSibPos()])
    {
        if(storage[pkt->getSibPos()]->getSibId()!=pkt->getSibId())
        {
            assert(!"m_sibPos is same, but Id differs!");
        }
        return false;
    }
    storage[pkt->getSibPos()] = pkt;
    return true;
}

/**
  \brief this gets next packet in sequence,
  \return Pointer to packet. nullptr if no packets are available or no next packet in sequence is available
    First.  if there are no packets in avail_packets return nullptr
    Second. if first available packet sequence number is the same as the last popped one was, remove this duplicate
    Third.  if first available packet sequence number is the one we want (recv_seq+1) we pop it from storage,
            strip it's shell, and return only a Dbg/Plain BitStream copy of it's payload
*/
CrudP_Packet *CrudP_Protocol::RecvPacket()
{
    CrudP_Packet *pkt=nullptr;

    if(avail_packets.empty())
        return nullptr;
    sort(avail_packets.begin(),avail_packets.end(),&CrudP_Protocol::PacketSeqCompare);
    pkt = avail_packets.front();
    avail_packets.pop_front();
    // duplicate packet removal
    while(pkt->GetSequenceNumber()<=recv_seq)
    {
        delete pkt;
        if(avail_packets.empty())
        return nullptr;
        avail_packets.pop_front();
        pkt = avail_packets.front();
    }
    assert(pkt);
    if(pkt->GetSequenceNumber()!=recv_seq+1) // nope this packet is not a next one in the sequence
        return nullptr;
    if(pkt->getNumSibs()>0)
        recv_seq+=pkt->getNumSibs();
    else
        recv_seq++;
    return pkt;
}

//! this acknowledges that packet with id was successfully received => acknowledged packet is removed from send queue
void CrudP_Protocol::PacketAck(uint32_t id)
{
    for (CrudP_Packet *&pack : reliable_packets )
    {
        if( !pack || pack->GetSequenceNumber() != id)
            continue;
        //todo: update ping times here ( basically calculate time before packet's xmit and now
        if( !retransmit_queue.empty() )
        {
            // check if our packet is already in retransmit_queue, if so, remove it from there.
            auto iter = std::find(retransmit_queue.begin(),retransmit_queue.end(),pack);
            if( iter!=retransmit_queue.end() )
                retransmit_queue.erase(iter);
        }
        delete pack;
        pack = nullptr;
        break;
    }
    // if we get here it means we got ACK for packet we don't have in our
    // reliable_packets
}

vCrudP_Packet packetSplit(CrudP_Packet &src,size_t block_size)
{
    vCrudP_Packet res;
    CrudP_Packet *act;
    uint32_t sib_idx = 0;
    BitStream* bit_stream = src.GetStream();

    while(bit_stream->GetReadableDataSize()>block_size)
    {
        act = new CrudP_Packet;
        act->GetStream()->PutBytes(bit_stream->read_ptr(),block_size);
        act->setSibPos(sib_idx++);
        act->SetReliabilty(src.isReliable());
        bit_stream->read_ptr(int(block_size));
        res.push_back(act);
    }
    if(bit_stream->GetReadableDataSize()>0) // store leftover
    {
        act = new CrudP_Packet;
        act->GetStream()->StoreBitArray(bit_stream->read_ptr(),bit_stream->GetReadableBits());
        act->setSibPos(sib_idx);
        act->SetReliabilty(src.isReliable());
        res.push_back(act);
    }
    return res;

}

//! create a copy of the packet and then wrap it's content in protocol related parts.
CrudP_Packet *CrudP_Protocol::wrapPacket(CrudP_Packet *_p)
{
    int cmd=-1;
    if( 0==_p->getSibId())
    {
        _p->GetStream()->ResetReading();
        cmd = _p->GetPackedBits(1);
        _p->GetStream()->ResetReading();
    }
    BitStream *strm =new BitStream(_p->GetStream()->GetReadableDataSize()+64);
    memset(strm->read_ptr(),0,64);
    strm->Put<uint32_t>(0); // bitlength placeholder
    strm->Put<uint32_t>(0); // checksum placeholder
    strm->StoreBits(1,false);
    strm->StoreBits(32,_p->GetSequenceNumber());
    strm->StorePackedBits(1,_p->getNumSibs()); // sibling count
    if(_p->HasSiblings())
    {
        strm->StorePackedBits(1,_p->getSibPos());
        strm->StoreBits(32,_p->getSibId());
    }
    storeAcks(*strm);
    if(cmd!=1)
        strm->StoreBits(1,_p->getIsCompressed());
    strm->ByteAlign();
    strm->StoreBitArray(_p->GetStream()->read_ptr(),_p->GetStream()->GetReadableBits());
    strm->ResetReading();
    uint32_t *head =  (uint32_t *)strm->read_ptr();
    head[0] = strm->GetReadableBits();

    CrudP_Packet *res = new CrudP_Packet(*_p);
    res->SetStream(strm);
    return res;
}

void CrudP_Protocol::sendRaw(CrudP_Packet *pak,lCrudP_Packet &tgt )
{
    CrudP_Packet *wrapped = wrapPacket(pak);

    wrapped->GetStream()->ByteAlign();
    size_t length =wrapped->GetStream()->GetReadableDataSize();
    size_t fixedlen=((length + 3) & ~7u) + 4;
    while(wrapped->GetStream()->GetReadableDataSize()<fixedlen)
    {
        wrapped->GetStream()->Put<uint8_t>(0);
    }
    uint32_t *head = (uint32_t*)wrapped->GetStream()->read_ptr();
    head[1] = m_codec->Checksum((uint8_t*)&head[2],fixedlen-8); // this is safe because all bitstreams have padding
    m_codec->Encrypt((uint8_t*)&head[1],fixedlen-4);//res->GetReadableDataSize()
    tgt.emplace_back(wrapped);
}

bool CrudP_Protocol::addToSendQueue(CrudP_Packet *pak)
{
    if(send_queue.isFull())
        return false;

    pak->setSeqNo(++send_seq);
    pak->setLastSend(steady_clock::now());
    {
        std::lock_guard<std::mutex> grd(m_packets_mutex);
        send_queue.push_back(pak);
    }
    return true;
}

///
/// \brief add the given packet to the send queue of this link, if large payload - split it into mulitple CRUDP packets
/// \param packet
bool CrudP_Protocol::SendPacket(CrudP_Packet *packet)
{
    if(m_compression_allowed && packet->compressRequested())
    {
        assert(false);
        // compress the bitstream here.
    }
    if(packet->GetStream()->GetReadableDataSize() < max_packet_data_size)
    {
        bool res = addToSendQueue(packet);
        if(!res)
            delete packet;
        return res;
    }
    // this is a large packet, split it, and put parts on send queue
    vCrudP_Packet split_packets = packetSplit(*packet, max_packet_data_size);
    uint32_t      sib_count     = uint32_t(split_packets.size());
    uint32_t      sib_id        = ++sibling_id;
    for (CrudP_Packet *packet_part : split_packets)
    {
        packet_part->setNumSibs(sib_count);
        packet_part->setSibId(sib_id);
        if(!addToSendQueue(packet_part))
            delete packet_part;
    }
    delete packet; // at this point the packet is of no use anymore, since it was split.
    return true;
}

///
/// \brief CrudP_Protocol::isUnresponsiveLink
/// \return true if any packet in the reliable packets array is older than 300ms
///
bool CrudP_Protocol::isUnresponsiveLink()
{

    if(m_last_activity == timepoint())
        return false; // we don't know if we're unresponsive yet.
    auto time_now = steady_clock::now();
    //TODO: make the 15seconds a parameter ?
    long seconds_since_last=duration_cast<seconds>(time_now-m_last_activity).count();
    if(seconds_since_last < 15)
        return false; // client didn't send anything in less than 15 s, give it a bit more time
    for(CrudP_Packet * pkt : reliable_packets)
    {
        if(duration_cast<milliseconds>(time_now - pkt->creationTime()).count() >= 300)
            return true;
    }
    return false;
}

bool CrudP_Protocol::batchSend(lCrudP_Packet &tgt)
{
    if(isUnresponsiveLink())
    {
        qDebug() << "Unresponsive link";
        return false;
    }
    // move some packets from reliable_packets to retransmit_queue
    processRetransmits();
    // first handle retransmit queue
    CrudP_Packet *pak;
    if(!retransmit_queue.empty())
    {
        while ((pak = retransmit_queue.front()) != nullptr)
        {
            //qDebug() << "Retrans packet with id" << pak->GetSequenceNumber();
            sendRaw(pak, tgt);
            retransmit_queue.pop_front();
        }
    }
    if(send_queue.empty())
        return true;
    // than handle the main queue
    while ((pak = send_queue.front()) != nullptr)
    {
        sendRaw(pak, tgt);
        send_queue.pop_front();
        if(pak->isReliable())
        {
            if(!reliable_packets.empty())
                assert(pak->GetSequenceNumber() > reliable_packets.back()->GetSequenceNumber());
            reliable_packets.push_back(pak);
        } else
            delete pak;
    }
    return true;
}

///
/// \brief Finds packets that need to be retransmitted and wraps them in protocol related bits
///
void CrudP_Protocol::processRetransmits()
{
    if(reliable_packets.empty())
        return;
    auto now       = steady_clock::now();
    int  ping_time = 50;

    uint32_t first_packet_id = reliable_packets.front()->GetSequenceNumber();
    for (CrudP_Packet *pkt : reliable_packets)
    {
        if(retransmit_queue.isFull())
            break;
        int resend_period =
                getPacketResendDelay(pkt->retransmitCount(), ping_time, pkt->GetSequenceNumber() < first_packet_id);
        long milliseconds_since_xfer = duration_cast<milliseconds>(now - pkt->lastSend()).count();
        if(milliseconds_since_xfer <= resend_period)
            continue;
        retransmit_queue.push_back(pkt);
        pkt->setLastSend(steady_clock::now());
        // todo: record packet send time in protocol instance ?
        pkt->incRetransmits();
    }
}

//! @}
