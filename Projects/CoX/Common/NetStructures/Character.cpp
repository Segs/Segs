/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup NetStructures Projects/CoX/Common/NetStructures
 * @{
 */

#include "Character.h"

#include "CharacterHelpers.h"
#include "BitStream.h"
#include "Entity.h"
#include "Costume.h"
#include "Friend.h"
#include "serialization_common.h"
#include "Servers/GameDatabase/GameDBSyncEvents.h"
#include "GameData/chardata_serializers.h"
#include "GameData/entitydata_serializers.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/playerdata_serializers.h"
#include "GameData/GameDataStore.h"
#include "Servers/MapServer/DataHelpers.h"
#include "Logging.h"
#include <QtCore/QString>
#include <QtCore/QDebug>

using namespace SEGSEvents;

Character::Character()
{
    m_multiple_costumes                         = false;
    m_current_costume_idx                       = 0;
    m_current_costume_set                       = false;
    m_char_data.m_supergroup_costume            = false;
    m_sg_costume                                = nullptr;
    m_char_data.m_using_sg_costume              = false;
    m_char_data.m_current_attribs.m_HitPoints   = 25;
    m_max_attribs.m_HitPoints                   = 50;
    m_char_data.m_current_attribs.m_Endurance   = 33;
    m_max_attribs.m_Endurance                   = 43;
    m_char_data.m_has_titles = m_char_data.m_has_the_prefix
            || !m_char_data.m_titles[0].isEmpty()
            || !m_char_data.m_titles[1].isEmpty()
            || !m_char_data.m_titles[2].isEmpty();
    m_char_data.m_sidekick.m_has_sidekick = false;
    m_char_data.m_current_attribs.initAttribArrays();
}

void Character::reset()
{
    m_char_data.m_level = 0;
    m_char_data.m_combat_level = m_char_data.m_level + 1;
    m_name="EMPTY";
    m_char_data.m_class_name="EMPTY";
    m_char_data.m_origin_name="EMPTY";
    m_villain=false;
    m_multiple_costumes=false;
    m_current_costume_idx=0;
    m_current_costume_set=false;
    m_char_data.m_supergroup_costume=false;
    m_sg_costume=nullptr;
    m_char_data.m_using_sg_costume=false;
    m_char_data.m_has_titles = false;
    m_char_data.m_sidekick.m_has_sidekick = false;
    m_char_data.m_powersets.clear();
}


bool Character::isEmpty()
{
    return ( 0==m_name.compare("EMPTY",Qt::CaseInsensitive)&&
            (0==m_char_data.m_class_name.compare("EMPTY",Qt::CaseInsensitive)));
}

void Character::setName(const QString &val )
{
    if(val.length()>0)
        m_name = val;
    else
        m_name = "EMPTY";
}

void Character::sendTray(BitStream &bs) const
{
    m_char_data.m_trays.serializeto(bs);
}

void Character::finalizeLevel(const GameDataStore &data)
{
    uint32_t max_xp = data.expMaxLevel();

    m_char_data.m_combat_level = m_char_data.m_level + 1; // m_combat_level is display level?

    if(m_char_data.m_combat_level >= max_xp)
        m_char_data.m_combat_level = max_xp - 1;

    if(m_char_data.m_experience_points < data.expForLevel(m_char_data.m_combat_level))
        m_char_data.m_experience_points = data.expForLevel(m_char_data.m_combat_level);

    m_char_data.m_max_insp_rows = data.countForLevel(m_char_data.m_combat_level, data.m_pi_schedule.m_InspirationRow);
    m_char_data.m_max_insp_cols = data.countForLevel(m_char_data.m_combat_level, data.m_pi_schedule.m_InspirationCol);
    m_char_data.m_max_enhance_slots = data.countForLevel(m_char_data.m_combat_level, data.m_pi_schedule.m_BoostSlot);

    // TODO: client will only accept 5col x 4row of insps MAX, see Issue #524
    assert(m_char_data.m_max_insp_cols <= 5 || m_char_data.m_max_insp_rows <= 4);

    int num_powersets = m_char_data.m_powersets.size();
    for(int idx = 1; idx < num_powersets; ++idx) // Skipping 0 powerset (temporary powers)
    {
        CharacterPowerSet pset = m_char_data.m_powersets[idx];
        for(CharacterPower &pow : pset.m_powers)
            reserveEnhancementSlot(data,m_char_data, &pow);
    }

    m_char_data.m_powers_updated = false;
}

void Character::addStartingInspirations(const GameDataStore &data)
{
    // TODO: We can make this configurable in settings.cfg
    QStringList starting_insps = {
        "Resurgence",       // 25.2.3
        "Phenomenal_Luck",  // 25.2.0
    };

    for (QString &name : starting_insps)
        addInspirationByName(data,m_char_data, name);
}

void Character::getStartingPowers(const GameDataStore &data,const QString &pcat_name, const QString &pset_name, const QStringList &power_names)
{
    PowerPool_Info ppool;

    ppool.m_pcat_idx = getPowerCatByName(data,pcat_name);
    ppool.m_pset_idx = getPowerSetByName(data,pset_name, ppool.m_pcat_idx);

    addPowerSet(m_char_data, ppool); // add empty powerset

    for(const QString &name : power_names)
    {
        ppool.m_pow_idx = getPowerByName(data,name, ppool.m_pcat_idx, ppool.m_pset_idx);
        addPower(data,m_char_data, ppool);
    }
}

void Character::getPowerFromBuildInfo(const GameDataStore &data,BitStream &src)
{
    for(int i = 0; i < 2; ++i)
    {
        PowerPool_Info ppinfo;
        ppinfo.serializefrom(src);

        addPower(data,m_char_data, ppinfo);
    }
}

void Character::GetCharBuildInfo(BitStream &src, const GameDataStore &data)
{
    m_char_data.m_level = 0;
    m_char_data.m_combat_level = m_char_data.m_level + 1;
    src.GetString(m_char_data.m_class_name);
    src.GetString(m_char_data.m_origin_name);

    // TODO: Make these configurable in settings.cfg?
    QStringList starting_temps =
    {
        "EMP_Glove",                    // 27.0.0
        "Cryoprojection_Bracers",       // 27.0.1
    };

    // TODO: Make these configurable in settings.cfg?
    QStringList inherent_and_preorders =
    {
        "Brawl",                        // 26.0.0
        "prestige_generic_Sprintp",     // 26.0.1
        //"prestige_preorder_Sprintp",    // 26.0.2
        //"prestige_BestBuy_Sprintp",     // 26.0.3
        //"prestige_EB_Sprintp",          // 26.0.4
        //"prestige_Gamestop_Sprintp",    // 26.0.5
        "Sprint",                       // 26.0.6
        "Rest",                         // 26.0.7
    };

    // Temporary Powers MUST come first (must be idx 0)
    getStartingPowers(data,QStringLiteral("Temporary_Powers"), QStringLiteral("Temporary_Powers"), starting_temps);
    getStartingPowers(data,QStringLiteral("Inherent"), QStringLiteral("Inherent"), inherent_and_preorders);
    getPowerFromBuildInfo(data,src);     // primary, secondary
    finalizeLevel(data);
    addStartingInspirations(data);      // resurgence and phenomenal_luck

    m_char_data.m_trays.serializefrom(src);
}

void Character::sendEnhancements(BitStream &bs) const
{
    bs.StorePackedBits(5, m_char_data.m_enhancements.size()); // count
    for(size_t i = 0; i < m_char_data.m_enhancements.size(); ++i)
    {
        bs.StorePackedBits(3, m_char_data.m_enhancements[i].m_slot_idx); // boost idx, maybe use m_enhancement_idx
        bs.StoreBits(1, m_char_data.m_enhancements[i].m_slot_used); // 1 set, 0 clear
        if(m_char_data.m_enhancements[i].m_slot_used)
        {
            m_char_data.m_enhancements[i].m_enhance_info.serializeto(bs);
            bs.StorePackedBits(5, m_char_data.m_enhancements[i].m_level); // boost idx
            bs.StorePackedBits(2, m_char_data.m_enhancements[i].m_num_combines); // boost idx
        }
    }
}

void Character::sendInspirations(BitStream &bs) const
{
    int max_cols = m_char_data.m_max_insp_cols;
    int max_rows = m_char_data.m_max_insp_rows;

    bs.StorePackedBits(3, max_cols); // count
    bs.StorePackedBits(3, max_rows); // count

    for(int i = 0; i < max_cols; ++i)
    {
        for(int j = 0; j < max_rows; ++j)
        {
            bs.StoreBits(1, m_char_data.m_inspirations[i][j].m_has_insp);
            if(m_char_data.m_inspirations[i][j].m_has_insp)
                m_char_data.m_inspirations[i][j].m_insp_info.serializeto(bs);
        }
    }
}

void Character::sendOwnedPowers(BitStream &bs) const
{
    bs.StorePackedBits(4, m_char_data.m_powersets.size()); // count
    for(const CharacterPowerSet &pset : m_char_data.m_powersets)
    {
        bs.StorePackedBits(5, pset.m_level_bought);
        bs.StorePackedBits(4, pset.m_powers.size());
        for(const CharacterPower &power : pset.m_powers)
        {
            power.m_power_info.serializeto(bs);
            bs.StorePackedBits(5, power.m_level_bought);
            bs.StoreFloat(power.m_range);

            bs.StorePackedBits(4, power.m_total_eh_slots);
            for(int i = 0; i < power.m_total_eh_slots; ++i)
            {
                bs.StoreBits(1, power.m_enhancements[i].m_slot_used); // slot has enhancement
                if(power.m_enhancements[i].m_slot_used)
                {
                    power.m_enhancements[i].m_enhance_info.serializeto(bs);
                    bs.StorePackedBits(5, power.m_enhancements[i].m_level);
                    bs.StorePackedBits(2, power.m_enhancements[i].m_num_combines);
                }
            }
        }
    }
}

void Character::SendCharBuildInfo(BitStream &bs) const
{
    Character c = *this;
    bs.StoreString(getClass(c));   // class name
    bs.StoreString(getOrigin(c));  // origin name
    bs.StorePackedBits(5, getCombatLevel(c)); // related to combat level?
    PUTDEBUG("SendCharBuildInfo after plevel");

    // Owned Powers
    sendOwnedPowers(bs);
    PUTDEBUG("SendCharBuildInfo after powers");

    // Inspirations
    sendInspirations(bs);
    PUTDEBUG("SendCharBuildInfo after inspirations");

    // Enhancements
    sendEnhancements(bs);
    PUTDEBUG("SendCharBuildInfo after boosts");
}

void Character::serializetoCharsel( BitStream &bs, const QString& entity_map_name )
{
    Character c = *this;
    bs.StorePackedBits(1,getLevel(c));
    bs.StoreString(getName());
    bs.StoreString(getClass(c));
    bs.StoreString(getOrigin(c));
    if(m_costumes.size()==0)
    {
        assert(m_name.compare("EMPTY")==0); // only empty characters can have no costumes
        CharacterCostume::NullCostume.storeCharsel(bs);
    }
    else
        m_costumes[m_current_costume_set].storeCharsel(bs);
    bs.StoreString(entity_map_name);
    bs.StorePackedBits(1,1);
}

const CharacterCostume * Character::getCurrentCostume() const
{
    assert(!m_costumes.empty());
    if(m_current_costume_set)
        return &m_costumes[m_current_costume_idx];
    else
        return &m_costumes[0];
}

void Character::serialize_costumes(BitStream &bs, const ColorAndPartPacker *packer , bool all_costumes) const
{
    // full costume
    if(all_costumes) // this is only sent to the current player
    {
        bs.StoreBits(1,m_current_costume_set);
        if(m_current_costume_set)
        {
            bs.StoreBits(32,m_current_costume_idx);
            bs.StoreBits(32,uint32_t(m_costumes.size()));
        }
        bs.StoreBits(1,m_multiple_costumes);
        if(m_multiple_costumes)
        {
            for(const Costume & c : m_costumes)
            {
                ::serializeto(c,bs,packer);
            }
        }
        else
        {
            ::serializeto(m_costumes[m_current_costume_idx],bs,packer);
        }
        bs.StoreBits(1,m_char_data.m_supergroup_costume);
        if(m_char_data.m_supergroup_costume)
        {
            ::serializeto(*m_sg_costume,bs,packer);

            bs.StoreBits(1,m_char_data.m_using_sg_costume);
        }
    }
    else // other player's costumes we're sending only their current.
    {
        ::serializeto(*getCurrentCostume(),bs,packer);
    }
}

void Character::DumpSidekickInfo()
{
    QString msg = QString("Sidekick Info\n  has_sidekick: %1 \n  db_id: %2 \n  type: %3 ")
            .arg(m_char_data.m_sidekick.m_has_sidekick)
            .arg(m_char_data.m_sidekick.m_db_id)
            .arg(m_char_data.m_sidekick.m_type);

    qDebug().noquote() << msg;
}

void Character::DumpBuildInfo()
{
    Character &c = *this;
    QString msg = "//--------------Char Debug--------------\n  "
            + getName()
            + "\n  " + getOrigin(c)
            + "\n  " + getClass(c)
            + "\n  db_id: " + QString::number(m_db_id)
            + "\n  acct: " + QString::number(getAccountId())
            + "\n  lvl/clvl: " + QString::number(getLevel(c)) + "/" + QString::number(getCombatLevel(c))
            + "\n  inf: " + QString::number(getInf(c))
            + "\n  xp/debt: " + QString::number(getXP(c)) + "/" + QString::number(getDebt(c))
            + "\n  lfg: " + QString::number(m_char_data.m_lfg)
            + "\n  afk: " + QString::number(m_char_data.m_afk)
            + "\n  description: " + getDescription(c)
            + "\n  battleCry: " + getBattleCry(c)
            + "\n  Last Online: " + m_char_data.m_last_online;

    qDebug().noquote() << msg;
}

void Character::dump()
{
    DumpBuildInfo();
    qDebug() << "//--------------Owned Powers--------------";
    dumpOwnedPowers(m_char_data);
    qDebug() << "//-----------Owned Inspirations-----------";
    dumpInspirations(m_char_data);
    qDebug() << "//-----------Owned Enhancements-----------";
    dumpEnhancements(m_char_data);
    qDebug() << "//--------------Sidekick Info--------------";
    DumpSidekickInfo();
    qDebug() << "//------------------Tray------------------";
    m_char_data.m_trays.dump();
    qDebug() << "//-----------------Costume-----------------";
    if(!m_costumes.empty())
        getCurrentCostume()->dump();
}

void Character::recv_initial_costume( BitStream &src, const ColorAndPartPacker *packer )
{
    assert(m_costumes.size()==0);
    m_costumes.emplace_back();
    m_current_costume_idx=0;
    ::serializefrom(m_costumes.back(),src,packer);
}

void serializeStats(const Parse_CharAttrib &src,BitStream &bs, bool /*sendAbsolute*/)
{
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,0);
    bs.StorePackedBits(5,src.m_HitPoints/5.0f);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,1);
    bs.StorePackedBits(5,src.m_Endurance/5.0f);
    bs.StoreBits(1,0); // no more data
}

