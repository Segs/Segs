#pragma once

#include "LinkLevelEvent.h"
#include "Buffer.h"
#include <QString>


typedef SerializableEvent<GrowingBuffer> AdminLinkEvent;
enum AdminEventTypes
{
    evContinue=SEGS_EventTypes::evLAST_EVENT,
    evLoginRequest,
    evLoginResponse,
    evServerListRequest,
    evServerListResponse,
    evUNKNOWN,
};
class ContinueEvent : public SEGSEvent // this event is posted from AuthLink to AuthLink, it means there are leftover unsent bytes.
{
public:
    ContinueEvent() : SEGSEvent(evContinue)
    {}
};

struct AdminLoginRequest : SEGSEvent
{
    QString login;
    QString pass;
    AdminLoginRequest() : SEGSEvent(AdminEventTypes::evLoginRequest) {}
    template<class Archive>
    void serialize( Archive & ar )
    {
        ar( login );
        ar( pass );
    }
};
struct AdminLoginResponse : SEGSEvent
{
    int login_status;
    AdminLoginResponse() : SEGSEvent(AdminEventTypes::evLoginResponse) {}
    template<class Archive>
    void serialize( Archive & ar )
    {
        ar( login_status );
    }
};
