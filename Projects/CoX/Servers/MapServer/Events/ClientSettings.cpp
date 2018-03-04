#include "ClientSettings.h"

//#include "BitStream.h"


void ClientSettings::serializefrom(BitStream & bs)
{
    data.mouse_speed = bs.GetFloat();
    data.turn_speed = bs.GetFloat();
    data.invert_mouse = bs.GetBits(1);
    data.win_4_autofade = bs.GetBits(1);
    data.win_7_autofade = bs.GetBits(1);
    data.show_tooltips = bs.GetBits(1);
    data.disable_profanity_filter = bs.GetBits(1);
    data.use_chat_baloons = bs.GetBits(1);
    data.archetype_visibility = (ClientSettingsData::Show)bs.GetBits(3);
    data.supergroup_visibility = (ClientSettingsData::Show)bs.GetBits(3);
    data.player_name = (ClientSettingsData::Show)bs.GetBits(3);
    data.player_bars = (ClientSettingsData::Show)bs.GetBits(3);
    data.enemy_name = (ClientSettingsData::Show)bs.GetBits(3);
    data.enemy_bars = (ClientSettingsData::Show)bs.GetBits(3);
    data.player_reticles=(ClientSettingsData::Show)bs.GetBits(3);
    data.enemy_reticles=(ClientSettingsData::Show)bs.GetBits(3);
    data.assist_reticles=(ClientSettingsData::Show)bs.GetBits(3);
    data.font_size = bs.GetPackedBits(5);
}

void ClientSettings::serializeto(BitStream &tgt) const
{
    qDebug() << "Serializing to ClientSettings";
    tgt.StorePackedBits(1,65);
    tgt.StoreFloat(data.mouse_speed);
    tgt.StoreFloat(data.turn_speed);
    tgt.StoreBits(1,data.invert_mouse);
    tgt.StoreBits(1,data.win_4_autofade);
    tgt.StoreBits(1,data.win_7_autofade);
    tgt.StoreBits(1,data.show_tooltips);
    tgt.StoreBits(1,data.disable_profanity_filter);
    tgt.StoreBits(1,data.use_chat_baloons);
    tgt.StoreBits(3,data.archetype_visibility);
    tgt.StoreBits(3,data.supergroup_visibility);
    tgt.StoreBits(3,data.player_name);
    tgt.StoreBits(3,data.player_bars);
    tgt.StoreBits(3,data.enemy_name);
    tgt.StoreBits(3,data.enemy_bars);
    tgt.StoreBits(3,data.player_reticles);
    tgt.StoreBits(3,data.enemy_reticles);
    tgt.StoreBits(3,data.assist_reticles);
    tgt.StoreBits(5,data.font_size);
}