void serializeFullStats(const Parse_CharAttrib &src,BitStream &bs, bool /*sendAbsolute*/)
{
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,0);
    bs.StorePackedBits(7,src.m_HitPoints);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,1);
    bs.StorePackedBits(7,src.m_Endurance);
    bs.StoreBits(1,0); // no more data
}

void serializeLevelsStats(const Character &src,BitStream &bs, bool /*sendAbsolute*/)
{
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,0);
    bs.StorePackedBits(4,getLevel(src));
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,1);
    bs.StorePackedBits(4,getCombatLevel(src));
    bs.StoreBits(1,0); // no more data
}

void serializeStats(const Character &src,BitStream &bs, bool sendAbsolute)
{
    // Send CurrentAttribs
    // Send MaxAttribs
    // Send levels
    uint32_t field_idx=0;
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // CurrentAttribs
    serializeStats(src.m_char_data.m_current_attribs,bs,sendAbsolute);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // MaxAttribs
    serializeStats(src.m_max_attribs,bs,sendAbsolute);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // levels
    serializeLevelsStats(src,bs,sendAbsolute);
    bs.StoreBits(1,0); // we have no more data
}

void serializeFullStats(const Character &src,BitStream &bs, bool sendAbsolute)
{
    uint32_t field_idx=0;
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // first field is CurrentAttribs
    serializeFullStats(src.m_char_data.m_current_attribs,bs,sendAbsolute);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // first field is MaxAttribs
    serializeFullStats(src.m_max_attribs,bs,sendAbsolute);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // levels
    serializeLevelsStats(src,bs,sendAbsolute);
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // ExperiencePoints
    bs.StorePackedBits(16,getXP(src));
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // ExperienceDebt
    bs.StorePackedBits(16,getDebt(src));
    bs.StoreBits(1,1); // we have more data
    bs.StorePackedBits(1,field_idx++); // Influence
    bs.StorePackedBits(16,getInf(src));

    bs.StoreBits(1,0); // we have no more data
}
void Character::sendFullStats(BitStream &bs) const
{
    // if sendAbsolutoOverride
    serializeFullStats(*this,bs,false);
}

