#include "SEGSEvent.h"


const char *SEGSEvent::info()
{
    return typeid(*this).name();
}
