#ifndef CHATDIVIDERMOVED_H
#define CHATDIVIDERMOVED_H

#include "MapEvents.h"
#include "MapLink.h"

#include <QtCore/QString>

class ChatDividerMoved : public MapLinkEvent
{
public:
    float m_position;
    ChatDividerMoved() : MapLinkEvent(MapEventTypes::evChatDividerMoved) {}

    // SerializableEvent interface
    void serializefrom(BitStream &src) override;
    void serializeto(BitStream &) const override;
};

#endif // CHATDIVIDERMOVED_H
