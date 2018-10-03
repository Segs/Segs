/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameDatabase Projects/CoX/Servers/GameDatabase
 * @{
 *
 * @image html dbschema/segs_game_dbschema.png
 */

#include <memory>

#include "GameDBSyncContext.h"

#include "GameDBSyncEvents.h"
#include "Settings.h"
#include "Database.h"

#include <ace/Thread.h>

#include <QDebug>
#include <QSettings>
#include <QSqlDriver>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

using namespace SEGSEvents;

namespace
{
    bool prepQuery(QSqlQuery &qr,const QString &txt) {
        if(!qr.prepare(txt))
        {
            qDebug() << "SQL_ERROR:"<<qr.lastError();
            return false;
        }
        return true;
    }
    bool doIt(QSqlQuery &qr) {
        if(!qr.exec())
        {
            qDebug() << "SQL_ERROR:"<<qr.lastError();
            return false;
        }
        return true;
    }
} // namespace

GameDbSyncContext::GameDbSyncContext() = default;
GameDbSyncContext::~GameDbSyncContext() = default;

int64_t GameDbSyncContext::getDbVersion(QSqlDatabase &db)
{
    QSqlQuery version_query(
                QStringLiteral("SELECT version FROM table_versions WHERE table_name='db_version' ORDER BY id DESC LIMIT 1"),
                db);
    if(!version_query.exec())
    {
        qDebug() << version_query.lastError();
        return -1;
    }
    if(!version_query.next())
        return -1;
    return version_query.value(0).toInt();
}

// Maybe one day we'll need to read different db configs per-thread, for now all just read the same file.
bool GameDbSyncContext::loadAndConfigure()
{
    char thread_name_buf[16];
    ACE_Thread_ID our_id;
    if(m_setup_complete)
    {
        qCritical()<< "This DbSyncContext has already been configured";
        return false;
    }

    qInfo() << "Loading GameDbSync settings...";
    QSettings config(Settings::getSettingsPath(),QSettings::IniFormat,nullptr);

    config.beginGroup(QStringLiteral("AdminServer"));
    QStringList driver_list {"QSQLITE", "QPSQL", "QMYSQL"};
    our_id.to_string(thread_name_buf); // Ace is using template specialization to acquire the lenght of passed buffer

    config.beginGroup(QStringLiteral("CharacterDatabase"));
    // this indent is here to mark the nesting of config block
        QString dbdriver = config.value(QStringLiteral("db_driver"),"QSQLITE").toString();
        QString dbhost = config.value(QStringLiteral("db_host"),"127.0.0.1").toString();
        int dbport = config.value(QStringLiteral("db_port"),"5432").toInt();
        QString dbname = config.value(QStringLiteral("db_name"),"segs_game").toString();
        QString dbuser = config.value(QStringLiteral("db_user"),"segsadmin").toString();
        QString dbpass = config.value(QStringLiteral("db_pass"),"segs123").toString();
    config.endGroup(); // CharacterDatabase
    config.endGroup(); // AdminServer

    QSqlDatabase *db2;
    if(!driver_list.contains(dbdriver.toUpper())) {
        qCritical() << "Database driver" << dbdriver << " not supported";
        return false;
    }
    db2 = new QSqlDatabase(QSqlDatabase::addDatabase(dbdriver,QStringLiteral("CharacterDatabase_")+thread_name_buf));
    db2->setHostName(dbhost);
    db2->setPort(dbport);
    db2->setDatabaseName(dbname);
    db2->setUserName(dbuser);
    db2->setPassword(dbpass);
    m_db.reset(db2); // at this point we become owner of the db
    if(!m_db->open())
    {
        qCritical().noquote() << "Failed to open database:" <<dbname;
        return false;
    }
    int db_version=getDbVersion(*m_db);
    if(db_version!=required_db_version)
    {
        qCritical() << "Wrong db version:" << db_version << "this GameDatabase service requires:" << required_db_version;
        return false;
    }
    m_prepared_fill = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_account_insert = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_account_select = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_entity_select = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_entity_select_by_name = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_char_select = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_char_exists = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_char_insert = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_char_delete = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_char_update = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_costume_insert = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_costume_update = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_get_char_slots = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_options_update = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_player_update = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_supergroup_insert = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_supergroup_select = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_supergroup_exists = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_supergroup_delete = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_supergroup_update = std::make_unique<QSqlQuery>(*m_db);

    prepQuery(*m_prepared_char_update,
              "UPDATE characters SET "
              "char_name=:char_name, chardata=:chardata, entitydata=:entitydata, bodytype=:bodytype, "
              "height=:height, physique=:physique,"
              "supergroup_id=:supergroup_id, player_data=:player_data "
              "WHERE id=:id ");
    prepQuery(*m_prepared_player_update,
              "UPDATE characters SET "
              "player_data=:player_data "
              "WHERE id=:id ");
    prepQuery(*m_prepared_costume_update,
              "UPDATE costume SET "
              "costume_index=:costume_index, skin_color=:skin_color, parts=:parts "
              "WHERE character_id=:id ");
    prepQuery(*m_prepared_options_update,
              "UPDATE characters SET "
              "player_data=:player_data "
              "WHERE id=:id ");

    prepQuery(*m_prepared_fill,"SELECT * FROM costume WHERE character_id=? AND costume_index=?");
    prepQuery(*m_prepared_account_select,"SELECT * FROM accounts WHERE id=?");
    prepQuery(*m_prepared_account_insert,"INSERT INTO accounts  (id,max_slots) VALUES (?,?)");
    prepQuery(*m_prepared_char_insert,
                "INSERT INTO characters  ("
                "slot_index, account_id, char_name, chardata, entitydata, "
                "bodytype, height, physique, "
                "supergroup_id, player_data"
                ") VALUES ("
                ":slot_index, :account_id, :char_name, :chardata, :entitydata, "
                ":bodytype, :height, :physique, "
                ":supergroup_id, :player_data"
                ")");
    prepQuery(*m_prepared_costume_insert,
                "INSERT INTO costume (character_id,costume_index,skin_color,parts) VALUES "
                "(:id,:costume_index,:skin_color,:parts)");
    prepQuery(*m_prepared_entity_select,"SELECT * FROM characters WHERE id=:id");
    prepQuery(*m_prepared_entity_select_by_name, "SELECT * FROM characters WHERE char_name=:char_name");
    prepQuery(*m_prepared_char_select,"SELECT * FROM characters WHERE account_id=? AND slot_index=?");
    prepQuery(*m_prepared_char_exists,"SELECT exists (SELECT 1 FROM characters WHERE char_name = ? LIMIT 1)");
    prepQuery(*m_prepared_char_delete,"DELETE FROM characters WHERE account_id=? AND slot_index=?");
    prepQuery(*m_prepared_get_char_slots,"SELECT slot_index FROM characters WHERE account_id=?");

    prepQuery(*m_prepared_supergroup_insert,
                "INSERT INTO supergroups (id, sg_name, sg_data, sg_members) "
                "VALUES (:sg_idx, :sg_name, :sg_data, :sg_members)");
    prepQuery(*m_prepared_supergroup_select,"SELECT * FROM supergroups WHERE id=?");
    prepQuery(*m_prepared_supergroup_exists,"SELECT exists (SELECT 1 FROM supergroups WHERE sg_name=? LIMIT 1)");
    prepQuery(*m_prepared_supergroup_delete,"DELETE FROM supergroups WHERE id=?");
    prepQuery(*m_prepared_supergroup_update,
                "UPDATE supergroups SET "
                "sg_name=:sg_name, sg_data=:sg_data, sg_members=:sg_members "
                "WHERE id=:sg_idx ");

    return true;
}

bool GameDbSyncContext::performUpdate(const CharacterUpdateData &data)
{
    m_prepared_char_update->bindValue(QStringLiteral(":id"), data.m_id); // for WHERE statement only
    m_prepared_char_update->bindValue(QStringLiteral(":char_name"), data.m_char_name);
    m_prepared_char_update->bindValue(QStringLiteral(":chardata"), data.m_char_data);
    m_prepared_char_update->bindValue(QStringLiteral(":entitydata"), data.m_entity_data);
    m_prepared_char_update->bindValue(QStringLiteral(":bodytype"), data.m_bodytype);
    m_prepared_char_update->bindValue(QStringLiteral(":height"), data.m_height);
    m_prepared_char_update->bindValue(QStringLiteral(":physique"), data.m_physique);
    m_prepared_char_update->bindValue(QStringLiteral(":supergroup_id"), data.m_supergroup_id);
    m_prepared_char_update->bindValue(QStringLiteral(":player_data"), data.m_player_data);
    return doIt(*m_prepared_char_update);
}

bool GameDbSyncContext::performUpdate(const CostumeUpdateData &data)
{
    m_prepared_costume_update->bindValue(QStringLiteral(":id"), uint32_t(data.m_db_id)); // for WHERE statement only
    m_prepared_costume_update->bindValue(QStringLiteral(":costume_index"), data.m_costume_index);
    m_prepared_costume_update->bindValue(QStringLiteral(":skin_color"), data.m_skin_color);
    m_prepared_costume_update->bindValue(QStringLiteral(":parts"), data.m_parts);
    return doIt(*m_prepared_costume_update);
}

bool GameDbSyncContext::performUpdate(const PlayerUpdateData &data)
{
    m_prepared_player_update->bindValue(QStringLiteral(":id"), data.m_id);
    m_prepared_player_update->bindValue(QStringLiteral(":player_data"), data.m_player_data);
    return doIt(*m_prepared_player_update);
}

bool GameDbSyncContext::getAccount(const GameAccountRequestData &data,GameAccountResponseData &result)
{
    // Try to find the acount in db
    m_prepared_account_select->bindValue(0,quint64(data.m_auth_account_id));
    if(!doIt(*m_prepared_account_select))
        return false;
    bool account_exists = m_prepared_account_select->next();
    if(!account_exists && !data.create_if_does_not_exist)
        return false;

    if(!account_exists && data.create_if_does_not_exist)
    {
        m_prepared_account_insert->bindValue(0,quint64(data.m_auth_account_id));
        m_prepared_account_insert->bindValue(1,data.max_character_slots);
        if(!doIt(*m_prepared_account_insert))
            return false;
        m_prepared_account_select->bindValue(0,quint64(data.m_auth_account_id));
        if(!doIt(*m_prepared_account_select))
            return false;
        if(!m_prepared_account_select->next() && !data.create_if_does_not_exist)
            return false;
    }
    result.m_game_server_acc_id = data.m_auth_account_id;
    result.m_max_slots = m_prepared_account_select->value("max_slots").toUInt();
    result.m_characters.resize(result.m_max_slots);
    int idx=0;
    m_prepared_char_select->bindValue(0,quint64(result.m_game_server_acc_id));
    for(auto &character : result.m_characters)
    {
        m_prepared_char_select->bindValue(1,uint16_t(idx));
        character.index = idx++;
        if(!doIt(*m_prepared_char_select))
            return false;
        if(!m_prepared_char_select->next())
        {
            character.reset(); // empty slot
            continue;
        }
        character.m_db_id = (m_prepared_char_select->value("id").toUInt());
        character.m_account_id = (m_prepared_char_select->value("account_id").toUInt());
        QString name=m_prepared_char_select->value("char_name").toString();
        character.m_name =  name.isEmpty() ? "EMPTY" : name;
        character.m_serialized_chardata = m_prepared_char_select->value("chardata").toString();
        character.m_serialized_player_data = m_prepared_char_select->value("player_data").toString();
        character.m_serialized_entity_data = m_prepared_char_select->value("entitydata").toString();

        GameAccountResponseCostumeData costume;
        // appearance related.

        costume.m_body_type = m_prepared_char_select->value("bodytype").toUInt();
        costume.m_height = m_prepared_char_select->value("height").toFloat();
        costume.m_physique = m_prepared_char_select->value("physique").toFloat();
        costume.m_slot_index = 0;
        costume.m_character_id = character.m_db_id;

        m_prepared_fill->bindValue(0,quint64(character.m_db_id));
        m_prepared_fill->bindValue(1,(uint16_t)costume.m_slot_index);
        if(!doIt(*m_prepared_fill))
            continue;

        if(!m_prepared_fill->next()) // retry with the first one
        {
            m_prepared_fill->bindValue(1,0); // get first costume

            if(!doIt(*m_prepared_fill))
                continue;
            if(!m_prepared_fill->next()) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT ("(%P|%t) GameDbSyncContext::getAccount no costumes.\n")),false);
                continue;
            }
        }
        costume.skin_color = m_prepared_fill->value("skin_color").toUInt();
        costume.m_serialized_data = m_prepared_fill->value("parts").toString();
        character.m_costumes.emplace_back(costume);

    }
    return true;
}