void Character::sendDescription(BitStream &bs) const
{

    qCDebug(logDescription) << "Sending Description & BattleCry"
             << "\nDescription: " << m_char_data.m_character_description
             << "\nBattle Cry: " << m_char_data.m_battle_cry;

    bs.StoreString(m_char_data.m_character_description);
    bs.StoreString(m_char_data.m_battle_cry);
}

void Character::sendTitles(BitStream &bs, NameFlag hasname, ConditionalFlag conditional) const
{
    if(hasname == NameFlag::HasName)
        bs.StoreString(getName());

    bs.StoreBits(1, m_char_data.m_has_the_prefix); // an index to a title prefix ( 0 - None; 1 - The )

    if(conditional)
    {
        storeStringConditional(bs, m_char_data.m_titles[0]); // Title 1 - generic title (first)
        storeStringConditional(bs, m_char_data.m_titles[1]); // Title 2 - origin title (second)
        storeStringConditional(bs, m_char_data.m_titles[2]); // Title 3 - yellow title (special)
    }
    else
    {
        bs.StoreString(m_char_data.m_titles[0]);             // Title 1 - generic title (first)
        bs.StoreString(m_char_data.m_titles[1]);             // Title 2 - origin title (second)
        bs.StoreString(m_char_data.m_titles[2]);             // Title 3 - yellow title (special)
    }
}

