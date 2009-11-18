#pragma once
#include "SEGSEvent.h"

template<class BUFFER,class LINK>
class LinkLevelEvent : public SEGSEvent
{
protected:
    virtual ~LinkLevelEvent() {}
public:
	LinkLevelEvent(size_t evtype,EventProcessor *ev_src=0) : SEGSEvent(evtype,ev_src)
	{}
	virtual void serializeto(BUFFER &) const=0;
	virtual void serializefrom(BUFFER &)=0;
};
