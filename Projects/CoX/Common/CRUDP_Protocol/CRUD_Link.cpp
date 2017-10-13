#include "CRUD_Link.h"

#include "CRUD_Events.h"

#include <QDebug>
#include <set>

// CRUD link receives messages from ServerEndpoint,
// these are basically CRUDP_Packets preprocessed by CRUDP_Protocol

// ServerEndpoint gets new input -> Bytes -> CRUDP_Protocol -> pushq(PacketEvent)
// ACE_Reactor knows to wake CRUDLink up, whenever there are new events
std::set<CRUDLink *> all_links;
CRUDLink::CRUDLink() :  m_notifier(0, 0, ACE_Event_Handler::WRITE_MASK)
{
    m_notifier.event_handler(this);
    m_protocol.setCodec(new PacketCodecNull);
    all_links.insert(this);
}

CRUDLink::~CRUDLink() {
    all_links.erase(this);
}

///
/// \brief CRUDLink::event_for_packet - convert incoming packet into higher level events and push them to our target()
/// \param pak_ev - received packet
///
void CRUDLink::event_for_packet(PacketEvent * pak_ev)
{
    CrudP_Packet *pak=pak_ev->m_pkt;
    // switch this to while, maybe many events are coming from single packet ?
    CRUDLink_Event *res = factory().EventFromStream(*pak->GetStream());
    if(!res)
    {
        ACE_ERROR((LM_ERROR, ACE_TEXT ("%p EventFromStream returned NULL\n")));
        return;
    }
    res->serializefrom(*pak->GetStream());
    res->src(this);
    res->m_seq_number = pak->GetSequenceNumber();
    target()->putq(res);
    if(pak->GetStream()->GetReadableBits()>1) {
        qDebug() << res->info() << "left" << pak->GetStream()->GetReadableBits() <<"bits";
    }
}
///
/// \brief CRUDLink::packets_for_event - convert event to 1-n packets and push them to our net_layer()
/// \param ev - an event that we've received from our downstream.
///
void CRUDLink::packets_for_event(SEGSEvent *ev)
{
    lCrudP_Packet packets;
    CRUDLink_Event *c_ev =static_cast<CRUDLink_Event *>(ev);
    CrudP_Packet *res   = new CrudP_Packet;
    ACE_ASSERT(res);
    ACE_DEBUG((LM_WARNING,ACE_TEXT("(%P|%t) packets_for_event %s\n"),ev->info()));

    c_ev->serializeto(*res->GetStream()); // serialize packet into res packet
    // create one or more properly formated CrudP_Packets in the protocol object
    qDebug() << "Adding packets for"<<c_ev->info();
    m_protocol.SendPacket(res);
    size_t cnt=m_protocol.GetUnsentPackets(packets);
    // wrap all packets as PacketEvents and put them on link queue
    while(cnt--)
    {
        net_layer()->putq(new PacketEvent(this,*packets.begin(),peer_addr()));
        packets.pop_front();
    }
    connection_sent_packet(); // data was sent, update
}
//! Called when we start to service a new connection, here we tell reactor to wake us
//! when queue() is not empty.
int CRUDLink::open (void *p)
{
    if (EventProcessor::open (p) == -1)
        return -1;
    m_notifier.reactor(reactor());  // notify reactor with write event,
    msg_queue()->notification_strategy (&m_notifier);   // whenever there is a new event on msg_queue()
    connection_update(); // record first activity time.
    connection_sent_packet();
    return 0;
}
int CRUDLink::handle_output( ACE_HANDLE )
{
    SEGSEvent *ev;
    ACE_Time_Value nowait (ACE_OS::gettimeofday ());
    while (-1 != getq(ev, &nowait))
    {
        switch(ev->type())
        {
            case SEGS_EventTypes::evFinish:
                ev->release();
                return -1;
            case SEGS_EventTypes::evConnect:
                m_peer_addr=static_cast<ConnectEvent *>(ev)->src_addr;
                connection_update();
                break;
            case SEGS_EventTypes::evDisconnect:
                putq(new SEGSEvent(SEGS_EventTypes::evFinish)); // close the link
                break;
            case CRUD_EventTypes::evPacket: // CRUDP_Protocol has posted a pre-parsed packet to us
                event_for_packet(static_cast<PacketEvent *>(ev));
                connection_update(); // we've received some bytes -> connection update
                break;
            default:
                packets_for_event(static_cast<CRUDLink_Event *>(ev));
                break;
        }
        ev->release();
        //TODO: check how getq works when nowait is before now() ??
        //nowait = ACE_OS::gettimeofday();
        //TODO: consider breaking out of this loop after processing N messages ?
    }
    // Now if our message queue is empty, we will wait unitl m_notifier awakens us.
    if (msg_queue()->is_empty ()) // we don't want to be woken up
        reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    else // unless there is something to send still
        reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    return 0;
}
///
/// \brief CRUDLink::received_block - convert incoming bytes to PacketEvent's
/// \param bytes - raw data to convert into packets.
///
void CRUDLink::received_block( BitStream &bytes )
{
    size_t recv_count=0; // count of proper packets
    // Fill the protocol with 'raw' bit stream
    m_protocol.ReceivedBlock(bytes);
    // now try to get actual packets
    CrudP_Packet *pkt = m_protocol.RecvPacket(false);
    while(pkt)
    {
        putq(new PacketEvent(net_layer(),pkt,peer_addr()));
        ++recv_count;
        pkt=m_protocol.RecvPacket(false);
    }
    if(recv_count>0)
        connection_update(); //update the last time we've seen packets on this link
}