bool GameDbSyncContext::removeCharacter(const RemoveCharacterRequestData &data)
{
    m_prepared_char_delete->bindValue(0,quint64(data.account_id));
    m_prepared_char_delete->bindValue(1,(uint32_t)data.slot_idx);
    if(!doIt(*m_prepared_char_delete))
        return false;
    return true;
}

bool GameDbSyncContext::checkNameClash(const WouldNameDuplicateRequestData &data, WouldNameDuplicateResponseData &result)
{
    m_prepared_char_exists->bindValue(0,data.m_name);
    if(!doIt(*m_prepared_char_exists))
        return false;
    if(!m_prepared_char_exists->next())
        return false;
    // TODO: handle case of multiple accounts with same name ?
    result.m_would_duplicate = m_prepared_char_exists->value(0).toBool();
    return true;
}

bool GameDbSyncContext::createNewChar(const CreateNewCharacterRequestData &data, CreateNewCharacterResponseData &result)
{
    DbTransactionGuard grd(*m_db);
    m_prepared_get_char_slots->bindValue(0,data.m_client_id);
    if(!doIt(*m_prepared_get_char_slots))
        return false;

    std::set<int> slots_in_use;
    while(m_prepared_get_char_slots->next())
    {
        slots_in_use.insert(m_prepared_get_char_slots->value(0).toInt());
    }
    if(slots_in_use.size()>=data.m_max_allowed_slots)
    {
        result.slot_idx = -1;
        return true;
    }
    int selected_slot = data.m_slot_idx;
    if(selected_slot<0 || slots_in_use.find(selected_slot)!= slots_in_use.end())
    {
        // selecting first slot available
        for(int i=0;i<data.m_max_allowed_slots; ++i)
        {
            if(slots_in_use.find(i)==slots_in_use.end())
            {
                selected_slot = i;
                break;
            }
        }
    }
    const GameAccountResponseCostumeData & costume(data.m_character.current_costume());
    assert(m_db->driver()->hasFeature(QSqlDriver::LastInsertId));
    assert(data.m_slot_idx<8);
    //cd->m_last_online = QDateTime::currentDateTimeUtc().toString();
    m_prepared_char_insert->bindValue(":slot_index", uint32_t(selected_slot));
    m_prepared_char_insert->bindValue(":account_id", data.m_character.m_account_id);
    m_prepared_char_insert->bindValue(":char_name", data.m_character.m_name);
    m_prepared_char_insert->bindValue(":bodytype", costume.m_body_type);
    m_prepared_char_insert->bindValue(QStringLiteral(":height"), costume.m_height);
    m_prepared_char_insert->bindValue(QStringLiteral(":physique"), costume.m_physique);
    m_prepared_char_insert->bindValue(":supergroup_id", 0);
    m_prepared_char_insert->bindValue(":player_data", data.m_character.m_serialized_player_data);
    m_prepared_char_insert->bindValue(":entitydata", data.m_ent_data);
    m_prepared_char_insert->bindValue(":chardata", data.m_character.m_serialized_chardata);

    if(!doIt(*m_prepared_char_insert))
        return false;
    int64_t char_id = m_prepared_char_insert->lastInsertId().toLongLong();
    result.m_char_id = char_id;
    result.slot_idx = selected_slot;
    // create costume
    m_prepared_costume_insert->bindValue(":id",quint64(char_id));
    m_prepared_costume_insert->bindValue(":costume_index",uint32_t(0));
    m_prepared_costume_insert->bindValue(":skin_color",uint32_t(costume.skin_color));
    m_prepared_costume_insert->bindValue(":parts",costume.m_serialized_data);

    if(!doIt(*m_prepared_costume_insert))
        return false;
    grd.commit();
    return true;
}

