
#include "AuthOpcodes.h"
#include "AuthLink.h"
#include "AuthHandler.h"
#include "AuthEventFactory.h"

EventProcessor *AuthLink::g_target=0;

AuthLink::AuthLink() :  m_client(0),
                        m_received_bytes_storage(0x1000,0,40),
                        m_unsent_bytes_storage(0x200,0,40),
                        m_protocol_version(-1),
                        m_state(INITIAL),
                        m_notifier(0, this, ACE_Event_Handler::WRITE_MASK)
                        
{
	ACE_ASSERT(g_target);
}
AuthLink::~AuthLink( void )
{
    m_client=0;
}
eAuthPacketType AuthLink::OpcodeToType( u8 opcode,bool direction /*= false */ ) const
{
	switch(opcode)
	{	
	case 0:
		if(direction)
			return SMSG_AUTHVERSION;
		else
			return CMSG_AUTH_LOGIN;
	case 2:
		return CMSG_AUTH_SELECT_DBSERVER;
	case 3:
		return CMSG_DB_CONN_FAILURE;
	case 4:
		return CMSG_AUTH_LOGIN;
	case 5:
		return CMSG_AUTH_REQUEST_SERVER_LIST;
	case 6:
		return CMSG_AUTH_LOGIN;
    default:
        return MSG_AUTH_UNKNOWN;
	}
	return MSG_AUTH_UNKNOWN;
}

SEGSEvent * AuthLink::bytes_to_event() 
{
	u16  packet_size(0);
	u8 * tmp(NULL);

	while(true) // we loop and loop and loop loopy loop through the buffery contents!
	{
		if(m_received_bytes_storage.GetReadableDataSize()<=2) // no more bytes for us, so we'll go hungry for a while
			return NULL;
		// And the skies are clear on the Packet Fishing Waters
		m_received_bytes_storage.uGet(packet_size);	// Ah ha! I smell packet in there!
		if(m_received_bytes_storage.GetReadableDataSize()<packet_size) // tis' a false trail capt'n !
		{
			m_received_bytes_storage.PopFront(1); // Crew, Dead Slow Ahead ! we're in hunting mode !
			continue;
		}
		// this might be a live packet in there
		tmp = (u8 *)&(m_received_bytes_storage.GetBuffer()[2]);

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
			m_codec.DesDecode(static_cast<u8*>(&tmp[1]),24); // It'll crack it's chitinous armor
		}
		evt->serializefrom(m_received_bytes_storage); 
		m_received_bytes_storage.PopFront(packet_size+3); //Let's sail away from this depleted fishery.
		return evt; // And throw our catch to the Cook.
	}
}

int AuthLink::open (void *p)
{
    m_state=AuthLink::INITIAL;
    if (this->peer_.get_remote_addr (m_peer_addr) == -1)
        ACE_ERROR_RETURN ((LM_ERROR,ACE_TEXT ("%p\n"),ACE_TEXT ("get_remote_addr")),-1);
	if (EventProcessor::open (p) == -1)
		return -1;
    if (this->reactor () && this->reactor ()->register_handler(this,ACE_Event_Handler::READ_MASK) == -1)
        ACE_ERROR_RETURN ((LM_ERROR,ACE_TEXT ("%p\n"),ACE_TEXT ("unable to register client handler")),-1);

	m_notifier.reactor(reactor());						// notify reactor with write event, 
	msg_queue()->notification_strategy (&m_notifier);	// whenever there is a new event on msg_queue()
	ConnectEvent *res=new ConnectEvent(this);
	res->src_addr=m_peer_addr;
	g_target->putq(res);				// inform the AuthHandler of our existence
	return 0;
}

int AuthLink::handle_input( ACE_HANDLE ) /*! some bytes arrived */
{
	const size_t INPUT_SIZE = 4096;
	char buffer[INPUT_SIZE];
	ssize_t recv_cnt;
	if ((recv_cnt = peer_.recv(buffer, sizeof(buffer))) <= 0)
	{
		ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Connection closed\n")));
		return -1;
	}
	m_received_bytes_storage.PutBytes((u8 *)buffer,recv_cnt);
	m_received_bytes_storage.ResetReading();
	if(m_received_bytes_storage.GetReadableDataSize()<2) // not enough data even for the simplest of packets
		return 0;
	// For now BytesEvent will copy the buffer contents
	SEGSEvent *s_event=bytes_to_event();
	s_event->src(this); // allows upper levels to post responses to us
	g_target->putq(s_event);
	return 0;
}

