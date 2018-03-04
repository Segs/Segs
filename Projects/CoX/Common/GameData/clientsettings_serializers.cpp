#include "clientsettings_serializers.h"

#include "clientsettings_definitions.h"
#include "DataStorage.h"
#include "serialization_common.h"

CEREAL_CLASS_VERSION(ClientSettingsData, 1); // register CharacterData class version

template<class Archive>
void serialize(Archive & archive, ClientSettingsData &cd, uint32_t const version)
{
    archive(cereal::make_nvp("MouseSpeed",cd.mouse_speed));
    archive(cereal::make_nvp("TurnSpeed",cd.turn_speed));
    archive(cereal::make_nvp("InvertMouse",cd.invert_mouse));
    archive(cereal::make_nvp("Win4AutoFade",cd.win_4_autofade));
    archive(cereal::make_nvp("Win7AutoFade",cd.win_7_autofade));
    archive(cereal::make_nvp("ShowTooltips",cd.show_tooltips));
    archive(cereal::make_nvp("ProfanityFilter",cd.disable_profanity_filter));
    archive(cereal::make_nvp("ChatBalloons",cd.use_chat_baloons));
    archive(cereal::make_nvp("ArchetypeVisibility",cd.archetype_visibility));
    archive(cereal::make_nvp("SuperGroupVisibility",cd.supergroup_visibility));
    archive(cereal::make_nvp("PlayerName",cd.player_name));
    archive(cereal::make_nvp("PlayerBars",cd.player_bars));
    archive(cereal::make_nvp("EnemyName",cd.enemy_name));
    archive(cereal::make_nvp("EnemyBars",cd.enemy_bars));
    archive(cereal::make_nvp("PlayerReticles",cd.player_reticles));
    archive(cereal::make_nvp("EnemyReticles",cd.enemy_reticles));
    archive(cereal::make_nvp("AssistReticles",cd.assist_reticles));
    archive(cereal::make_nvp("FontSize",cd.font_size));
}

void saveTo(const ClientSettingsData & target, const QString & baseName, bool text_format)
{
    commonSaveTo(target,"ClientSettingsData",baseName,text_format);
}

template
void serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive & archive, ClientSettingsData & m, uint32_t const version);
template
void serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive & archive, ClientSettingsData & m, uint32_t const version);

void serializeToDb(const ClientSettingsData &data, QString &tgt)
{
    std::ostringstream ostr;
    {
        cereal::JSONOutputArchive ar(ostr);
        ar(data);
    }
    tgt = QString::fromStdString(ostr.str());
}

void serializeFromDb(ClientSettingsData &data,const QString &src)
{
    if(src.isEmpty())
        return;
    std::istringstream istr;
    istr.str(src.toStdString());
    {
        cereal::JSONInputArchive ar(istr);
        ar(data);
    }
}
