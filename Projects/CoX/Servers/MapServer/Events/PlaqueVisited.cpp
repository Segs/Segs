#include "PlaqueVisited.h"


void PlaqueVisited::serializefrom(BitStream & src)
{
    src.GetString(m_name);
    m_pos.x = src.GetFloat();
    m_pos.y = src.GetFloat();
    m_pos.z = src.GetFloat();
}

void PlaqueVisited::serializeto(BitStream &) const
{
    assert(!"unimplemented");
}
