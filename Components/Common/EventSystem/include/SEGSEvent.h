#pragma once
class EventProcessor;
class SEGSEvent
{
protected:
	EventProcessor *m_event_source;
	size_t m_type;

    virtual ~SEGSEvent()
    {
        m_event_source=0;
    }
public:
	enum {
		evFinish=0, // this event will finish the Processor that receives it
		evContinue=1,
		evAuthConnect=3,
        evAuthDisconnect,
		evAuthProtocolVersion,
		evAuthorizationError,
		evServerSelectRequest,
		evDbError,
		evLogin,
		evLoginResponse,
		evServerListResponse,
		evServerListRequest,
		evServerSelectResponse,
        evLAST_EVENT=0xFFFFFFFF
	};
	SEGSEvent(size_t evtype,EventProcessor *ev_src=0) : m_event_source(ev_src),m_type(evtype)
	{

	}
	void release()
	{
		delete this;
	}
	void				src(EventProcessor *ev_src) {m_event_source=ev_src;}
	EventProcessor *	src() {return m_event_source;}
	size_t              type() const {return m_type;}
};
