#include "Auth.h"
#include "AuthConnection_ClientSide.h"
#include "AuthPacket.h"
#include "AuthOpcodes.h"
#include "AuthProtocol.h"
int Auth_Handler::open (void *p)
{
	ACE_Time_Value iter_delay (2);   // Two seconds
	if (PARENT::open (p) == -1)
		return -1;

	m_conn = new AuthConnection_ClientSide(m_observer);
	ACE_INET_Addr remote;
	peer().get_remote_addr(remote);	
	m_observer->notify_connection_created(new AuthInterface(m_conn));
	m_conn->Established(remote);

	this->notifier_.reactor (this->reactor ());
	this->msg_queue ()->notification_strategy (&this->notifier_);
	int res= this->reactor ()->schedule_timer(this, 0, ACE_Time_Value::zero, iter_delay);
	return res;
}
int Auth_Handler::handle_input (ACE_HANDLE)
{
	char buf[64];
	ssize_t recv_cnt = this->peer ().recv (buf, sizeof (buf) - 1);
	if (recv_cnt > 0)
	{
		m_conn->ReceivedBytes(reinterpret_cast<u8 *>(buf),recv_cnt);
		return 0;
	}
	if (recv_cnt == 0 || ACE_OS::last_error () != EWOULDBLOCK)
	{
		this->reactor ()->end_reactor_event_loop ();
		m_conn->Closed();
		return -1;
	}
	return 0;
}
int Auth_Handler::handle_timeout(const ACE_Time_Value &, const void *)
{
	if (this->iterations_ >= ITERATIONS)
	{
		this->peer ().close_writer ();
		return 0;
	}

	ACE_Message_Block *mb;
	char msg[128];
	ACE_OS::sprintf (msg, "Iteration %d\n", this->iterations_);
	ACE_NEW_RETURN (mb, ACE_Message_Block (msg), -1);
	this->putq (mb);
	return 0;
}
int Auth_Handler::handle_output (ACE_HANDLE)
{
	ACE_Message_Block *mb;
	ACE_Time_Value nowait (ACE_OS::gettimeofday ());
	while (-1 != this->getq (mb, &nowait))
	{
		ssize_t send_cnt = this->peer ().send (mb->rd_ptr (), mb->length ());
		if (send_cnt == -1)
			ACE_ERROR ((LM_ERROR,ACE_TEXT ("(%P|%t) %p\n"),ACE_TEXT ("send")));
		else
			mb->rd_ptr (ACE_static_cast (size_t, send_cnt));
		if (mb->length () > 0)
		{
			this->ungetq (mb);
			break;
		}
		mb->release ();
	}
	if (this->msg_queue ()->is_empty ())
		this->reactor ()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
	else
		this->reactor ()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
	return 0;
}

AuthInterface::AuthInterface( AuthConnection * connection ) : m_conn(connection)
{
	
}

void AuthInterface::login(const std::string &login,const std::string &passw )
{
	pktAuthLogin *pkt = static_cast<pktAuthLogin *>(AuthPacketFactory::PacketForType(CMSG_AUTH_LOGIN));
	strncpy(pkt->username,login.c_str(),14);
	strncpy(pkt->password,passw.c_str(),14);
	pkt->m_seed = 0;
	m_conn->getProtocol()->sendPacket(pkt);

}