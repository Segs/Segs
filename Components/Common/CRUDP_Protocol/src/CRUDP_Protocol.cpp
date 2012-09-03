/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

//#include "GameProtocol.h"
#include <cassert>
#include "PacketCodec.h"
#include "CRUDP_Protocol.h"
#include <zlib.h>
//#include <ace/OS_NS
//#define LOG_PACKETS 1

/*
 Cryptic Reliable UDP
 CrudP
 Incoming
  BitStream Block ->
   Parse Header [ Checksum + Decode ]
   Remove from send queue packets acknowledged by this header
   if seq_no < min_recv_seq, reject packet
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
    m_codec=NULL;
}
CrudP_Protocol::CrudP_Protocol() :	send_seq(0),recv_seq(0),m_codec(NULL)
{
}
void CrudP_Protocol::clearQueues(bool recv_queue,bool send_queue)
{
    //	seen_seq.clear();
    if(recv_queue)
    {
        for_each(avail_packets.begin(),avail_packets.end(),PacketDestroyer);
        for_each(sibling_map.begin(),sibling_map.end(),PacketSibDestroyer);
        sibling_map.clear();
        avail_packets.clear();
    }
    if(send_queue)
    {
        ACE_Guard<ACE_Thread_Mutex> grd(m_packets_mutex);
        for_each(unsent_packets.begin(),unsent_packets.end(),PacketDestroyer);
    }
}
void CrudP_Protocol::ReceivedBlock(BitStream &src)
{
    CrudP_Packet *res=NULL;
    if(src.GetReadableDataSize()<12)
        return;
    uint32_t bitlength,checksum,sibcount;

    src.Get(bitlength);
    assert(src.GetReadableBits()>=bitlength-32);
    if(!m_codec)
        return; // later on we might allow codec-less operation ?
    m_codec->Decrypt((uint8_t *)src.read_ptr(),src.GetReadableDataSize());

    src.Get(checksum);

    uint32_t realcsum  = PacketCodecNull::Checksum((uint8_t*)src.read_ptr(),src.GetReadableDataSize());
    if(realcsum!=checksum)
    {
        return;
    }
    res	= new CrudP_Packet; //PacketFactory::newDataPacket;
    res->m_checksum = realcsum;
    res->SetHasDebugInfo((bool)src.uGetBits(1));
    res->setSeqNo(src.uGetBits(32));
    sibcount = src.GetPackedBits(1);
    res->setNumSibs(sibcount);
    if(sibcount)
    {
        res->setSibPos(src.GetPackedBits(1));
        res->setSibId(src.uGetBits(32));
    }
    parseAcks(src,res);
    res->SetIsCompressed(src.GetBits(1));
    //src.GetBits(1); //unknown flag
    //src.Get_2_10_24_32(); // ordered_id
    src.ByteAlign();
    // how much data did we actually read
    size_t bits_left=(bitlength-src.GetReadPos())+1;
    //	src.PopFront(src.GetDataSize()-src.GetReadableDataSize()); //this shifts the stream so it now begins at last read position i.e. Real packet payload
    res->StoreBitArray(src.read_ptr(),bits_left);
    PushRecvPacket(res);
}
void CrudP_Protocol::parseAcks(BitStream &src,CrudP_Packet *tgt)
{
    uint32_t numUniqueAcks = src.GetPackedBits(1);
    if(numUniqueAcks  == 0) return;

    uint32_t firstAck = src.GetBits(32);
    tgt->addAck(firstAck);
    for(uint32_t i = 1; i < numUniqueAcks; i++)
    {
        //	The first sequence number is sent in it's entirety.  Every subsequent
        //	number is sent as a delta between it, and it's predecessor.  This is
        //	a clever way of compressing the ack table, because if you have the
        //	first ack in the sequence, all you need is the delta to the next one
        //	to calculate it, and the Packed Bits format achieves much higher
        //	compression ratios with smaller values
        firstAck += src.GetPackedBits(1) + 1;
        tgt->addAck(firstAck);
    }
}
void CrudP_Protocol::storeAcks(BitStream &bs)
{
    //TODO: sort + binary search for id
    ACE_Guard<ACE_Thread_Mutex> grd(m_packets_mutex);
    if(recv_acks.size()==0)
    {
        bs.StorePackedBits(1,0);
        return;
    }
    recv_acks.sort();
    recv_acks.unique();
    std::list<uint32_t>::iterator iter = recv_acks.begin();
    uint32_t last_ack = 0;
    uint32_t num_acks = (uint32_t)((recv_acks.size()>16) ? 16:recv_acks.size());
    bs.StorePackedBits(1,num_acks);

    last_ack = *iter;
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
}
bool CrudP_Protocol::allSiblingsAvailable(int sibling_group_id)
{
    pPacketStorage &storage = sibling_map[sibling_group_id];
    size_t avail=0;
    for(size_t i=0; i<storage.size(); i++)
    {
        avail+= (storage[i]!=NULL);
    }
    return avail==storage.size();
}
void CrudP_Protocol::PushRecvPacket(CrudP_Packet *a)
{
    ACE_Guard<ACE_Thread_Mutex> grd(m_packets_mutex);
    for(size_t i=0; i<a->getNumAcks(); i++)
    {
        PacketAck(a->getNextAck()); // endpoint acknowledged those packets
    }
    recv_acks.push_back(a->GetSequenceNumber());
    if(!a->HasSiblings())
    {
        avail_packets.push_back(a);
    }
    else
    {
        if(!insert_sibling(a))
        {
            delete a;
            return;
        }
        if(allSiblingsAvailable(a->getSibId()))
        {
            avail_packets.push_back(mergeSiblings(a->getSibId()));
        }
    }
}
CrudP_Packet *CrudP_Protocol::mergeSiblings(uint32_t id)
{
    pPacketStorage &storage=sibling_map[id];
    assert(storage.size()>=1); // wtf ??
    BitStream *pkt_bs,*bs=new BitStream(32);
    CrudP_Packet *res= new CrudP_Packet(*storage[0]); //copy packet info from first sibling
    for(uint32_t i = 0; i < storage.size(); i++)
    {
        //	Skip duplicate siblings
        //if(i > 0 && storage[i]->getSibPos() == storage[i-1]->getSibPos()) continue;
        assert(storage[i]->getSibId() == id);
        pkt_bs = storage[i]->GetStream();
        bs->PutBytes(pkt_bs->read_ptr(),pkt_bs->GetReadableDataSize());
        delete storage[i];
        //PacketFactory::Delete(storage[i])
    }
    res->SetStream(bs);

    sibling_map.erase(sibling_map.find(id));
    return res;
}
bool CrudP_Protocol::insert_sibling(CrudP_Packet *pkt)
{
    pPacketStorage &storage=sibling_map[pkt->getSibId()];
    if(storage.size()==0)
    {
        storage.resize(pkt->getNumSibs());
    }
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
  \arg disregard_seq if it's set returned pack will be next available, not the next in sequence
  \return Pointer to packet. NUKK if no packets are available or no next packet in sequence is available
    First.  if there are no packets in avail_packets return NULL
    Second. if first available packet sequence number is the same as the last popped one was remove this duplicate
    Third.  if first available packet sequence number is the one we want (recv_seq+1) we pop it from storage,
            strip it's shell, and return only a BitStreamDbg,BitStreamPlain copy of it's payload
*/
CrudP_Packet *CrudP_Protocol::RecvPacket(bool disregard_seq)
{
    CrudP_Packet *pkt=NULL;

    if(0==avail_packets.size())
        return NULL;
    sort(avail_packets.begin(),avail_packets.end(),&CrudP_Protocol::PacketSeqCompare);
    ipPacketStorage iter = avail_packets.begin();
    // duplicate/old_packet removal
    while(iter!=avail_packets.end())
    {
        pkt = *iter;
        if(pkt->GetSequenceNumber()>recv_seq)
            break;
        iter = avail_packets.erase(iter);//remove a duplicate
        //PacketFactory::destroy(a);
    }
    if(iter==avail_packets.end())
        return NULL;
    assert(pkt);
    if(disregard_seq)
        return pkt;
    if(pkt->GetSequenceNumber()!=recv_seq+1) // nope this packet is not a next one in the sequence
        return NULL;
    if(pkt->getNumSibs()>0)
        recv_seq+=pkt->getNumSibs();
    else
        recv_seq++;
    return pkt;
}
void CrudP_Protocol::PacketDestroyer(CrudP_Packet *a)
{
    delete a;
}
void CrudP_Protocol::PacketSibDestroyer(const std::pair<int,pPacketStorage> &a)
{
    for_each(a.second.begin(),a.second.end(),&CrudP_Protocol::PacketDestroyer);
}
//! this acknowledges that packet with id was successfully received => acknowledged packet is removed from send queue
void CrudP_Protocol::PacketAck(uint32_t id)
{
    //TODO: sort + binary search for id
    ipPacketStorage iter = unsent_packets.begin();
    while(iter!=unsent_packets.end())
    {
        if((*iter)->GetSequenceNumber()==id)
            iter=unsent_packets.erase(iter);
        else
            iter++;
    }
}
vCrudP_Packet packetSplit(CrudP_Packet &src,size_t block_size)
{

    vCrudP_Packet res;
    CrudP_Packet*act;
    int i=0;
    while(src.GetStream()->GetReadableDataSize()>block_size)
    {
        act = new CrudP_Packet;
        act->GetStream()->PutBytes(src.GetStream()->read_ptr(),block_size);
        act->setSibPos(i++);
        src.GetStream()->read_ptr((int)block_size);
        res.push_back(act);
    }
    if(src.GetStream()->GetReadableDataSize()>0) // store leftover
    {
        act = new CrudP_Packet;
        //act->GetStream()->PutBytes(src.GetStream()->read_ptr(),src.GetStream()->GetReadableDataSize());
        act->GetStream()->StoreBitArray((uint8_t *)src.GetStream()->read_ptr(),(uint32_t)src.GetStream()->GetReadableBits());
        act->setSibPos(i);
        res.push_back(act);
    }
    return res;

}
uint8_t * compressStream(BitStream &stream)
{
    uLongf comp_length =(uLongf )(12+1.1*stream.GetReadableDataSize());
    uint8_t *dest= new uint8_t[comp_length];
    compress(dest,&comp_length,stream.read_ptr(),stream.GetReadableDataSize());
    return dest;
}
void CrudP_Protocol::SendPacket(CrudP_Packet *p)
{

    // p is just a thin wrapper around the payload
    // Todo: Implement packet splits
    if(p->GetStream()->GetReadableDataSize()>1200)
    {
        static int sib_id=0;
        size_t block_size;
        vCrudP_Packet split_packets = packetSplit(*p,1200);
        CrudP_Packet *pkt;
        ++sib_id;
        for(ivCrudP_Packet iter=split_packets.begin(); iter!=split_packets.end(); iter++)
        {
            pkt = *iter;
            pkt->setNumSibs((uint32_t)split_packets.size());
            pkt->setSibId(sib_id);
            pkt->m_checksum	= 0;
            pkt->setSeqNo(++send_seq);
            block_size = pkt->GetStream()->GetReadableDataSize();
            BitStream *res =new BitStream((uint32_t)(block_size+64));
            res->Put((uint32_t)0); // readable bits holder
            res->Put((uint32_t)0); // checksum placeholder
            res->StoreBits(1,pkt->HasDebugInfo());
            res->StoreBits(32,pkt->GetSequenceNumber());
            res->StorePackedBits(1,pkt->getNumSibs()); // sibling count
            if(pkt->getNumSibs())
            {
                res->StorePackedBits(1,pkt->getSibPos());
                res->StoreBits(32,pkt->getSibId());
            }
            storeAcks(*res);
            res->StoreBits(1,(int)p->getIsCompressed());
            res->StoreBits(1,0); // HasOrderedId
            if(false)
            {
                res->StoreBits_4_10_24_32(0); // instead of  zero we have to put ordered_id here, whatever that is
            }
            res->ByteAlign();
            res->StoreBitArray((uint8_t*)pkt->GetStream()->read_ptr(),(uint32_t)pkt->GetStream()->GetReadableBits());
            res->ResetReading();
            uint32_t *head =  (uint32_t *)res->read_ptr();
            head[0] = (uint32_t)res->GetReadableBits();
            res->ByteAlign();
            size_t length =res->GetReadableDataSize();
            size_t fixedlen=((length + 3) & ~7) + 4;
            while(res->GetReadableDataSize()<fixedlen)
            {
                res->Put((uint8_t)0);
            }
            head[1] = m_codec->Checksum((uint8_t*)&head[2],fixedlen-8); // this is safe because all bitstreams have padding
            m_codec->Encrypt((uint8_t*)&head[1],fixedlen-4);//res->GetReadableDataSize()
            delete pkt->GetStream();
            pkt->SetStream(res);
            ACE_Guard<ACE_Thread_Mutex> grd(m_packets_mutex);
            unsent_packets.push_back(pkt);
        }
    }
    else
    {
        p->setNumSibs(0);
        p->setSibId(0);
        p->m_checksum	= 0;
        p->setSeqNo(++send_seq);
        p->setSibPos(0);

        BitStream *res =new BitStream(p->GetStream()->GetReadableDataSize()+64);
        memset(res->read_ptr(),0,64);
        res->Put((uint32_t)p->GetStream()->GetReadableBits());
        res->Put(p->m_checksum);

        res->StoreBits(1,p->HasDebugInfo());
        res->StoreBits(32,p->GetSequenceNumber());
        res->StorePackedBits(1,p->getNumSibs()); // sibling count
        if(p->getNumSibs())
        {
            res->StorePackedBits(1,p->getSibPos());
            res->StoreBits(32,p->getSibId());
        }
        storeAcks(*res);
        res->StoreBits(1,(int)p->getIsCompressed());
        res->StoreBits(1,0); // HasOrderedId
        if(false)
        {
            res->StoreBits_4_10_24_32(0); // instead of  zero we have to put ordered_id here, whatever that is
        }
        res->ByteAlign();
        res->StoreBitArray((uint8_t*)p->GetStream()->read_ptr(),p->GetStream()->GetReadableBits());
        res->ResetReading();
        uint32_t *head =  (uint32_t *)res->read_ptr();
        head[0] = uint32_t(res->GetReadableBits());
        res->ByteAlign();
        size_t length =res->GetReadableDataSize();
        size_t fixedlen=((length + 3) & ~7) + 4;
        while(res->GetReadableDataSize()<fixedlen)
        {
            res->Put((uint8_t)0);
        }
        head[1] = m_codec->Checksum((uint8_t*)&head[2],fixedlen-8); // this is safe because all bitstreams have padding
        m_codec->Encrypt((uint8_t*)&head[1],fixedlen-4);//res->GetReadableDataSize()
        delete p->GetStream();
        p->SetStream(res);
        ACE_Guard<ACE_Thread_Mutex> grd(m_packets_mutex);
        unsent_packets.push_back(p);
    }
}
//! this gets all currently unacknowledged packets
size_t CrudP_Protocol::GetUnsentPackets(std::list<CrudP_Packet *> &res)
{
    ACE_Guard<ACE_Thread_Mutex> grd(m_packets_mutex);
    res.assign(unsent_packets.begin(),unsent_packets.end());
    return unsent_packets.size();
}
