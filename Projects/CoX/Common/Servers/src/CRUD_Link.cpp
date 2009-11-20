#include "CRUD_Link.h"
#include "PacketCodec.h"

template<class EVENT_FACTORY>
EventProcessor *CRUDLink<EVENT_FACTORY>::g_target=0;
template<class EVENT_FACTORY>
EventProcessor *CRUDLink<EVENT_FACTORY>::g_link_target=0;

// CRUD link receives messages from ServerEndpoint, 
// these are basically CRUDP_Packets preprocessed by CRUDP_Protocol

// ServerEndpoint gets new input: Bytes -> CRUDP_Protocol -> pushq(PacketEvent)
// ACE_Reactor knows to wake CRUDLink up, as there are new events

template<class EVENT_FACTORY>
void CRUDLink<EVENT_FACTORY>::event_for_packet(SEGSEvent *ev)
{
    PacketEvent * pak_ev=static_cast<PacketEvent *>(ev);
    CrudP_Packet *pak=pak_ev->m_pkt;
    // switch this to while, maybe many events are coming from single packet ?
    CRUD_Event *res = EVENT_FACTORY::EventFromStream(*pak->GetStream());
    if(res)
        res->serializefrom(*pak->GetStream());
    res->src(this);
    g_target->putq(res);
}
template<class EVENT_FACTORY>
void CRUDLink<EVENT_FACTORY>::packets_for_event(SEGSEvent *ev)
{
    CRUD_Event *c_ev =static_cast<CRUD_Event *>(ev);
    CrudP_Packet *res=new CrudP_Packet;
    BitStream *tgt = res->GetStream();
    c_ev->serializeto(*tgt);
    get_proto()->SendPacket(res); // this will create one or more properly formated CrudP_Packets in the protocol object
    lCrudP_Packet packets;
    size_t cnt=get_proto()->GetUnsentPackets(packets);
    while(cnt--)
    {
        g_link_target->putq(new PacketEvent(this,*packets.begin(),peer_addr()));
        packets.pop_front();
    }
}
template<class EVENT_FACTORY>
int CRUDLink<EVENT_FACTORY>::open (void *p)
{
	if (EventProcessor::open (p) == -1)
		return -1;
	m_notifier.reactor(reactor());						// notify reactor with write event, 
	msg_queue()->notification_strategy (&m_notifier);	// whenever there is a new event on msg_queue()
	return 0;
}

template<class EVENT_FACTORY>
int CRUDLink<EVENT_FACTORY>::handle_output( ACE_HANDLE )
{
	SEGSEvent *ev;
	ACE_Time_Value nowait (ACE_OS::gettimeofday ());
	while (-1 != getq(ev, &nowait))
	{
        switch(ev->type())
        {
        case SEGSEvent::evFinish:
            ev->release();
            return -1;
        case evPacket: // CRUDP_Protocol has posted a pre-parsed packet to us
            {
                PacketEvent *pkt_ev = static_cast<PacketEvent *>(ev);
                event_for_packet(pkt_ev);
            }
        default:
            {
                CRUD_Event *c_ev = static_cast<CRUD_Event *>(ev);
                packets_for_event(c_ev);
            }
        }
		ev->release();
	}
	if (msg_queue()->is_empty ()) // we don't want to be woken up 
		reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
	else // unless there is something to send still
		reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
	return 0;
}

template<class EVENT_FACTORY>
void CRUDLink<EVENT_FACTORY>::dispatch( SEGSEvent *ev )
{
    ACE_ASSERT(!"Should not be called");
}

template<class EVENT_FACTORY>
void CRUDLink<EVENT_FACTORY>::received_block( BitStream &bytes )
{
    CrudP_Packet *pkt;
    get_proto()->ReceivedBlock(bytes);
    // now convert all available packets into events
    while(NULL!=(pkt=get_proto()->RecvPacket(false)))
    {
        putq(new PacketEvent(g_link_target,pkt,peer_addr()));
    }
}
