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

#include "BitStream.h"
#include "Entity.h"
#include "Costume.h"
#include "Friend.h"
#include "GameData/serialization_common.h"
#include "Servers/GameDatabase/GameDBSyncEvents.h"
#include "GameData/chardata_serializers.h"
#include "GameData/entitydata_serializers.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/playerdata_serializers.h"
#include "Servers/MapServer/DataHelpers.h"
#include "Logging.h"
#include <QtCore/QString>
#include <QtCore/QDebug>

// Anonymous namespace
namespace {

}
// End Anonymous namespace

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
    m_char_data.m_level=0;
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
    m_trays.serializeto(bs);
}

void Character::GetCharBuildInfo(BitStream &src)
{
    m_char_data.m_level=0;
    src.GetString(m_char_data.m_class_name);
    src.GetString(m_char_data.m_origin_name);
    CharacterPower primary,secondary;
    primary.power_id.serializefrom(src);
    secondary.power_id.serializefrom(src);

    m_powers.push_back(primary); // primary_powerset power
    m_powers.push_back(secondary); // secondary_powerset power
    m_trays.serializefrom(src);
}

void Character::SendCharBuildInfo(BitStream &bs) const
{
    Character c = *this;
    PowerPool_Info null_power = {0,0,0};
    bs.StoreString(getClass(c));   // class name
    bs.StoreString(getOrigin(c));  // origin name
    bs.StorePackedBits(5,getCombatLevel(c)); // related to combat level?
    PUTDEBUG("SendCharBuildInfo after plevel");

    {
        // TODO: this is character powers related, refactor it out of here.
        int count=0; // FixMe: count is explicitly set and never modified.
        bs.StorePackedBits(4,count); // count
        for(int i=0; i<count; i++)
        {
            uint32_t num_powers=m_powers.size();
            bs.StorePackedBits(5,0);
            bs.StorePackedBits(4,num_powers);
            for(const CharacterPower &power : m_powers)
            {
                //sendPower(bs,0,0,0);
                power.power_id.serializeto(bs);
                bs.StorePackedBits(5,power.bought_at_level);
                bs.StoreFloat(power.range);
                bs.StorePackedBits(4,power.boosts.size());

                for(const CharacterPowerBoost &boost : power.boosts)
                {
                    //sendPower(bs,0,0,0);
                    boost.boost_id.serializeto(bs);
                    bs.StorePackedBits(5,boost.level);
                    bs.StorePackedBits(2,boost.num_combines);
                }
            }
        }
    }
    PUTDEBUG("SendCharBuildInfo after powers");
    // main tray inspirations
    {
        uint32_t max_num_cols=3;
        uint32_t max_num_rows=1;
        bs.StorePackedBits(3,max_num_cols); // count
        bs.StorePackedBits(3,max_num_rows); // count
        for(uint32_t col=0; col<max_num_cols; col++)
        {
            for(size_t row=0; row<max_num_rows; ++row)
            {
                bool is_power=false; // FixMe: is_power is explicitly set and never modified.
                bs.StoreBits(1,is_power);
                if(is_power)
                    null_power.serializeto(bs);
            }
        }
    }
    PUTDEBUG("SendCharBuildInfo after inspirations");
    // boosts
    uint32_t num_boosts=0; // FixMe: num_boosts is never modified and the body of the for loop below will never fire.
    bs.StorePackedBits(5,num_boosts); // count
    for(size_t idx=0; idx<num_boosts; ++idx)
    {
        bool set_boost=false;
        bs.StorePackedBits(3,0); // boost idx
        bs.StoreBits(1,set_boost); // 1 set, 0 clear
        if(set_boost)
        {
            int level=0;
            int num_combines=0;
            null_power.serializeto(bs);
            bs.StorePackedBits(5,level); // boost idx
            bs.StorePackedBits(2,num_combines); // boost idx
        }
    }
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

void Character::DumpPowerPoolInfo( const PowerPool_Info &pool_info )
{
    for (int i = 0; i < 3; i++)
    {
        qDebug().nospace().noquote() << "    "
                                     << QString("Pool_id[%1]: %2 %3 %4")
                                            .arg(i)
                                            .arg(pool_info.category_idx)
                                            .arg(pool_info.powerset_entry_idx)
                                            .arg(pool_info.power_idx);
    }
}

void Character::DumpBuildInfo()
{
    Character &c = *this;
    QString msg = "CharDebug\n  "
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
    //DumpPowerPoolInfo(m_powers[0].power_id);
    //DumpPowerPoolInfo(m_powers[1].power_id);
}

void Character::dump()
{
    DumpBuildInfo();
    DumpSidekickInfo();
    qDebug() <<"//------------------Tray------------------";
    m_trays.dump();
    qDebug() <<"//-----------------Costume-----------------";
    if(!m_costumes.empty())
        getCurrentCostume()->dump();
    qDebug() <<"//-----------------Options-----------------";
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
