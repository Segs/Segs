/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup AuthProtocol Projects/CoX/Common/AuthProtocol
 * @{
 */

#include "AuthLink.h"

#include "AuthProtocol/AuthOpcodes.h"
#include "AuthProtocol/AuthEventFactory.h"

AuthLink::AuthLink() :
    m_received_bytes_storage(0x1000,0,40),
    m_unsent_bytes_storage(0x200,0,40),
    m_notifier(nullptr, nullptr, ACE_Event_Handler::WRITE_MASK)
{
// This constructor exists only to make the default ACE implementation of make_svc_handler happy
// it should never be called by our code, since we override make_svc_handler.
    assert(false);
}

AuthLink::AuthLink(EventProcessor *target, AuthLinkType link_type) :
    m_target(target),
    m_received_bytes_storage(0x1000,0,40),
    m_unsent_bytes_storage(0x200,0,40),
    m_notifier(nullptr, nullptr, ACE_Event_Handler::WRITE_MASK),
    m_protocol_version(-1),
    m_state(INITIAL),
    m_direction(link_type)
{
    m_notifier.event_handler(this); // notify 'this' object on WRITE events
    m_buffer_mutex = new ACE_Thread_Mutex;
    assert(m_target);
}
AuthLink::~AuthLink( )
{
    delete m_buffer_mutex;
}
void AuthLink::init_crypto(int vers,uint32_t seed)
{
    ACE_Guard<ACE_Thread_Mutex> guard_buffer(*m_buffer_mutex); // Prevent modifying the buffer while the key is beeing changed
    set_protocol_version(vers);
    m_codec.SetXorKey(seed);
}
/**
  \brief Convert opcode byte to corresponding packet type
  \arg opcode packet opcode byte
  \arg direction if this is false then the packet is from server to client, other way around otherwise
*/
eAuthPacketType AuthLink::OpcodeToType( uint8_t opcode ) const
{
    // packets coming in from server to client
    if(m_direction==AuthLinkType::Client)
    {
        switch(opcode)
        {
        case 0: return SMSG_AUTHVERSION;
        case 1: return SMSG_AUTH_ALREADY_LOGGED_IN;
        case 2: return SMSG_AUTH_INVALID_PASSWORD;
        case 3: return PKT_AUTH_LOGIN_SUCCESS;
        case 4: return SMSG_AUTH_SERVER_LIST;
        case 5: return CMSG_DB_CONN_FAILURE;
        case 6: return SMSG_AUTH_OK; // still a type of error ?
        case 7: return PKT_SELECT_SERVER_RESPONSE;
        case 9: return SMSG_AUTH_ERROR;
        }

    }
    else
    {
        // packets incoming from client to server
        switch(opcode)
        {
        case 0: return CMSG_AUTH_LOGIN;
        case 2: return CMSG_AUTH_SELECT_DBSERVER;
        case 3: return CMSG_RECONNECT_ATTEMPT;
        case 5: return CMSG_AUTH_REQUEST_SERVER_LIST;
        }
    }
    return MSG_AUTH_UNKNOWN;
}
//! tries to convert the available bytes into a valid AuthHandler LinkLevelEvent.
SEGSEvent * AuthLink::bytes_to_event()
{
    uint16_t  packet_size(0);
    uint8_t * tmp(nullptr);

    while(true) // we loop and loop and loop loopy loop through the buffery contents!
    {
        if(m_received_bytes_storage.GetReadableDataSize()<=2) // no more bytes for us, so we'll go hungry for a while
            return nullptr;
        // And the skies are clear on the Packet Fishing Waters
        m_received_bytes_storage.uGet(packet_size); // Ah ha! I smell packet in there!
        if(m_received_bytes_storage.GetReadableDataSize()<packet_size) // tis' a false trail capt'n !
        {
            m_received_bytes_storage.PopFront(1); // Crew, Dead Slow Ahead ! we're in hunting mode !
            continue;
        }
        // this might be a live packet in there
        tmp = m_received_bytes_storage.GetBuffer()+2;

        m_codec.XorDecodeBuf(tmp, packet_size+1); // Let's see what's in those murky waters
        eAuthPacketType recv_type = OpcodeToType(tmp[0]);
        AuthLinkEvent *evt = AuthEventFactory::EventForType(recv_type); // Crow's nest, report !
        if(!evt)
        {
            if(m_received_bytes_storage.GetReadableDataSize()>2) // False alarm Skipper!
                m_received_bytes_storage.uGet(packet_size); // On to next adventure crew. Slow Ahead !
            continue;
        }
        // A catch !
        if(evt->type() == evLogin) // Is tis' on of those pesky AuthLogin Packets ?!!?
        {
            // Bring out the Codec Cannon, an' load it with Des
            m_codec.DesDecode(tmp+1,30); // It'll crack it's chitinous armor
        }
        evt->serializefrom(m_received_bytes_storage);
        m_received_bytes_storage.PopFront(packet_size+3); //Let's sail away from this depleted fishery.
        return evt; // And throw our catch to the Cook.
    }
}
/**
    \brief Called when we start to service a new connection, here we tell reactor to wake us when queue() is not empty.
    \return -1 if there was a problem opening a new link, getting remote's address or registering client handler
  */
int AuthLink::open (void *p)
{
    m_state=AuthLink::INITIAL;
    if (this->m_peer.get_remote_addr (m_peer_addr) == -1)
        ACE_ERROR_RETURN ((LM_ERROR,ACE_TEXT ("%p\n"),ACE_TEXT ("get_remote_addr")),-1);
    if (EventProcessor::open (p) == -1)
        return -1;
    // Register this as a READ handler, this way will be notified/waken up when new bytes are available
    if (this->reactor () && this->reactor ()->register_handler(this,ACE_Event_Handler::READ_MASK) == -1)
        ACE_ERROR_RETURN ((LM_ERROR,ACE_TEXT ("%p\n"),ACE_TEXT ("unable to register client handler")),-1);
    // m_notifier will tell reactor to wake us when new packet is ready for sending
    m_notifier.reactor(reactor());                      // notify reactor with write event,
    msg_queue()->notification_strategy (&m_notifier);   // whenever there is a new event on msg_queue() we will be notified
    //TODO: consider using sync query here.
    m_target->putq(new ConnectEvent(this,m_peer_addr)); // also, inform the AuthHandler of our existence
    return 0;
}
/**
    \brief Called when new bytes are available.
    \return -1 on error
*/
int AuthLink::handle_input( ACE_HANDLE )
{
    const size_t INPUT_SIZE = 4096;
    char buffer[INPUT_SIZE];
    ssize_t recv_cnt;
    if ((recv_cnt = m_peer.recv(buffer, sizeof(buffer))) <= 0)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Connection closed\n")));
        return -1;
    }
    ACE_Guard<ACE_Thread_Mutex> guard_buffer(*m_buffer_mutex);
    m_received_bytes_storage.PutBytes((uint8_t *)buffer,recv_cnt);
    m_received_bytes_storage.ResetReading();
    // early out optimization
    if(m_received_bytes_storage.GetReadableDataSize()<2)
        return 0; // if not enough data even for the simplest of packets

    // For now BytesEvent will copy the buffer contents
    SEGSEvent *s_event=bytes_to_event(); // convert raw bytes into higher level event
    //TODO: what about partially received events ?
    if(s_event)
    {
        s_event->src(this); // allows upper levels to post responses to us
        m_target->putq(s_event);
    }
    return 0;
}
/**
        \brief Called from ACEReactor when there are events in our queue()
*/
int AuthLink::handle_output( ACE_HANDLE /*= ACE_INVALID_HANDLE*/ )
{
    SEGSEvent *ev;
    ACE_Time_Value nowait (ACE_OS::gettimeofday ());
    while (-1 != getq(ev, &nowait))
    {
        if(ev->type()==SEGS_EventTypes::evFinish)
        {
            ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Error sent, closing connection\n")));
            return -1;
        }
        if(ev->type()==evContinue) // we have asked ourselves to send leftovers
        {
            assert(m_unsent_bytes_storage.GetReadableDataSize() > 0); // be sure we have some
        }
        else
        {
            size_t start_offset=m_unsent_bytes_storage.GetReadableDataSize();
            encode_buffer(static_cast<AuthLinkEvent *>(ev),start_offset);
        }
        if(!send_buffer()) // trying to send the contents of the buffer
        {
            ev->release(); // we have failed somehow
            break;
        }
        ev->release();
    }
    if (msg_queue()->is_empty ()) // we don't want to be woken up
        reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    else // unless there is something to send still
        reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    return 0;
}

