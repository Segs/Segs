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