void Character::sendFriendList(BitStream &bs) const
{
    const FriendsList *fl(&m_char_data.m_friendlist);
    bs.StorePackedBits(1,1); // v2 = force_update
    bs.StorePackedBits(1,fl->m_friends_count);

    for(int i=0; i<fl->m_friends_count; ++i)
    {
        bs.StoreBits(1,fl->m_has_friends); // if false, client will skip this iteration
        bs.StorePackedBits(1,fl->m_friends[i].m_db_id);
        bs.StoreBits(1,fl->m_friends[i].m_online_status);
        bs.StoreString(fl->m_friends[i].m_name);
        bs.StorePackedBits(1,fl->m_friends[i].m_class_idx);
        bs.StorePackedBits(1,fl->m_friends[i].m_origin_idx);

        if(!fl->m_friends[i].m_online_status)
            continue; // if friend is offline, the rest is skipped

        bs.StorePackedBits(1,fl->m_friends[i].m_map_idx);
        bs.StoreString(fl->m_friends[i].m_mapname);
    }
}

void toActualCostume(const GameAccountResponseCostumeData &src, Costume &tgt)
{
    tgt.skin_color = src.skin_color;
    try
    {
        tgt.serializeFromDb(src.m_serialized_data);
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << e.what();
    }

    tgt.m_non_default_costme_p = false;
}