void AuthLink::encode_buffer(const AuthLinkEvent *ev,size_t start)
{
    assert(ev);
    if(ev==nullptr)
        return;
    // remember the location we'll put the packet size into
    uint16_t *packet_size = reinterpret_cast<uint16_t *>(m_unsent_bytes_storage.write_ptr());
    // put 0 as size for now
    m_unsent_bytes_storage.uPut((uint16_t)0);
    // remember start location
    size_t actual_packet_start = m_unsent_bytes_storage.GetReadableDataSize();
    // store bytes
    ev->serializeto(m_unsent_bytes_storage);
    // calculate the number of stored bytes, and set it in packet_size,
    // -1 because opcode is not counted toward packet size
    *packet_size = (m_unsent_bytes_storage.GetReadableDataSize() - actual_packet_start) - 1; 

    // additional encryption of login details
    if(ev->type()==evLogin)
        m_codec.DesCode(m_unsent_bytes_storage.read_ptr()+actual_packet_start+1,30); //only part of packet is encrypted with des

    // every packet, but the authorization protocol, is encrypted
    if(ev->type()!=evAuthProtocolVersion)
        m_codec.XorCodeBuf(m_unsent_bytes_storage.read_ptr()+actual_packet_start,m_unsent_bytes_storage.GetReadableDataSize()-actual_packet_start); // opcode gets encrypted

}

