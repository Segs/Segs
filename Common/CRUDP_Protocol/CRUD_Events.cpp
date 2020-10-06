#include "CRUD_Events.h"

#include "Components/BitStream.h"


using namespace SEGSEvents;

const uint8_t *Packet::bytes() const
{
    return m_pkt->GetStream()->read_ptr();
}
size_t Packet::size() const
{
    return m_pkt->GetStream()->GetReadableDataSize();
}

void ConnectRequest::serializeto(BitStream &tgt) const
{
    tgt.StorePackedBits(1, 1); // opcode 1
    tgt.StorePackedBits(1, m_tickcount);
    tgt.StorePackedBits(1, m_version);
}
void ConnectRequest::serializefrom(BitStream &src)
{
    m_tickcount = src.GetPackedBits(1);
    m_version   = src.GetPackedBits(1);
}


void ConnectResponse::serializeto(BitStream &tgt) const
{
    tgt.StorePackedBits(1, 0); // ctrl opcode
    tgt.StorePackedBits(1, 4); // opcode
}


void DisconnectRequest::serializefrom(BitStream &bs)
{
    bs.GetPackedBits(1);
    bs.GetPackedBits(1);
}
void DisconnectRequest::serializeto(BitStream &bs) const
{
    bs.StorePackedBits(1, 0);
    bs.StorePackedBits(1, 5);
}
void DisconnectResponse::serializefrom(BitStream &bs)
{
    bs.GetPackedBits(1);
    bs.GetPackedBits(1);
}
void DisconnectResponse::serializeto(BitStream &bs) const
{
    bs.StorePackedBits(1, 0);
    bs.StorePackedBits(1, 6);
}
void Idle::serializefrom(BitStream &bs)
{
    // TODO: check this
    bs.GetPackedBits(1);
    bs.GetPackedBits(1);
}
void Idle::serializeto(BitStream &bs) const
{
    bs.StorePackedBits(1, 0);
    bs.StorePackedBits(1, 0);
}


SEGSEvents::CRUDLink_Event *CRUD_EventFactory::EventFromStream(BitStream &bs)
{
    int32_t opcode = bs.GetPackedBits(1);
    if(opcode != 0)
        return nullptr;
    // it seems idle commands can be shortened to only contain opcode==0.
    int32_t control_opcode = bs.GetReadableBits() == 0 ? 0 : bs.GetPackedBits(1);
    switch (control_opcode)
    {
    case 0:
        return new SEGSEvents::Idle(); // CTRL_IDLE
    case 5:
        return new SEGSEvents::DisconnectRequest(); // CTRL_DISCONNECT_REQ
    default: break;
    }
    qWarning("Unhandled control event type %d", control_opcode);
    return nullptr;
}