int AuthLink::handle_output( ACE_HANDLE /*= ACE_INVALID_HANDLE*/ )
{
	SEGSEvent *ev;
	ACE_Time_Value nowait (ACE_OS::gettimeofday ());
	while (-1 != getq(ev, &nowait))
	{
		if(ev->type()==SEGSEvent::evFinish)
		{
			ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Error sent, closing connection\n")));
			return -1;
		}
		if(ev->type()==evContinue) // we have asked ourselves to send leftovers
		{
			ACE_ASSERT(m_unsent_bytes_storage.GetReadableDataSize() > 0); // be sure we have some
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
    ACE_ASSERT(ev);
    if(ev==0)
        return;
	// remember the location we'll put the packet size into
    u16 *packet_size = reinterpret_cast<u16 *>(m_unsent_bytes_storage.write_ptr()); 
	// put 0 as size for now
	m_unsent_bytes_storage.uPut((u16)0);
	// remember start location
	size_t actual_packet_start = m_unsent_bytes_storage.GetDataSize();
	// store bytes
	ev->serializeto(m_unsent_bytes_storage);
	// calculate the number of stored bytes, and set it in packet_size
	*packet_size = (m_unsent_bytes_storage.GetDataSize() - actual_packet_start) - 1; // -1 because opcode is not counted toward packet size

    // every packet, but the authorization protocol, is encrypted
	if(ev->type()!=evAuthProtocolVersion)
		m_codec.XorCodeBuf(static_cast<u8 *>(m_unsent_bytes_storage.GetBuffer())+start+2,m_unsent_bytes_storage.GetDataSize()-2); // opcode gets encrypted

    // additional encryption of login details
	if(ev->type()==evLogin) 
		m_codec.DesCode(m_unsent_bytes_storage.GetBuffer()+start+3,24); //only part of packet is encrypted with des
}

bool AuthLink::send_buffer()
{
	ssize_t send_cnt = peer_.send(m_unsent_bytes_storage.read_ptr(), m_unsent_bytes_storage.GetReadableDataSize());
	if (send_cnt == -1)
		ACE_ERROR ((LM_ERROR,ACE_TEXT ("(%P|%t) %p\n"),	ACE_TEXT ("send")));
	else
	{
        m_unsent_bytes_storage.PopFront(send_cnt); // this many bytes were read
	}
	if (m_unsent_bytes_storage.GetReadableDataSize() > 0) // and still there is something left
	{
		ungetq(new ContinueEvent);
		return false; // couldn't send all
	}
	return true;
}

static u64 KeyPrepare(const char *co_string)
{
	u64 t = 0;
	char *p_llt = (char *)&t;
	int index = 0;
    ACE_ASSERT(co_string);
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
	static char key_30207[] = {	0x39, 0x3D, 0x33, 0x2A, 0x32, 0x3B, 0x78, 0x5D,
								0x31, 0x31, 0x73, 0x61, 0x3B, 0x2F, 0x34, 0x73,
								0x64, 0x2E, 0x25, 0x2B, 0x24, 0x40, 0x61, 0x2A,
								0x27, 0x21, 0x32, 0x7A, 0x30, 0x7E, 0x67, 0x60,
								0x7B, 0x7A, 0x5D, 0x3F, 0x6E, 0x38, 0x28, 0};
	m_protocol_version = vers;
	if(m_protocol_version==30206)
		m_codec.SetDesKey(KeyPrepare("TEST\0\0\0"));
	else
		m_codec.SetDesKey(KeyPrepare((char *)key_30207));
}

int AuthLink::handle_close( ACE_HANDLE handle,ACE_Reactor_Mask close_mask )
{
    // client handle was closed, posting disconnect event with higher priority
    g_target->msg_queue()->enqueue_prio(new DisconnectEvent(this),0,100);
    if (close_mask == ACE_Event_Handler::WRITE_MASK)
        return 0;
    return super::handle_close (handle, close_mask);

}

void AuthLink::dispatch( SEGSEvent * )
{
    ACE_ASSERT(!"Should not be called");
}
