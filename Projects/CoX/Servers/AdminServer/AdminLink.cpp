#include "AdminLink.h"

#include "AdminEvents.h"

#include <cereal/archives/binary.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>
#include <strstream>
#include <cassert>
EventProcessor *AdminLink::g_target=nullptr;

namespace cereal {
inline void epilogue(BinaryOutputArchive &, QString const &) { }
inline void epilogue(BinaryInputArchive &, QString const &) { }
inline void epilogue(JSONOutputArchive &, QString const &) { }
inline void epilogue(JSONInputArchive &, QString const &) { }

inline void prologue(JSONOutputArchive &, QString const &) { }
inline void prologue(JSONInputArchive &, QString const &) { }
inline void prologue(BinaryOutputArchive &, QString const &) { }
inline void prologue(BinaryInputArchive &, QString const &) { }

template<class Archive> inline void CEREAL_SAVE_FUNCTION_NAME(Archive & ar, ::QString const & str)
{
    ar( str.toStdString() );
}
//! Serialization for basic_string types, if binary data is supported
template<class Archive> inline void CEREAL_LOAD_FUNCTION_NAME(Archive & ar, ::QString & str)
{
    std::string rd;
    ar( rd );
    str = QString::fromStdString(rd);
}
}
namespace {
AdminEventTypes OpcodeToType( uint8_t opcode)
{
    switch(opcode)
    {
        case 1: return AdminEventTypes::evLoginRequest;
    }
    return evUNKNOWN;
}
SEGSEvent *bytes_to_event(GrowingBuffer &buf)
{
    uint16_t  packet_size {0};
    uint8_t * tmp {nullptr};

    while(true)
    {
        if(buf.GetReadableDataSize()<=2)
            return nullptr;
        buf.uGet(packet_size);
        if(buf.GetReadableDataSize()<packet_size)
        {
            // not enough bytes in buffer to decode the packet skip it.
            buf.PopFront(1);
            continue;
        }
        tmp = buf.GetBuffer();

        SEGSEvent *deserialized=nullptr;
        std::string wrapped((const char *)tmp+1,buf.GetReadableDataSize());
        std::stringstream string_stream_wrap(wrapped);
        cereal::JSONInputArchive arc(string_stream_wrap);
        switch(OpcodeToType(tmp[0]))
        {
            case evLoginRequest:
            {
                AdminLoginRequest *res = new AdminLoginRequest();
                arc(*res);
                deserialized = res;
            }
        }
        if(!deserialized)
        {
            // bad packet, cannot be decoded, retry with following bytes
            if(buf.GetReadableDataSize()>2)
                buf.uGet(packet_size);
            continue;
        }
        buf.PopFront(packet_size+3); //Let's sail away from this depleted fishery.
        return deserialized; // And throw our catch to the Cook.
    }
}
void event_to_bytes(const SEGSEvent *ev,GrowingBuffer & buf)
{
    assert(ev);
    if(ev==nullptr)
        return;
    // remember the location we'll put the packet size into
    uint16_t *packet_size = reinterpret_cast<uint16_t *>(buf.write_ptr());
    // put 0 as size for now
    buf.uPut((uint16_t)0);
    // remember start location
    size_t actual_packet_start = buf.GetReadableDataSize();
    // store bytes
    std::stringstream string_stream_wrap;
    {
        cereal::JSONOutputArchive arc(string_stream_wrap);
        switch(ev->type())
        {
            case evLoginResponse: {
                arc(*static_cast<const AdminLoginResponse *>(ev));
            }
        }
    }
    buf.PutString(string_stream_wrap.str().c_str());
    // calculate the number of stored bytes, and set it in packet_size
    *packet_size = buf.GetReadableDataSize() - actual_packet_start;
}
}

AdminLink::AdminLink() :
    m_notifier(nullptr, nullptr, ACE_Event_Handler::WRITE_MASK),
    m_state(INITIAL),
    m_received_bytes_storage(0x1000,0,40),
    m_unsent_bytes_storage(0x400,0,40),
    m_buffer_mutex()
{
    m_notifier.event_handler(this); // notify 'this' object on WRITE events
}

int AdminLink::open(void *p)
{
    if (this->peer_.get_remote_addr (m_peer_addr) == -1)
        ACE_ERROR_RETURN ((LM_ERROR,ACE_TEXT ("%p\n"),ACE_TEXT ("get_remote_addr")),-1);
    if (EventProcessor::open (p) == -1)
        return -1;
    m_state = INITIAL;
    // Register this as a READ handler, this way will be notified/waken up when new bytes are available
    if (this->reactor () && this->reactor ()->register_handler(this,ACE_Event_Handler::READ_MASK) == -1)
        ACE_ERROR_RETURN ((LM_ERROR,ACE_TEXT ("%p\n"),ACE_TEXT ("unable to register client handler")),-1);
    // m_notifier will tell reactor to wake us when new packet is ready for sending
    m_notifier.reactor(reactor());                      // notify reactor with write event,
    msg_queue()->notification_strategy (&m_notifier);   // whenever there is a new event on msg_queue() we will be notified

    g_target->putq(new ConnectEvent(this,m_peer_addr)); // also, inform the AuthHandler of our existence
    return 0;

}

int AdminLink::handle_input(ACE_HANDLE)
{
    const size_t INPUT_SIZE = 4096;
    char buffer[INPUT_SIZE];
    ssize_t recv_cnt;
    if ((recv_cnt = peer_.recv(buffer, sizeof(buffer))) <= 0)
    {
        ACE_DEBUG ((LM_DEBUG,ACE_TEXT ("(%P|%t) Connection closed\n")));
        return -1;
    }
    ACE_Guard<ACE_Thread_Mutex> guard_buffer(m_buffer_mutex);
    m_received_bytes_storage.PutBytes((uint8_t *)buffer,recv_cnt);
    m_received_bytes_storage.ResetReading();
    // early out optimization
    if(m_received_bytes_storage.GetReadableDataSize()<2)
        return 0; // if not enough data even for the simplest of packets

    // For now BytesEvent will copy the buffer contents
    SEGSEvent *s_event=bytes_to_event(m_received_bytes_storage); // convert raw bytes into higher level event
    //TODO: what about partially received events ?
    if(s_event)
    {
        s_event->src(this); // allows upper levels to post responses to us
        g_target->putq(s_event);
    }
    return 0;
}

bool AdminLink::send_buffer()
{
    ssize_t send_cnt = peer_.send(m_unsent_bytes_storage.read_ptr(), m_unsent_bytes_storage.GetReadableDataSize());
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
int AdminLink::handle_output(ACE_HANDLE fd)
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
            event_to_bytes(ev,m_unsent_bytes_storage);
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

int AdminLink::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
    // client handle was closed, posting disconnect event with higher priority
    g_target->msg_queue()->enqueue_prio(new DisconnectEvent(this),nullptr,100);
    if (close_mask == ACE_Event_Handler::WRITE_MASK)
        return 0;
    return super::handle_close (handle, close_mask);
}

void AdminLink::dispatch(SEGSEvent *ev)
{
    assert(!"Should not be called");
}

SEGSEvent *AdminLink::dispatch_sync(SEGSEvent *ev)
{
    assert(!"Should not be called");
    return nullptr;
}