bool GameDbSyncContext::getEntity(const GetEntityRequestData &data, GetEntityResponseData &result)
{
    m_prepared_entity_select->bindValue(0, data.m_char_id);
    if(!doIt(*m_prepared_entity_select))
        return false;
    if(!m_prepared_entity_select->next())
        return false;
    result.m_ent_data = m_prepared_entity_select->value("entitydata").toString();
    return true;
}

bool GameDbSyncContext::getEntityByName(const GetEntityByNameRequestData &data, GetEntityByNameResponseData &result)
{
    m_prepared_entity_select_by_name->bindValue(0, data.m_char_name);

    if(!doIt(*m_prepared_entity_select_by_name))
        return false;
    if(!m_prepared_entity_select_by_name->next())
        return false;

    result.m_ent_data = m_prepared_entity_select_by_name->value("entitydata").toString();
    return true;
}

// Update Client Options/Keybinds
bool GameDbSyncContext::updateClientOptions(const SetClientOptionsData &data)
{
    m_prepared_options_update->bindValue(":id", data.m_client_id); // for WHERE statement only
    m_prepared_options_update->bindValue(":options", data.m_options);
    m_prepared_options_update->bindValue(":keybinds", data.m_keybinds);
    if (!doIt(*m_prepared_options_update))
        return false;
    return true;
}

