/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

// segs includes
#include "CharacterDatabase.h"
#include "AccountInfo.h"
#include "AdminServer.h"
#include "Character.h"
#include "Costume.h"
#include "GameServer/GameServer.h"
#include "PlayerMethods.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtCore/QDebug>
#include <QtSql/QSqlError>
#include <QDateTime>
#include <cstdlib>
#include <cstdio>
using namespace std;
QMutex mutex;

CharacterDatabase::~CharacterDatabase()
{
    if(m_db)
        m_db->close();
}
static bool prepQuery(QSqlQuery &qr,const QString &txt) {
    if(!qr.prepare(txt)) {
        qDebug() << "SQL_ERROR:"<<qr.lastError();
        return false;
    }
    return true;
}
static bool doIt(QSqlQuery &qr) {
    QMutexLocker locker(&mutex);
    if(!qr.exec()) {
        qDebug() << "SQL_ERROR:"<<qr.lastError();
        return false;
    }
    return true;
}
void CharacterDatabase::on_connected(QSqlDatabase *db)
{
    m_db = db;
    if(!db)
        return;

    m_prepared_fill = QSqlQuery(*db);
    m_prepared_account_insert = QSqlQuery(*db);
    m_prepared_char_insert = QSqlQuery(*db);
    m_prepared_costume_insert = QSqlQuery(*db);
    m_prepared_char_select = QSqlQuery(*db);
    m_prepared_char_update = QSqlQuery(*db);
    m_prepared_costume_update = QSqlQuery(*db);
    m_prepared_account_select = QSqlQuery(*db);
    m_prepared_char_exists = QSqlQuery(*db);
    m_prepared_char_delete = QSqlQuery(*db);


    prepQuery(m_prepared_fill,"SELECT * FROM costume WHERE character_id=? AND costume_index=?");
    prepQuery(m_prepared_account_insert,"INSERT INTO accounts  (account_id,max_slots) VALUES (?,?)");
    prepQuery(m_prepared_char_insert,
                "INSERT INTO characters  ("
                "char_level, slot_index, account_id, char_name, archetype, origin, description, "
                "battlecry, current_map, bodytype, last_costume_id, last_online, hitpoints, endurance, "
                "inf, xp, xpdebt, xppatrol, alignment, posx, posy, posz, orientp, orienty, orientr, "
                "title, badgetitle, specialtitle, supergroup_id, options, gui"
                ") VALUES ("
                ":char_level, :slot_index, :account_id, :char_name, :archetype, :origin, :description, "
                ":battlecry, :current_map, :bodytype, :last_costume_id, :last_online, :hitpoints, :endurance, "
                ":inf, :xp, :xpdebt, :xppatrol, :alignment, :posx, :posy, :posz, :orientp, :orienty, :orientr, "
                ":title, :badgetitle, :specialtitle, :supergroup_id, :options, :gui"
                ")");
    prepQuery(m_prepared_costume_insert,
                "INSERT INTO costume (character_id,costume_index,skin_color,parts) VALUES "
                "(:id,:costume_index,:skin_color,:parts)");
    prepQuery(m_prepared_char_update,
                "UPDATE characters SET "
                "char_level=:char_level, char_name=:char_name, "
                "archetype=:archetype, origin=:origin, "
                "description=:description, battlecry=:battlecry, "
                "current_map=:current_map, bodytype=:bodytype, "
                "last_costume_id=:last_costume_id, "
                "last_online=:last_online, "
                "hitpoints=:hitpoints, endurance=:endurance, "
                "inf=:inf, xp=:xp, xpdebt=:xpdebt, "
                "xppatrol=:xppatrol, alignment=:alignment, "
                "posx=:posx, posy=:posy, posz=:posz, "
                "orientp=:orientp, orienty=:orienty, orientr=:orientr, "
                "title=:title, badgetitle=:badgetitle, "
                "specialtitle=:specialtitle, supergroup_id=:supergroup_id "
                "WHERE id=:id ");
    prepQuery(m_prepared_costume_update,
                "UPDATE costume SET "
                "costume_index=:costume_index, skin_color=:skin_color, parts=:parts "
                "WHERE character_id=:id ");
    prepQuery(m_prepared_char_select,"SELECT * FROM characters WHERE account_id=? AND slot_index=?");
    prepQuery(m_prepared_account_select,"SELECT * FROM accounts WHERE account_id=?");
    prepQuery(m_prepared_char_exists,"SELECT exists (SELECT 1 FROM characters WHERE char_name = $1 LIMIT 1)");
    prepQuery(m_prepared_char_delete,"DELETE FROM characters WHERE account_id=? AND slot_index=?");
}


