#include "CharacterData.h"

#include "Components/serialization_common.h"
#include "Components/serialization_types.h"


template<class Archive>
void CharacterData::serialize(Archive &archive, uint32_t const version)
{
    if(version != CharacterData::class_version)
    {
        qCritical() << "Failed to serialize CharacterData, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("Level",m_level));
    archive(cereal::make_nvp("CombatLevel",m_combat_level));
    archive(cereal::make_nvp("SecurityThreat",m_security_threat));
    archive(cereal::make_nvp("XP",m_experience_points));
    archive(cereal::make_nvp("Debt",m_experience_debt));
    archive(cereal::make_nvp("PatrolXP",m_experience_patrol));
    archive(cereal::make_nvp("Influence",m_influence));
    archive(cereal::make_nvp("HasTitles",m_has_titles));
    archive(cereal::make_nvp("ThePrefix",m_has_the_prefix));
    archive(cereal::make_nvp("Titles",m_titles));
    archive(cereal::make_nvp("BattleCry",m_battle_cry));
    archive(cereal::make_nvp("Description",m_character_description));
    archive(cereal::make_nvp("AFK",m_afk));
    archive(cereal::make_nvp("AfkMsg",m_afk_msg));
    archive(cereal::make_nvp("LFG",m_lfg));
    archive(cereal::make_nvp("Alignment",m_alignment));
    archive(cereal::make_nvp("LastCostumeID",m_current_costume_idx));
    archive(cereal::make_nvp("LastOnline",m_last_online));
    archive(cereal::make_nvp("Class",m_class_name));
    archive(cereal::make_nvp("Origin",m_origin_name));
    archive(cereal::make_nvp("SuperGroupCostume",m_has_sg_costume));
    archive(cereal::make_nvp("UsingSGCostume",m_using_sg_costume));
    archive(cereal::make_nvp("SideKick",m_sidekick));
    archive(cereal::make_nvp("FriendList",m_friendlist));
    archive(cereal::make_nvp("CurrentAttribs", m_current_attribs));
    archive(cereal::make_nvp("OnTaskForce", m_is_on_task_force));
    archive(cereal::make_nvp("PowerSets", m_powersets));
    archive(cereal::make_nvp("PowerTrayGroups", m_trays));
    archive(cereal::make_nvp("Inspirations", m_inspirations));
    archive(cereal::make_nvp("Enhancements", m_enhancements));

    archive(cereal::make_nvp("MaxInspirationCols", m_max_insp_cols));
    archive(cereal::make_nvp("MaxInspirationRows", m_max_insp_rows));
    archive(cereal::make_nvp("MaxEnhancementSlots", m_max_enhance_slots));
}

SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(CharacterData)
CEREAL_CLASS_VERSION(CharacterData, CharacterData::class_version) // register PowerTrayGroup class version

