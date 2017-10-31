#ifndef ILINK_H
#define ILINK_H

#include "EventProcessor.h"

class BitStream;
class ILink : public EventProcessor
{
public:
    virtual ~ILink() = default;
    virtual void received_block(BitStream &)=0;
};

#endif // ILINK_H