// UserToken,
bool CharacterDatabase::remove_character(AccountInfo *c,int8_t slot_idx)
{
    assert(c!=nullptr);
    m_prepared_char_delete.bindValue(0,quint64(c->account_server_id()));
    m_prepared_char_delete.bindValue(1,(uint32_t)slot_idx);
    if(!doIt(m_prepared_char_delete))
        return false;
    return true;
}
bool CharacterDatabase::named_character_exists(const QString &name)
{
    m_prepared_char_exists.bindValue(0,name);
    if(!doIt(m_prepared_char_exists))
        return false;

    if(!m_prepared_char_exists.next())
        return false;
    // TODO: handle case of multiple accounts with same name ?
    return m_prepared_char_exists.value(0).toBool();
}
bool CharacterDatabase::fill( AccountInfo *c )
{
    assert(c&&c->account_server_id());

    m_prepared_account_select.bindValue(0,quint64(c->account_server_id()));
    if(!doIt(m_prepared_account_select))
        return false;


    if(!m_prepared_account_select.next())
        return false;
    // TODO: handle case of multiple accounts with same name ?

    c->max_slots(m_prepared_account_select.value("max_slots").toUInt());
    c->game_server_id(m_prepared_account_select.value("id").toULongLong());

    ACE_DEBUG((LM_INFO,"CharacterClient id: %i\n",c->account_server_id()));
    ACE_DEBUG((LM_INFO,"CharacterClient slots: %i\n",c->max_slots()));
    return true;
}
#define STR_OR_EMPTY(c) ((!c.isEmpty()) ? c:"EMPTY")
#define STR_OR_VERY_EMPTY(c) ((c!=0) ? c:"")
bool CharacterDatabase::fill( Character *c)
{
    CharacterData *cd = &c->m_char_data;
    assert(c&&c->getAccountId());

    m_prepared_char_select.bindValue(0,quint64(c->getAccountId()));
    m_prepared_char_select.bindValue(1,(uint16_t)c->getIndex());
    if(!doIt(m_prepared_char_select))
        return false;

    if(!m_prepared_char_select.next())
    {
        c->reset(); // empty slot
        return true;
    }
//    else if (results.num_rows()>1)
//        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill query returned wrong number of results. %s failed.\n"), query.str().c_str()),false);

    c->m_db_id = (m_prepared_char_select.value("id").toUInt());
    qDebug() << "m_db_id:" << c->m_db_id;
    c->m_account_id = (m_prepared_char_select.value("account_id").toUInt());
    setLevel(*c,(uint8_t)m_prepared_char_select.value("char_level").toUInt());
    c->setName(STR_OR_EMPTY(m_prepared_char_select.value("char_name").toString()));
    cd->m_class_name = (STR_OR_EMPTY(m_prepared_char_select.value("archetype").toString()));
    cd->m_origin_name = (STR_OR_EMPTY(m_prepared_char_select.value("origin").toString()));
    cd->m_character_description = (STR_OR_EMPTY(m_prepared_char_select.value("description").toString()));
    cd->m_battle_cry = (STR_OR_EMPTY(m_prepared_char_select.value("battlecry").toString()));
    cd->m_experience_debt = (m_prepared_char_select.value("xp").toUInt());
    cd->m_experience_points = (m_prepared_char_select.value("xpdebt").toUInt());
    cd->m_influence = (m_prepared_char_select.value("inf").toUInt());
    c->m_current_attribs.m_HitPoints = (m_prepared_char_select.value("hitpoints").toUInt());
    c->m_current_attribs.m_Endurance = (m_prepared_char_select.value("endurance").toUInt());
    setMapName(*c,STR_OR_EMPTY(m_prepared_char_select.value("current_map").toString()));

    if(!m_prepared_char_select.value("title").isNull()
            || !m_prepared_char_select.value("badgetitle").isNull()
            || !m_prepared_char_select.value("specialtitle").isNull())
    {
        cd->m_has_titles = true;
        cd->m_titles[0] = (m_prepared_char_select.value("title").toString());
        cd->m_titles[1] = (m_prepared_char_select.value("badgetitle").toString());
        cd->m_titles[2] = (m_prepared_char_select.value("specialtitle").toString());
    }

    //cd->m_char_data.m_options = (m_prepared_char_select.value("options");
    //cd->m_char_data.m_gui = (m_prepared_char_select.value("gui");

    CharacterCostume *main_costume = new CharacterCostume;
    // appearance related.
    main_costume->m_body_type = m_prepared_char_select.value("bodytype").toUInt();
    setLastCostumeId(*c,m_prepared_char_select.value("last_costume_id").toUInt());
    main_costume->setSlotIndex(0);
    main_costume->setCharacterId(m_prepared_char_select.value("id").toULongLong());
    c->m_costumes.push_back(main_costume);
    return fill(main_costume);
}
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[1]))
#define DEFINED_ARRAYSIZE
#endif
bool CharacterDatabase::fill( CharacterCostume *c)
{
    assert(c&&c->getCharacterId());

    m_prepared_fill.bindValue(0,quint64(c->getCharacterId()));
    m_prepared_fill.bindValue(1,(uint16_t)c->getSlotIndex());
    if(!doIt(m_prepared_fill))
        return false;

    if(!m_prepared_fill.next()) // retry with the first one
    {
        m_prepared_fill.bindValue(0,quint64(c->getCharacterId()));
        m_prepared_fill.bindValue(1,0); // get first costume

        if(!doIt(m_prepared_fill))
            return false;
        if(!m_prepared_fill.next()) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill no costumes.\n")),false);
            return false;
        }
    }
    c->skin_color = m_prepared_fill.value("skin_color").toUInt();
    QString serialized_parts= m_prepared_fill.value("parts").toString();
    c->serializeFromDb(serialized_parts);
    c->m_non_default_costme_p = false;
    return true;
}

