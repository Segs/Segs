#pragma once
#include <ace/Task_Ex_T.h>
#include "SEGSEvent.h"
// Independent Task, each EventProcessor when activated dequeues SEGSEvents,
// and dispatches them
class EventProcessor : public ACE_Task_Ex<ACE_MT_SYNCH,SEGSEvent>
{
typedef ACE_Task_Ex<ACE_MT_SYNCH,SEGSEvent> super;


public:
        int             open(void *args = nullptr);
        int             handle_timeout(const ACE_Time_Value &current_time, const void *act /* = 0 */);
        int             svc(void);
virtual void            dispatch(SEGSEvent *ev)=0;
virtual SEGSEvent *     dispatch_sync(SEGSEvent *ev)=0;
};
