/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

#pragma once
#include <deque>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <ace/Message_Queue.h>
#include <ace/INET_Addr.h>
#include "hashmap_selector.h"
#include "Base.h"
#include "CRUDP_Packet.h"
using namespace std;
class PacketCodecNull;
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
class CrudP_Protocol
{
private:
	typedef deque<CrudP_Packet *> pPacketStorage;
	typedef pPacketStorage::iterator ipPacketStorage ;
	typedef hash_map<int,pPacketStorage> hmSibStorage;
	//	set<u32> seen_seq;
		u32 send_seq;
		u32 recv_seq;

		PacketCodecNull *m_codec;
		pPacketStorage avail_packets;
		pPacketStorage unsent_packets;
		list<u32> recv_acks; // each successful receive will store it's ack here
	// we need to lookup mPacketGroup quickly, and insert ordered packets into mPacketGroup
		hmSibStorage sibling_map;
		ACE_Thread_Mutex m_packets_mutex;

		CrudP_Packet *		mergeSiblings(int id);
		bool				insert_sibling(CrudP_Packet *pkt);
static inline bool			PacketSeqCompare(const CrudP_Packet *a,const CrudP_Packet *b);
static inline bool			PacketSibCompare(const CrudP_Packet *a,const CrudP_Packet *b);
        bool				allSiblingsAvailable(int );
public:
                            CrudP_Protocol();
                            ~CrudP_Protocol();
        void				setCodec(PacketCodecNull *codec){m_codec= codec;};
        PacketCodecNull *	getCodec() const {return m_codec;};

		size_t				UnsentPackets()    const {return unsent_packets.size();}
		size_t				AvailablePackets() const {return avail_packets.size();}

		size_t				GetUnsentPackets(list<CrudP_Packet *> &); // this gets all as of now unacknowledged packets
		void				ReceivedBlock(BitStream &bs); // bytes received, will create some packets in avail_packets

        void				SendPacket(CrudP_Packet *p); // this might split packet 'p' into a few packets
        CrudP_Packet *		RecvPacket(bool disregard_seq); // this get's next packet in sequence, if disregard_seq is set returned pack will be next available, an
protected:
		void				parseAcks(BitStream &src,CrudP_Packet *tgt);
		void				storeAcks(BitStream &bs);
		void				PushRecvPacket(CrudP_Packet *a); // this will try to join packet 'a' with it's siblings
		void				PacketAck(u32); // this acknowledges that packet with id was successfully received => acknowledged packet is removed from send queue
		void				clearQueues(bool recv,bool send); // clears out the recv/send queues

static	void				PacketDestroyer(CrudP_Packet *a);
static	void				PacketSibDestroyer(const pair<int,pPacketStorage> &a);
};
