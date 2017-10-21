#include "ChatDividerMoved.h"

void ChatDividerMoved::serializefrom(BitStream &src)
{
    m_position = src.GetFloat();
}

void ChatDividerMoved::serializeto(BitStream &tgt) const
{
    tgt.StorePackedBits(1,16);
    tgt.StoreFloat(m_position);
}
