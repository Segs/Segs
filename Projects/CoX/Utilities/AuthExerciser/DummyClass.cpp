#include "DummyClass.h"

DummyClass::DummyClass()
{

}

void DummyClass::dispatch(SEGSEvent *ev)
{

}

SEGSEvent* DummyClass::dispatch_sync(SEGSEvent *ev)
{
    return ev;
}
