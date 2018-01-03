#pragma once

#include <EventProcessor.h>

class DummyClass : public EventProcessor
{
public:
    DummyClass();

    // EventProcessor interface
    virtual void dispatch(SEGSEvent *ev);
    virtual SEGSEvent * dispatchSync(SEGSEvent *ev);

protected:
    class AuthLink *      m_our_link = nullptr;

    void onConnect(class ConnectEvent * ev);
    void onServerVersion(class AuthorizationProtocolVersion * ev);
};
