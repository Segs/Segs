#pragma once

#include <ace/Connector.h>
#include <ace/Svc_Handler.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <ace/Reactor_Notification_Strategy.h>
#include <string>

class AuthPacket;
class AuthConnection;
class pktAuthVersion;
class AuthObserver;
class AuthInterface;
class Auth_Handler : public ACE_Svc_Handler<ACE_SOCK_STREAM,ACE_NULL_SYNCH>
{
    typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> PARENT;
public:
    Auth_Handler():notifier_ (0, this, ACE_Event_Handler::WRITE_MASK),m_observer(0)
    {
        ACE_ASSERT(!"Auth_Handler should not be automatically instantiated!");
    }
    Auth_Handler(AuthObserver *observer):notifier_ (0, this, ACE_Event_Handler::WRITE_MASK),m_observer(observer)
    {}

    virtual int open (void * = 0);

    // Called when input is available from the client.
    virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);

    // Called when output is possible.
    virtual int handle_output (ACE_HANDLE fd = ACE_INVALID_HANDLE);

    // Called when a timer expires.
    virtual int handle_timeout (const ACE_Time_Value &current_time,
        const void *act = 0);

private:
    enum { ITERATIONS = 5 };
    int iterations_;
    ACE_Reactor_Notification_Strategy notifier_;
    AuthConnection *m_conn;
    AuthObserver *m_observer;
};

class Auth_Client_Connector : public ACE_Connector<Auth_Handler,ACE_SOCK_CONNECTOR>
{
    typedef ACE_Connector<Auth_Handler,ACE_SOCK_CONNECTOR> PARENT;
public:
    // Initialization
    Auth_Client_Connector (void){};
    ~Auth_Client_Connector (void){};
private:
    ACE_INET_Addr remote_addr_; // this is where the auth server lives
};
// High level representation of Client-Server language

class AuthObserver
{
public:
    virtual void notify_connection_created(AuthInterface *conn)=0; //! This will allow the observ to send commands to the AuthServer
    virtual void notify_connected()=0;
    virtual void notify_disconnected()=0;
    virtual void notify_raw_data(char *dat,size_t sz)=0;
    virtual void notify_packet(pktAuthVersion *)=0;
};

class AuthInterface // interface wrapper to Auth server functions
{
    AuthConnection *m_conn;
public:
    AuthInterface(AuthConnection *); // this is connection this interface sends requests on.
    void login(const std::string & login,const std::string & passw);
};
