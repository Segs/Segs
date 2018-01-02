#pragma once

#include <EventProcessor.h>

class DummyClass : public EventProcessor
{
public:
    DummyClass();

    // EventProcessor interface
    virtual void dispatch(SEGSEvent *ev);
    virtual SEGSEvent *dispatch_sync(SEGSEvent *ev);
};