// SuperGroups
bool GameDbSyncContext::createNewSuperGroup(const CreateNewSuperGroupRequestData &data, CreateNewSuperGroupResponseData &result)
{
    DbTransactionGuard grd(*m_db);

    m_prepared_supergroup_insert->bindValue(QStringLiteral(":sg_name"), data.m_sg_name);
    m_prepared_supergroup_insert->bindValue(QStringLiteral(":sg_data"), data.m_serialized_sg_data);
    m_prepared_supergroup_insert->bindValue(QStringLiteral(":sg_members"), data.m_serialized_sg_members);

    if(!doIt(*m_prepared_supergroup_insert))
        return false;

    assert(m_db->driver()->hasFeature(QSqlDriver::LastInsertId));
    uint32_t sg_id = m_prepared_supergroup_insert->lastInsertId().toUInt();
    result.m_sg_id = sg_id; // return sg_id here, so that every sg has unique idx

    grd.commit();
    return true;
}

bool GameDbSyncContext::getSuperGroup(const GetSuperGroupRequestData &data, GetSuperGroupResponseData &result)
{
    // Try to find the supergroup in db
    m_prepared_supergroup_select->bindValue(0, data.m_sg_id);
    if(!doIt(*m_prepared_supergroup_select))
        return false;
    bool supergroup_exists = m_prepared_supergroup_select->next();
    if(!supergroup_exists && !data.create_if_does_not_exist)
        return false;

    if(!supergroup_exists && data.create_if_does_not_exist)
    {
        m_prepared_supergroup_insert->bindValue(QStringLiteral(":sg_idx"), data.m_sg_id);
        m_prepared_supergroup_insert->bindValue(QStringLiteral(":sg_name"), data.m_sg_name);
        if(!doIt(*m_prepared_supergroup_insert))
            return false;

        m_prepared_supergroup_select->bindValue(0, data.m_sg_id);
        if(!doIt(*m_prepared_supergroup_select))
            return false;
        if(!m_prepared_supergroup_select->next() && !data.create_if_does_not_exist)
            return false;
    }

    result.m_sg_id = data.m_sg_id;
    QString name = m_prepared_supergroup_select->value("sg_name").toString();
    result.m_sg_name =  name.isEmpty() ? "EMPTY" : name;
    result.m_serialized_sg_data = m_prepared_supergroup_select->value("sg_data").toString();
    result.m_serialized_sg_members = m_prepared_supergroup_select->value("sg_members").toString();

    return true;
}

