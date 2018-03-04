#pragma once
#include "../MapEvents.h"
#include "GameData/clientsettings_definitions.h"

class ClientSettings final : public MapLinkEvent
{
public:
    ClientSettingsData data;
    ClientSettings() : MapLinkEvent(MapEventTypes::evClientSettings) {}
    void serializeto(BitStream &bs) const override;
    void serializefrom(BitStream &bs) override;
};
