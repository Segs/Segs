/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CRUDP_Packet.h"

#include <unordered_map>
#include <deque>
#include <list>
#include <chrono>
#include <mutex>
#include <atomic>

template <size_t size>
struct FixedSizePacketQueue
{
    using iterator = std::deque<CrudP_Packet *>::iterator;
    std::deque<CrudP_Packet *> m_storage;
    bool                       isFull() const { return m_storage.size() >= size; }
    void                       push_back(CrudP_Packet *v)
    {
        if(!isFull())
            m_storage.push_back(v);
    }
    void          pop_front() { m_storage.pop_front(); }
    CrudP_Packet *front() { return m_storage.empty() ? nullptr : m_storage.front(); }
    bool          empty() const { return m_storage.empty(); }
    iterator      begin() { return m_storage.begin(); }
    iterator      end() { return m_storage.end(); }
    iterator      erase(iterator v) { return m_storage.erase(v); }
    void          clear() { m_storage.clear(); }
};
class PacketCodecNull;
class CrudP_Protocol
{
private:
        using timepoint = std::chrono::steady_clock::time_point;
        using pPacketStorage = std::deque<CrudP_Packet *>;
        using hmSibStorage = std::unordered_map<uint32_t,pPacketStorage>;

        friend void PacketSibDestroyer(const std::pair<int, pPacketStorage> &a);
        static constexpr const int max_packet_data_size = 1272;

        uint32_t            send_seq=0;
        uint32_t            recv_seq=0;
        uint32_t            sibling_id=0;

        PacketCodecNull *   m_codec = nullptr;
        pPacketStorage      avail_packets;
        pPacketStorage      unacked_packets;
        pPacketStorage      reliable_packets;
        FixedSizePacketQueue<16384> send_queue;
        FixedSizePacketQueue<16384> retransmit_queue;
        std::list<uint32_t> recv_acks; // each successful receive will store it's ack here
        std::atomic<size_t> m_unacked_count {0};
        hmSibStorage        sibling_map; // we need to lookup mPacketGroup quickly, and insert ordered packets into mPacketGroup
        std::mutex          m_packets_mutex;
        bool                m_compression_allowed=false;
        timepoint           m_last_activity;

        CrudP_Packet *      mergeSiblings(uint32_t id);
        bool                insert_sibling(CrudP_Packet *pkt);
static  bool                PacketSeqCompare(const CrudP_Packet *a,const CrudP_Packet *b);
static  bool                PacketSibCompare(const CrudP_Packet *a,const CrudP_Packet *b);
        bool                allSiblingsAvailable(uint32_t sibid);
        bool                addToSendQueue(CrudP_Packet *pak);
public:
                            ~CrudP_Protocol();
        void                setCodec(PacketCodecNull *codec){m_codec= codec;}
        PacketCodecNull *   getCodec() const {return m_codec;}

        size_t              AvailablePackets() const {return avail_packets.size();}
        size_t              UnackedPacketCount() const { return m_unacked_count; }
        size_t              GetUnsentPackets(std::list<CrudP_Packet *> &);
        void                ReceivedBlock(BitStream &bs); // bytes received, will create some packets in avail_packets

        bool                SendPacket(CrudP_Packet *p); // this might split packet 'p' into a few packets
        CrudP_Packet *      RecvPacket();
        bool                batchSend(lCrudP_Packet &tgt);
        bool                isUnresponsiveLink();
protected:
        void                sendRaw(CrudP_Packet *pak,lCrudP_Packet &tgt);
        void                processRetransmits();
        CrudP_Packet *      wrapPacket(CrudP_Packet *p);
        void                parseAcks(BitStream &src,CrudP_Packet *tgt);
        void                storeAcks(BitStream &bs);
        void                PushRecvPacket(CrudP_Packet *a); // this will try to join packet 'a' with it's siblings
        void                PacketAck(uint32_t);
        void                clearQueues(bool recv,bool clear_send_queue); // clears out the recv/send queues
};
