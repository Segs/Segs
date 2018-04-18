#include "SEGSEvent.h"

SEGSEvent SEGSEvent::s_ev_finish(SEGS_EventTypes::evFinish,nullptr);

const char *SEGSEvent::info()
{
    return typeid(*this).name();
}
