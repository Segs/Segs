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
#include "GameData/CoHMath.h"
#include "MapServer/DataHelpers.h"
#include "GameData/entitydata_serializers.h"
#include "GameData/chardata_serializers.h"
#include "GameData/clientoptions_serializers.h"
#include "GameData/gui_serializers.h"
#include "GameData/keybind_serializers.h"
#include "Logging.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtCore/QDebug>
#include <QtSql/QSqlError>
#include <glm/ext.hpp> // currently only needed for qDebug()
#include <QDateTime>
#include <cstdlib>
#include <cstdio>
using namespace std;
QMutex g_mutex;

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
    QMutexLocker locker(&g_mutex);
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
    m_prepared_entity_select = QSqlQuery(*db);
    m_prepared_char_select = QSqlQuery(*db);
    m_prepared_char_update = QSqlQuery(*db);
    m_prepared_options_update = QSqlQuery(*db);
    m_prepared_gui_update = QSqlQuery(*db);
    m_prepared_costume_update = QSqlQuery(*db);
    m_prepared_account_select = QSqlQuery(*db);
    m_prepared_char_exists = QSqlQuery(*db);
    m_prepared_char_delete = QSqlQuery(*db);


    prepQuery(m_prepared_fill,"SELECT * FROM costume WHERE character_id=? AND costume_index=?");
    prepQuery(m_prepared_account_insert,"INSERT INTO accounts  (account_id,max_slots) VALUES (?,?)");
    prepQuery(m_prepared_char_insert,
                "INSERT INTO characters  ("
                "slot_index, account_id, char_name, chardata, entitydata, bodytype, "
                "hitpoints, endurance, supergroup_id, options, gui, keybinds"
                ") VALUES ("
                ":slot_index, :account_id, :char_name, :chardata, :entitydata, :bodytype, "
                ":hitpoints, :endurance, :supergroup_id, :options, :gui, :keybinds"
                ")");
    prepQuery(m_prepared_costume_insert,
                "INSERT INTO costume (character_id,costume_index,skin_color,parts) VALUES "
                "(:id,:costume_index,:skin_color,:parts)");
    prepQuery(m_prepared_char_update,
                "UPDATE characters SET "
                "char_name=:char_name, chardata=:chardata, entitydata=:entitydata, bodytype=:bodytype, "
                "hitpoints=:hitpoints, endurance=:endurance, supergroup_id=:supergroup_id "
                "WHERE id=:id ");
    prepQuery(m_prepared_options_update,
                "UPDATE characters SET "
                "options=:options, gui=:gui, keybinds=:keybinds "
                "WHERE id=:id ");
    prepQuery(m_prepared_gui_update,
                "UPDATE characters SET "
                "gui=:gui "
                "WHERE id=:id ");
    prepQuery(m_prepared_costume_update,
                "UPDATE costume SET "
                "costume_index=:costume_index, skin_color=:skin_color, parts=:parts "
                "WHERE character_id=:id ");
    prepQuery(m_prepared_entity_select,"SELECT * FROM characters WHERE id=:id");
    prepQuery(m_prepared_char_select,"SELECT * FROM characters WHERE account_id=? AND slot_index=?");
    prepQuery(m_prepared_account_select,"SELECT * FROM accounts WHERE account_id=?");
    prepQuery(m_prepared_char_exists,"SELECT exists (SELECT 1 FROM characters WHERE char_name = $1 LIMIT 1)");
    prepQuery(m_prepared_char_delete,"DELETE FROM characters WHERE account_id=? AND slot_index=?");
}


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

    qCDebug(logDB) << "CharacterClient id:" << c->account_server_id();
    qCDebug(logDB) << "CharacterClient slots:" << c->max_slots();

    return true;
}
#define STR_OR_EMPTY(c) ((!c.isEmpty()) ? c:"EMPTY")
#define STR_OR_VERY_EMPTY(c) ((c!=0) ? c:"")
bool CharacterDatabase::fill( Entity *e)
{
    assert(e);
    EntityData *ed = &e->m_entity_data;
    qCDebug(logDB).noquote() << e->m_db_id;

    m_prepared_entity_select.bindValue(":id",quint64(e->m_char->m_db_id));
    if(!doIt(m_prepared_entity_select))
        return false;

    if(!m_prepared_entity_select.next()) // retry with the first one
    {
        m_prepared_entity_select.bindValue(":id",quint64(e->m_char->m_db_id));
        if(!doIt(m_prepared_entity_select))
            return false;
        if(!m_prepared_entity_select.next()) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) CharacterDatabase::fill no matching entity.\n")),false);
            return false;
        }
    }

    e->m_db_id = (m_prepared_entity_select.value("id").toUInt());
    e->m_supergroup.m_SG_id = (m_prepared_entity_select.value("supergroup_id").toUInt());

    QString entity_data;
    entity_data = (m_prepared_entity_select.value("entitydata").toString());
    serializeFromDb(*ed,entity_data);

    // Can't pass direction through cereal, so let's update it here.
    e->m_direction = fromCoHYpr(ed->m_orientation_pyr);

    qCDebug(logDB).noquote() << entity_data;
    qCDebug(logOrientation).noquote() << glm::to_string(ed->m_orientation_pyr).c_str();
    qCDebug(logOrientation).noquote() << glm::to_string(e->m_direction).c_str();
    return true;
}
bool CharacterDatabase::fill( Character *c)
{
    CharacterData &cd(c->m_char_data);
    ClientOptions &od(c->m_options);
    KeybindSettings &kbd(c->m_keybinds);
    GUISettings &gui(c->m_gui);

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
    c->m_account_id = (m_prepared_char_select.value("account_id").toUInt());
    c->setName(STR_OR_EMPTY(m_prepared_char_select.value("char_name").toString()));
    c->m_current_attribs.m_HitPoints = (m_prepared_char_select.value("hitpoints").toUInt());
    c->m_current_attribs.m_Endurance = (m_prepared_char_select.value("endurance").toUInt());

    QString char_data;
    char_data = (m_prepared_char_select.value("chardata").toString());
    serializeFromDb(cd,char_data);

    QString options_data;
    options_data = (m_prepared_char_select.value("options").toString());
    serializeFromDb(od,options_data);

    QString gui_data;
    gui_data = (m_prepared_char_select.value("gui").toString());
    serializeFromDb(gui,gui_data);

    QString keybind_data;
    keybind_data = (m_prepared_char_select.value("keybinds").toString());
    serializeFromDb(kbd,keybind_data);

    qCDebug(logDB).noquote() << "m_db_id:" << c->m_db_id;
    qCDebug(logDB).noquote() << char_data;
    qCDebug(logDB).noquote() << options_data;
    qCDebug(logDB).noquote() << gui_data;
    qCDebug(logDB).noquote() << keybind_data;

    c->m_costumes.emplace_back();
    CharacterCostume *main_costume = &c->m_costumes.back();
    // appearance related.
    main_costume->m_body_type = m_prepared_char_select.value("bodytype").toUInt();
    main_costume->setSlotIndex(0);
    main_costume->setCharacterId(m_prepared_char_select.value("id").toULongLong());
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

bool CharacterDatabase::create(uint64_t gid, uint8_t slot, Entity *e)
{
    assert(m_db->driver()->hasFeature(QSqlDriver::LastInsertId));
    assert(e && e->m_char);

    EntityData *ed = &e->m_entity_data;
    Character &c(*e->m_char);

    assert(gid>0);
    assert(slot<8);

    CharacterData *cd = &c.m_char_data;
    cd->m_last_online = QDateTime::currentDateTimeUtc().toString();

    ClientOptions &od(c.m_options);
    KeybindSettings &kbd(c.m_keybinds);
    GUISettings &gui(c.m_gui);
    const CharacterCostume *cst = c.getCurrentCostume();
    if(!cst) {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) CharacterDatabase::create cannot insert char without costume.\n"))
                         ,false);
    }

    /*
    ":slot_index, :account_id, :char_name, :chardata, :entitydata, :bodytype, "
    ":hitpoints, :endurance, :supergroup_id, :options, :gui, :keybinds"
    */
    m_prepared_char_insert.bindValue(":slot_index", uint32_t(slot));
    m_prepared_char_insert.bindValue(":account_id", quint64(gid));
    m_prepared_char_insert.bindValue(":char_name", c.m_name);
    m_prepared_char_insert.bindValue(":bodytype", c.getCurrentCostume()->m_body_type);
    m_prepared_char_insert.bindValue(":hitpoints", c.m_current_attribs.m_HitPoints);
    m_prepared_char_insert.bindValue(":endurance", c.m_current_attribs.m_Endurance);
    m_prepared_char_insert.bindValue(":supergroup_id", uint32_t(e->m_supergroup.m_SG_id));

    QString entity_data;
    serializeToDb(*ed,entity_data);
    m_prepared_char_insert.bindValue(":entitydata", entity_data);

    QString char_data;
    serializeToDb(*cd,char_data);
    m_prepared_char_insert.bindValue(":chardata", char_data);

    QString options_data;
    serializeToDb(od,options_data);
    m_prepared_char_insert.bindValue(":options", options_data);

    QString gui_data;
    serializeToDb(gui,gui_data);
    m_prepared_char_insert.bindValue(":gui", gui_data);

    QString keybind_data;
    serializeToDb(kbd,keybind_data);
    m_prepared_char_insert.bindValue(":keybinds", keybind_data);

    qCDebug(logDB).noquote() << entity_data;
    qCDebug(logDB).noquote() << char_data;
    qCDebug(logDB).noquote() << options_data;
    qCDebug(logDB).noquote() << gui_data;
    qCDebug(logDB).noquote() << keybind_data;

    if(!doIt(m_prepared_char_insert))
        return false;
    int64_t char_id = m_prepared_char_insert.lastInsertId().toLongLong();
    c.m_db_id = char_id;

    qCDebug(logDB) << "char_id: " << char_id << ":" << c.m_db_id;

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
    assert(e->m_char!=nullptr);
    EntityData *ed = &e->m_entity_data;
    Character &c(*e->m_char);
    CharacterData *cd = &c.m_char_data;
    cd->m_last_online = QDateTime::currentDateTimeUtc().toString();

    const CharacterCostume *cst = c.getCurrentCostume();
    if(!cst) {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) CharacterDatabase::update cannot update char without costume.\n"))
                         ,false);
    }

    /*
    ":id, :char_name, :chardata, :entitydata, :bodytype, :hitpoints, :endurance, :supergroup_id "
    */
    m_prepared_char_update.bindValue(":id", uint32_t(c.m_db_id)); // for WHERE statement only
    m_prepared_char_update.bindValue(":char_name", c.getName());
    m_prepared_char_update.bindValue(":bodytype", c.getCurrentCostume()->m_body_type);
    m_prepared_char_update.bindValue(":hitpoints", getHP(c));
    m_prepared_char_update.bindValue(":endurance", getEnd(c));
    m_prepared_char_update.bindValue(":supergroup_id", uint32_t(e->m_supergroup.m_SG_id));

    QString entity_data;
    serializeToDb(*ed,entity_data);
    m_prepared_char_update.bindValue(":entitydata", entity_data);

    QString char_data;
    serializeToDb(*cd,char_data);
    m_prepared_char_update.bindValue(":chardata", char_data);

    // Update Client Options/Keybinds
    if(!updateClientOptions(e))
        qDebug() << "Client Options failed to update in database!";

    if(!updateGUISettings(e))
        qDebug() << "Client GUISettings failed to update in database!";

    qCDebug(logDB).noquote() << entity_data;
    qCDebug(logDB).noquote() << char_data;

    if(!doIt(m_prepared_char_update))
        return false;

    // Update costume
    QString costume_parts;
    cst->serializeToDb(costume_parts);
    m_prepared_costume_update.bindValue(":id",c.m_db_id);
    m_prepared_costume_update.bindValue(":costume_index",uint32_t(0));
    m_prepared_costume_update.bindValue(":skin_color",uint32_t(cst->skin_color));
    m_prepared_costume_update.bindValue(":parts",costume_parts);

    if(!doIt(m_prepared_costume_update))
        return false;
    return true;
}
// Update Client Options/Keybinds
bool CharacterDatabase::updateClientOptions( Entity *e )
{
    assert(e);
    assert(e->m_char!=nullptr);
    Character &c = *e->m_char;
    ClientOptions &od(c.m_options);
    KeybindSettings &kbd(c.m_keybinds);

    /*
    ":id, :options, :keybinds "
    */
    m_prepared_options_update.bindValue(":id", uint32_t(c.m_db_id)); // for WHERE statement only

    QString options_data;
    serializeToDb(od,options_data);
    m_prepared_options_update.bindValue(":options", options_data);

    QString keybind_data;
    serializeToDb(kbd,keybind_data);
    m_prepared_options_update.bindValue(":keybinds", keybind_data);

    qCDebug(logDB).noquote() << options_data;
    qCDebug(logDB).noquote() << keybind_data;

    if(!doIt(m_prepared_options_update))
        return false;
    return true;
}
// Update Client GUI settings
bool CharacterDatabase::updateGUISettings( Entity *e )
{
    assert(e);
    assert(e->m_char!=nullptr);
    Character &c = *e->m_char;
    GUISettings &gui(c.m_gui);

    /*
    ":id, :gui "
    */
    m_prepared_gui_update.bindValue(":id", uint32_t(c.m_db_id)); // for WHERE statement only

    QString gui_data;
    serializeToDb(gui,gui_data);
    m_prepared_gui_update.bindValue(":gui", gui_data);

    qCDebug(logDB).noquote() << gui_data;

    if(!doIt(m_prepared_gui_update))
        return false;
    return true;
}
// Query by character name or db_id
CharacterFromDB * CharacterDatabase::getCharacter(int32_t db_id)
{
    CharacterFromDB ent;

    m_prepared_entity_select.bindValue(":id",quint64(db_id));
    if(!doIt(m_prepared_entity_select))
        return nullptr;

    if(!m_prepared_entity_select.next()) // retry with the first one
    {
        m_prepared_entity_select.bindValue(":id",quint64(db_id));
        if(!doIt(m_prepared_entity_select))
            return nullptr;
        if(!m_prepared_entity_select.next()) {
            qCDebug(logDB) << "Cannot find Character with ID" << db_id << "in database.";
        }
    }

    ent.name = (STR_OR_EMPTY(m_prepared_entity_select.value("char_name").toString()));
    ent.hitpoints = (m_prepared_entity_select.value("hitpoints").toUInt());
    ent.endurance = (m_prepared_entity_select.value("endurance").toUInt());
    ent.sg_id = (m_prepared_entity_select.value("supergroup_id").toUInt());

    QString char_data;
    char_data = (m_prepared_entity_select.value("chardata").toString());
    serializeFromDb(ent.char_data,char_data);

    QString entity_data;
    entity_data = (m_prepared_entity_select.value("entitydata").toString());
    serializeFromDb(ent.entity_data,entity_data);

    return &ent;
}

CharacterFromDB * CharacterDatabase::getCharacter(const QString &name)
{
    m_prepared_char_exists.bindValue(0,name);
    if(!doIt(m_prepared_char_exists))
        return nullptr;

    if(!m_prepared_char_exists.next()) // retry with the first one
    {
        m_prepared_char_exists.bindValue(0,name);
        if(!doIt(m_prepared_char_exists))
            return nullptr;
        if(!m_prepared_char_exists.next()) {
            qCDebug(logDB) << "Cannot find Character" << name << "in database.";
            return nullptr;
        }
    }

    uint32_t db_id = (m_prepared_char_select.value("id").toUInt());
    return getCharacter(db_id);
}

#ifdef DEFINED_ARRAYSIZE
#undef DEFINED_ARRAYSIZE
#undef ARRAYSIZE
#endif