bool AuthLink::send_buffer()
{
    ssize_t send_cnt = m_peer.send(m_unsent_bytes_storage.read_ptr(), m_unsent_bytes_storage.GetReadableDataSize());
    if (send_cnt == -1)
        ACE_ERROR ((LM_ERROR,ACE_TEXT ("(%P|%t) %p\n"), ACE_TEXT ("send")));
    else
    {
        m_unsent_bytes_storage.PopFront(send_cnt); // this many bytes were sent
    }
    if (m_unsent_bytes_storage.GetReadableDataSize() > 0) // and still there is something left
    {
        ungetq(new ContinueEvent);
        return false; // couldn't send all
    }
    return true;
}

static uint64_t KeyPrepare(const char *co_string)
{
    uint64_t t = 0;
    char *p_llt = (char *)&t;
    int index = 0;
    assert(co_string);
    if(!co_string)
        return t;
    while(co_string[0])
    {
        char val = co_string[0];
        if( index >= 40 )
            break;
        p_llt[index&7] = (p_llt[index&7] ^ val);
        co_string ++;
        index ++;
    }
    return t;
}

void AuthLink::set_protocol_version( int vers )
{
    static char key_30207[] = { 0x39, 0x3D, 0x33, 0x2A, 0x32, 0x3B, 0x78, 0x5D,
                                0x31, 0x31, 0x73, 0x61, 0x3B, 0x2F, 0x34, 0x73,
                                0x64, 0x2E, 0x25, 0x2B, 0x24, 0x40, 0x61, 0x2A,
                                0x27, 0x21, 0x32, 0x7A, 0x30, 0x7E, 0x67, 0x60,
                                0x7B, 0x7A, 0x5D, 0x3F, 0x6E, 0x38, 0x28, 0};
    m_protocol_version = vers;
    if(m_protocol_version==30206)
        m_codec.SetDesKey(KeyPrepare("TEST"));
    else
        m_codec.SetDesKey(KeyPrepare((char *)key_30207));
}
/**
  \brief Called when this handler is removed from the ACE_Reactor.
*/
int AuthLink::handle_close( ACE_HANDLE handle,ACE_Reactor_Mask close_mask )
{
    // client handle was closed, posting disconnect event with higher priority
    m_target->msg_queue()->enqueue_prio(new DisconnectEvent(session_token()),nullptr,100);
    if (close_mask == ACE_Event_Handler::WRITE_MASK)
        return 0;
    return super::handle_close (handle, close_mask);

}

void AuthLink::dispatch( SEGSEvent */*ev*/ )
{
    assert(!"Should not be called");
}

//! @}
