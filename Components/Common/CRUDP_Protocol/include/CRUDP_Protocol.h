/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: CRUDP_Protocol.h 253 2006-08-31 22:00:14Z malign $
 */

#pragma once
#include <deque>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <ace/Message_Queue.h>
#include <ace/INET_Addr.h>
#ifndef WIN32
#include <ext/hash_map>
using namespace __gnu_cxx;
#else
#include <hash_map>
using namespace stdext;
#endif
#include "Base.h"
#include "CRUDP_Packet.h"
using namespace std;
class PacketCodecNull;
class Client;
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
class IGameProtocol;
class CrudP_Protocol 
{
private:
	typedef deque<CrudP_Packet *> pPacketStorage;
	typedef pPacketStorage::iterator ipPacketStorage ;
	typedef hash_map<int,pPacketStorage> hmSibStorage;
	//	set<u32> seen_seq;
		Client *		m_client;
		u32 send_seq;
		u32 recv_seq;

		PacketCodecNull *m_codec;
		pPacketStorage avail_packets;
		pPacketStorage unsent_packets;
		IGameProtocol *m_layer;
		list<u32> recv_acks; // each sucessful receive will store it's acl here
	// we need to lookup mPacketGroup quickly, and insert ordered packets into mPacketGroup 
		hmSibStorage sibling_map;
		ACE_Thread_Mutex m_packets_mutex;
		ACE_Message_Queue_Base *m_queue;

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

		size_t				GetUnsentPackets(list<CrudP_Packet *> &); // this get's next packet in send sequence
		void				ReceivedBlock(BitStream &bs);

		void				setGameLayer(IGameProtocol *p){m_layer=p;};

		void				SendPacket(CrudP_Packet *p); // this might split packet 'a' into a few packets
		void				setMessageQueue(ACE_Message_Queue_Base *q){m_queue=q;};
		Client *			getClient(){return m_client;};
		void				setClient(Client *ptr){m_client = ptr;};
protected:
		void				parseAcks(BitStream &src,CrudP_Packet *tgt);
		void				storeAcks(BitStream &bs);
		void				PushRecvPacket(CrudP_Packet *a); // this will try to join packet 'a' with it's siblings
		void				PacketAck(u32); // this acknowledges that packet with id was successfully sent and received => acknowledged packet is removed from send queue
		void				clearQueues(bool recv,bool send); // clears out the recv/send queues
		CrudP_Packet *		RecvPacket(bool disregard_seq); // this get's next packet in sequence, if disregard_seq is set returned pack will be next available, an

static	void				PacketDestroyer(CrudP_Packet *a);
static	void				PacketSibDestroyer(const pair<int,pPacketStorage> &a);
}; 