bool GameDbSyncContext::checkNameClash(const SuperGroupNameDuplicateRequestData &data, SuperGroupNameDuplicateResponseData &result)
{
    m_prepared_supergroup_exists->bindValue(0,data.m_sg_name);
    if(!doIt(*m_prepared_supergroup_exists))
        return false;

    if(!m_prepared_supergroup_exists->next())
        return false;

    result.m_supergroup_duplicate = m_prepared_supergroup_exists->value(0).toBool();
    return true;
}

bool GameDbSyncContext::performUpdate(const SuperGroupUpdateData &data)
{
    m_prepared_supergroup_update->bindValue(QStringLiteral(":sg_idx"), data.m_sg_id);
    m_prepared_supergroup_update->bindValue(QStringLiteral(":sg_name"), data.m_sg_name);
    m_prepared_supergroup_update->bindValue(QStringLiteral(":sg_data"), data.m_serialized_sg_data);
    m_prepared_supergroup_update->bindValue(QStringLiteral(":sg_members"), data.m_serialized_sg_members);
    return doIt(*m_prepared_supergroup_update);
}

bool GameDbSyncContext::removeSuperGroup(const RemoveSuperGroupRequestData &data)
{
    m_prepared_supergroup_delete->bindValue(0,data.m_sg_id);
    if(!doIt(*m_prepared_supergroup_delete))
        return false;

    return true;
}

//! @}