void fromActualCostume(const Costume &src,GameAccountResponseCostumeData &tgt)
{
    tgt.skin_color = src.skin_color;
    try
    {
        src.serializeToDb(tgt.m_serialized_data);
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << e.what();
    }
}

bool toActualCharacter(const GameAccountResponseCharacterData &src,
                       Character &tgt,PlayerData &player, EntityData &entity)
{
    CharacterData &  cd(tgt.m_char_data);

    tgt.m_db_id      = src.m_db_id;
    tgt.m_account_id = src.m_account_id;
    tgt.setName(src.m_name);

    try
    {
        serializeFromQString(cd,src.m_serialized_chardata);
        serializeFromQString(player,src.m_serialized_player_data);
        serializeFromQString(entity, src.m_serialized_entity_data);
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << e.what();
    }

    for (const GameAccountResponseCostumeData &costume : src.m_costumes)
    {
        tgt.m_costumes.emplace_back();
        CharacterCostume &main_costume(tgt.m_costumes.back());
        toActualCostume(costume, main_costume);
        // appearance related.
        main_costume.m_body_type = src.m_costumes.back().m_body_type;
        main_costume.setSlotIndex(costume.m_slot_index);
        main_costume.setCharacterId(costume.m_character_id);
    }
    return true;
}

bool fromActualCharacter(const Character &src,const PlayerData &player,
                         const EntityData &entity, GameAccountResponseCharacterData &tgt)
{
    const CharacterData &  cd(src.m_char_data);

    tgt.m_db_id      = src.m_db_id;
    tgt.m_account_id = src.m_account_id;
    tgt.m_name = src.getName();

    try
    {
        serializeToQString(cd, tgt.m_serialized_chardata);
        serializeToQString(player, tgt.m_serialized_player_data);
        serializeToQString(entity, tgt.m_serialized_entity_data);
    }
    catch(cereal::RapidJSONException &e)
    {
        qWarning() << e.what();
    }
    catch(std::exception &e)
    {
        qCritical() << e.what();
    }

    for (const CharacterCostume &costume : src.m_costumes)
    {
        tgt.m_costumes.emplace_back();
        GameAccountResponseCostumeData &main_costume(tgt.m_costumes.back());
        fromActualCostume(costume, main_costume);
        // appearance related.
        main_costume.m_body_type = src.m_costumes.back().m_body_type;
        main_costume.m_slot_index = costume.getSlotIndex();
        main_costume.m_character_id= costume.getCharacterId();
        main_costume.m_height = costume.m_height;
        main_costume.m_physique = costume.m_physique;
    }
    return true;
}

//! @}
