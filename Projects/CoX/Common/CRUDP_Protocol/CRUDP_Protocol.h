/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include <unordered_map>
#include <deque>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <ace/Message_Queue.h>
#include <ace/INET_Addr.h>
#include "CRUDP_Packet.h"
class PacketCodecNull;
class CrudP_Protocol
{
private:
        typedef std::deque<CrudP_Packet *> pPacketStorage;
        typedef pPacketStorage::iterator ipPacketStorage ;
        typedef std::unordered_map<int,pPacketStorage> hmSibStorage;

        uint32_t            send_seq;
        uint32_t            recv_seq;

        PacketCodecNull *   m_codec;
        pPacketStorage      avail_packets;
        pPacketStorage      unsent_packets;
        std::list<uint32_t> recv_acks; // each successful receive will store it's ack here
        hmSibStorage        sibling_map; // we need to lookup mPacketGroup quickly, and insert ordered packets into mPacketGroup
        ACE_Thread_Mutex    m_packets_mutex;

        CrudP_Packet *      mergeSiblings(uint32_t id);
        bool                insert_sibling(CrudP_Packet *pkt);
static  bool                PacketSeqCompare(const CrudP_Packet *a,const CrudP_Packet *b);
static  bool                PacketSibCompare(const CrudP_Packet *a,const CrudP_Packet *b);
        bool                allSiblingsAvailable(int );
public:
                            CrudP_Protocol();
                            ~CrudP_Protocol();
        void                setCodec(PacketCodecNull *codec){m_codec= codec;}
        PacketCodecNull *   getCodec() const {return m_codec;}

        size_t              UnsentPackets()    const {return unsent_packets.size();}
        size_t              AvailablePackets() const {return avail_packets.size();}

        size_t              GetUnsentPackets(std::list<CrudP_Packet *> &);
        void                ReceivedBlock(BitStream &bs); // bytes received, will create some packets in avail_packets

        void                SendPacket(CrudP_Packet *p); // this might split packet 'p' into a few packets
        CrudP_Packet *      RecvPacket(bool disregard_seq);
protected:
        void                sendLargePacket(CrudP_Packet *p);
        void                sendSmallPacket(CrudP_Packet *p);
        void                parseAcks(BitStream &src,CrudP_Packet *tgt);
        void                storeAcks(BitStream &bs);
        void                PushRecvPacket(CrudP_Packet *a); // this will try to join packet 'a' with it's siblings
        void                PacketAck(uint32_t);
        void                clearQueues(bool recv,bool send); // clears out the recv/send queues

static	void                PacketDestroyer(CrudP_Packet *a);
static	void                PacketSibDestroyer(const std::pair<int,pPacketStorage> &a);
};