bool CharacterDatabase::CreateLinkedAccount( uint64_t auth_account_id,const std::string &username,int max_character_slots )
{
    //assert(auth_account_id>0);  sqlite3 autogenerated values start from 0
    assert(username.size()>2);
    m_prepared_account_insert.bindValue(0,quint64(auth_account_id));
    m_prepared_account_insert.bindValue(1,max_character_slots);

    if(!doIt(m_prepared_account_insert))
        return false;
    return true;
}

int CharacterDatabase::remove_account( uint64_t /*acc_serv_id*/ )
{
    return 0;
}
//TODO: SQL String sanitization
bool CharacterDatabase::create( uint64_t gid,uint8_t slot,Character *c )
{
    assert(m_db->driver()->hasFeature(QSqlDriver::LastInsertId));
    assert(gid>0);
    assert(c);
    assert(slot<8);

    CharacterData *cd = &c->m_char_data;
    QString timestamp;
    timestamp = QDateTime::currentDateTimeUtc().toString();

    Costume *cst = c->getCurrentCostume();
    if(!cst) {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) CharacterDatabase::create cannot insert char without costume.\n"))
                         ,false);
    }

    /*
    ":char_level, :slot_index, :account_id, :char_name, :archetype, :origin, :description, "
    ":battlecry, :current_map, :bodytype, :last_costume_id, :last_online, :hitpoints, :endurance, "
    ":inf, :xp, :xpdebt, :xppatrol, :alignment, :posx, :posy, :posz, :orientp, :orienty, :orientr, "
    ":title, :badgetitle, :specialtitle, :supergroup_id, :options, :gui"
    */

    m_prepared_char_insert.bindValue(":char_level", cd->m_level);
    m_prepared_char_insert.bindValue(":slot_index", uint32_t(slot));
    m_prepared_char_insert.bindValue(":account_id", quint64(gid));
    m_prepared_char_insert.bindValue(":char_name", c->m_name);
    m_prepared_char_insert.bindValue(":archetype", cd->m_class_name);
    m_prepared_char_insert.bindValue(":origin", cd->m_origin_name);
    m_prepared_char_insert.bindValue(":description", cd->m_character_description);
    m_prepared_char_insert.bindValue(":battlecry", cd->m_battle_cry);
    m_prepared_char_insert.bindValue(":current_map", cd->m_mapName);
    m_prepared_char_insert.bindValue(":bodytype", c->getCurrentCostume()->m_body_type);
    m_prepared_char_insert.bindValue(":last_costume_id", quint64(getLastCostumeId(*c)));
    m_prepared_char_insert.bindValue(":last_online", timestamp);
    m_prepared_char_insert.bindValue(":hitpoints", c->m_current_attribs.m_HitPoints);
    m_prepared_char_insert.bindValue(":endurance", c->m_current_attribs.m_Endurance);
    m_prepared_char_insert.bindValue(":inf", getInf(*c));
    m_prepared_char_insert.bindValue(":xp", getXP(*c));
    m_prepared_char_insert.bindValue(":xpdebt", cd->m_experience_debt);
    m_prepared_char_insert.bindValue(":xppatrol", cd->m_experience_patrol);
    m_prepared_char_insert.bindValue(":alignment", cd->m_alignment);
    m_prepared_char_insert.bindValue(":posx", 0);
    m_prepared_char_insert.bindValue(":posy", 0);
    m_prepared_char_insert.bindValue(":posz", 0);
    m_prepared_char_insert.bindValue(":orientp", 0);
    m_prepared_char_insert.bindValue(":orienty", 0);
    m_prepared_char_insert.bindValue(":orientr", 0);
    m_prepared_char_insert.bindValue(":title", cd->m_titles[0]);
    m_prepared_char_insert.bindValue(":badgetitle", cd->m_titles[1]);
    m_prepared_char_insert.bindValue(":specialtitle", cd->m_titles[2]);
    m_prepared_char_insert.bindValue(":supergroup_id", cd->m_supergroup_id);
    m_prepared_char_insert.bindValue(":options", 0);
    m_prepared_char_insert.bindValue(":gui", 0);

    if(!doIt(m_prepared_char_insert))
        return false;
    int64_t char_id = m_prepared_char_insert.lastInsertId().toLongLong();
    c->m_db_id = char_id;
    qDebug() << "char_id: " << char_id << ":" << c->m_db_id;

    // create costume
    QString costume_parts;
    cst->serializeToDb(costume_parts);
    m_prepared_costume_insert.bindValue(":id",quint64(char_id));
    m_prepared_costume_insert.bindValue(":costume_index",uint32_t(0));
    m_prepared_costume_insert.bindValue(":skin_color",uint32_t(cst->skin_color));
    m_prepared_costume_insert.bindValue(":parts",costume_parts);

    if(!doIt(m_prepared_costume_insert))
        return false;
    return true;
}
bool CharacterDatabase::update( Entity *e )
{
    assert(e);
    Character *c = &e->m_char;
    assert(c);
    CharacterData *cd = &c->m_char_data;
    QString timestamp;
    timestamp = QDateTime::currentDateTimeUtc().toString();

    Costume *cst = c->getCurrentCostume();
    if(!cst) {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) CharacterDatabase::update cannot update char without costume.\n"))
                         ,false);
    }

    /*
    ":char_level, :char_name, :archetype, :origin, :description, "
    ":battlecry, :current_map, :bodytype, :last_costume_id, :last_online, :hitpoints, :endurance, "
    ":inf, :xp, :xpdebt, :xppatrol, :alignment, :posx, :posy, :posz, :orientp, :orienty, :orientr, "
    ":title, :badgetitle, :specialtitle, :supergroup_id, :options, :gui"
    */
    
    m_prepared_char_update.bindValue(":id", c->m_db_id); // for WHERE statement only
    m_prepared_char_update.bindValue(":char_level", getLevel(*c));
    m_prepared_char_update.bindValue(":char_name", c->getName());
    m_prepared_char_update.bindValue(":archetype", getClass(*c));
    m_prepared_char_update.bindValue(":origin", getOrigin(*c));
    m_prepared_char_update.bindValue(":description", getDescription(*c));
    m_prepared_char_update.bindValue(":battlecry", getBattleCry(*c));
    m_prepared_char_update.bindValue(":current_map", getMapName(*c));
    m_prepared_char_update.bindValue(":bodytype", c->getCurrentCostume()->m_body_type);
    m_prepared_char_update.bindValue(":last_costume_id", quint64(getLastCostumeId(*c)));
    m_prepared_char_update.bindValue(":last_online", timestamp);
    m_prepared_char_update.bindValue(":hitpoints", getHP(*c));
    m_prepared_char_update.bindValue(":endurance", getEnd(*c));
    m_prepared_char_update.bindValue(":inf", getInf(*c));
    m_prepared_char_update.bindValue(":xp", getXP(*c));
    m_prepared_char_update.bindValue(":xpdebt", getDebt(*c));
    m_prepared_char_update.bindValue(":xppatrol", getPatrolXP(*c));
    m_prepared_char_update.bindValue(":alignment", getAlignment(*c));
    m_prepared_char_update.bindValue(":posx", e->pos.x);
    m_prepared_char_update.bindValue(":posy", e->pos.y);
    m_prepared_char_update.bindValue(":posz", e->pos.z);
    m_prepared_char_update.bindValue(":orientp", e->inp_state.m_orientation_pyr.p);
    m_prepared_char_update.bindValue(":orienty", e->inp_state.m_orientation_pyr.y);
    m_prepared_char_update.bindValue(":orientr", e->inp_state.m_orientation_pyr.r);
    m_prepared_char_update.bindValue(":title", cd->m_titles[0]);
    m_prepared_char_update.bindValue(":badgetitle", cd->m_titles[1]);
    m_prepared_char_update.bindValue(":specialtitle", cd->m_titles[2]);
    m_prepared_char_update.bindValue(":supergroup_id", cd->m_supergroup_id);
    //m_prepared_char_update.bindValue(":options", cd->m_options);
    //m_prepared_char_update.bindValue(":gui", cd->m_gui);
    
    if(!doIt(m_prepared_char_update))
        return false;

    // Update costume
    QString costume_parts;
    cst->serializeToDb(costume_parts);
    m_prepared_costume_update.bindValue(":id",c->m_db_id);
    m_prepared_costume_update.bindValue(":costume_index",uint32_t(0));
    m_prepared_costume_update.bindValue(":skin_color",uint32_t(cst->skin_color));
    m_prepared_costume_update.bindValue(":parts",costume_parts);

    if(!doIt(m_prepared_costume_update))
        return false;
    return true;
}
#ifdef DEFINED_ARRAYSIZE
#undef DEFINED_ARRAYSIZE
#undef ARRAYSIZE
#endif
