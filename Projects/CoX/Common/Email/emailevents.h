#ifndef EMAILEVENTS_H
#define EMAILEVENTS_H

enum EmailEventTypes : uint32_t
{
    evEmailHeaderRequest,
    evEmailRead,
    evEmailSent,
    evEmailDelete
};

#define ONE_WAY_MESSAGE(name)\
struct name ## Message final : public InternalEvent\
{\
    name ## Data m_data;\
    name ## Message(name ## Data &&d) :  InternalEvent(AuthDBEventTypes::ev ## name),m_data(d) {}\
};

#endif // EMAILEVENTS_H
