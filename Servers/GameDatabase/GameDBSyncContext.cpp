/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
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

#include "Messages/GameDatabase/GameDBSyncEvents.h"
#include "Components/Logging.h"
#include "Components/Settings.h"
#include "Database.h"
#include "Version.h"


#include <ace/Thread.h>

#include <QSqlDriver>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

using namespace SEGSEvents;

namespace
{
    bool prepQuery(QSqlQuery &qr,const QString &txt)
    {
        if(!qr.prepare(txt))
        {
            qDebug() << "SQL_ERROR:" << qr.lastError();
            qDebug() << "QUERY:" << qr.executedQuery();
            return false;
        }
        return true;
    }
    bool doIt(QSqlQuery &qr)
    {
        if(!qr.exec())
        {
            qDebug() << "SQL_ERROR:"<<qr.lastError();
            qDebug() << "QUERY:" << qr.executedQuery();
            return false;
        }
        return true;
    }
} // namespace

GameDbSyncContext::GameDbSyncContext() = default;
GameDbSyncContext::~GameDbSyncContext() = default;

int GameDbSyncContext::getDatabaseVersion(QSqlDatabase &db)
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
    {
        qCritical() << "No rows found when fetching database version.";
        return -1;
    }

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
        QString dbname = config.value(QStringLiteral("db_name"),"segs_game.db").toString();
        QString dbuser = config.value(QStringLiteral("db_user"),"segsadmin").toString();
        QString dbpass = config.value(QStringLiteral("db_pass"),"segs123").toString();
    config.endGroup(); // CharacterDatabase
    config.endGroup(); // AdminServer

    QSqlDatabase *db2;
    if(!driver_list.contains(dbdriver.toUpper()))
    {
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

    if(dbdriver == "QMYSQL")
    {
        db2->setConnectOptions("MYSQL_OPT_RECONNECT=true");
    }

    if(!m_db->open())
    {
        qCritical().noquote() << "Failed to open database:" << dbname;
        db2->setConnectOptions();
        return false;
    }

    int db_version = getDatabaseVersion(*m_db);
    int required_db_version = VersionInfo::getRequiredGameDBVersion();

    if(db_version != required_db_version)
    {
        // we should just stop the server, it isn't going to work anyway
        qFatal("Wrong database version (%d) Game database requires version: %d", db_version, required_db_version);
        db2->setConnectOptions();
        return false;
    }

    qCDebug(logDB) << "Preparing Queries...";
    m_prepared_account_insert = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_account_select = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_entity_select = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_entity_select_by_name = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_char_select = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_char_exists = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_char_insert = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_char_delete = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_char_update = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_costume_update = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_get_char_slots = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_options_update = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_player_update = std::make_unique<QSqlQuery>(*m_db);

    // emails
    m_prepared_email_insert = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_email_mark_as_read = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_email_update_sender_id_on_char_delete = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_email_update_recipient_id_on_char_delete = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_email_delete = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_email_select = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_email_select_all = std::make_unique<QSqlQuery>(*m_db);
    m_prepared_email_fill_recipient_id = std::make_unique<QSqlQuery>(*m_db);

    // Update prepQueries
    prepQuery(*m_prepared_char_update,
                "UPDATE characters SET "
                "char_name=:char_name, costume_data=:costume_data, "
                "chardata=:chardata, entitydata=:entitydata, player_data=:player_data, "
                "supergroup_id=:supergroup_id "
                "WHERE id=:id ");
    prepQuery(*m_prepared_player_update,
                "UPDATE characters SET "
                "player_data=:player_data "
                "WHERE id=:id ");
    prepQuery(*m_prepared_costume_update,
                "UPDATE characters SET "
                "costume_data=:costume_data "
                "WHERE id=:id ");
    prepQuery(*m_prepared_options_update,
                "UPDATE characters SET "
                "player_data=:player_data "
                "WHERE id=:id ");

    prepQuery(*m_prepared_account_select,"SELECT * FROM accounts WHERE id=?");
    prepQuery(*m_prepared_account_insert,"INSERT INTO accounts  (id,max_slots) VALUES (?,?)");
    prepQuery(*m_prepared_char_insert,
                "INSERT INTO characters  ("
                "slot_index, account_id, char_name, "
                "costume_data, chardata, entitydata, player_data, "
                "supergroup_id "
                ") VALUES ("
                ":slot_index, :account_id, :char_name, "
                ":costume_data, :chardata, :entitydata, :player_data, "
                ":supergroup_id "
                ")");
    prepQuery(*m_prepared_entity_select,"SELECT * FROM characters WHERE id=:id");
    prepQuery(*m_prepared_entity_select_by_name, "SELECT id FROM characters WHERE char_name=:char_name");
    prepQuery(*m_prepared_char_select,"SELECT * FROM characters WHERE account_id=? AND slot_index=?");
    prepQuery(*m_prepared_char_exists,"SELECT exists (SELECT 1 FROM characters WHERE char_name = ? LIMIT 1)");
    prepQuery(*m_prepared_char_delete,"DELETE FROM characters WHERE account_id=? AND slot_index=?");
    prepQuery(*m_prepared_get_char_slots,"SELECT slot_index FROM characters WHERE account_id=?");

    // email prepQueries
    prepQuery(*m_prepared_email_insert, "INSERT INTO emails (id, sender_id, recipient_id, email_data)"
                                        "VALUES (:id, :sender_id, :recipient_id, :email_data)");

    // new_id is either sender_id or 0 (deleted chara and/or account, could separate this tho)
    prepQuery(*m_prepared_email_mark_as_read, "UPDATE emails SET "
                                        "email_data=:email_data "
                                        "WHERE id=:id");

    prepQuery(*m_prepared_email_update_sender_id_on_char_delete, "UPDATE emails SET "
              "sender_id='0' WHERE sender_id=:deleted_id");
    prepQuery(*m_prepared_email_update_recipient_id_on_char_delete, "UPDATE emails SET "
              "recipient_id='0' WHERE recipient_id=:deleted_id");

    prepQuery(*m_prepared_email_delete, "DELETE FROM emails WHERE id=?");
    prepQuery(*m_prepared_email_select, "SELECT * FROM emails WHERE id=?");
    prepQuery(*m_prepared_email_select_all, "SELECT * FROM emails");
    prepQuery(*m_prepared_email_fill_recipient_id, "SELECT id FROM characters WHERE char_name=:recipient_name");

    return true;
}

bool GameDbSyncContext::performUpdate(const CharacterUpdateData &data)
{
    qCDebug(logDB) << "Attempting to update Character" << data.m_id;

    m_prepared_char_update->bindValue(QStringLiteral(":id"), data.m_id); // for WHERE statement only
    m_prepared_char_update->bindValue(QStringLiteral(":char_name"), data.m_char_name);
    m_prepared_char_update->bindValue(QStringLiteral(":costume_data"), data.m_costume_data);
    m_prepared_char_update->bindValue(QStringLiteral(":chardata"), data.m_char_data);
    m_prepared_char_update->bindValue(QStringLiteral(":entitydata"), data.m_entity_data);
    m_prepared_char_update->bindValue(QStringLiteral(":player_data"), data.m_player_data);
    m_prepared_char_update->bindValue(QStringLiteral(":supergroup_id"), data.m_supergroup_id);

   if(!doIt(*m_prepared_char_update))
        return false;

    qCDebug(logDB) << "Updating Character Successful" << data.m_char_name;
    return true;
}

bool GameDbSyncContext::performUpdate(const CostumeUpdateData &data)
{
    qCDebug(logDB) << "Attempting to update Player" << data.m_id;

    m_prepared_costume_update->bindValue(QStringLiteral(":id"), data.m_id);
    m_prepared_costume_update->bindValue(QStringLiteral(":costume_data"), data.m_costume_data);

    if(!doIt(*m_prepared_costume_update))
        return false;

    qCDebug(logDB) << "Updating Costume Successful" << data.m_id;
    return true;
}

bool GameDbSyncContext::performUpdate(const PlayerUpdateData &data)
{
    qCDebug(logDB) << "Attempting to update Player" << data.m_id;

    m_prepared_player_update->bindValue(QStringLiteral(":id"), data.m_id);
    m_prepared_player_update->bindValue(QStringLiteral(":player_data"), data.m_player_data);
    if(!doIt(*m_prepared_player_update))
        return false;

    qCDebug(logDB) << "Updating Player Successful" << data.m_id;
    return true;
}

bool GameDbSyncContext::getAccount(const GameAccountRequestData &data,GameAccountResponseData &result)
{
    qCDebug(logDB) << "Trying to find account in the database" << data.m_auth_account_id;
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
        character.m_slot_idx = idx++;
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
        character.m_name =  name.isEmpty() ? EMPTY_STRING : name;
        character.m_serialized_costume_data = m_prepared_char_select->value("costume_data").toString();
        character.m_serialized_chardata = m_prepared_char_select->value("chardata").toString();
        character.m_serialized_entity_data = m_prepared_char_select->value("entitydata").toString();
        character.m_serialized_player_data = m_prepared_char_select->value("player_data").toString();
        qCDebug(logDB) << "Serializing Character" << character.m_name;
    }

    qCDebug(logDB) << "Returning Account" << data.m_auth_account_id;
    return true;
}

bool GameDbSyncContext::removeCharacter(const RemoveCharacterRequestData &data)
{
    m_prepared_char_delete->bindValue(0,quint64(data.account_id));
    m_prepared_char_delete->bindValue(1,(uint32_t)data.slot_idx);
    if(!doIt(*m_prepared_char_delete))
        return false;

    qCDebug(logDB) << "Removing Character" << data.account_id << data.slot_idx;
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
    qCDebug(logDB) << "No name clash for" << data.m_name;
    return true;
}

bool GameDbSyncContext::createNewChar(const CreateNewCharacterRequestData &data, CreateNewCharacterResponseData &result)
{
    qCDebug(logDB) << "Creating new Character in GameDatabase";
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
    assert(m_db->driver()->hasFeature(QSqlDriver::LastInsertId));
    assert(data.m_slot_idx<8);

    m_prepared_char_insert->bindValue(":slot_index", uint32_t(selected_slot));
    m_prepared_char_insert->bindValue(":account_id", data.m_character.m_account_id);
    m_prepared_char_insert->bindValue(":char_name", data.m_character.m_name);
    m_prepared_char_insert->bindValue(":costume_data", data.m_character.m_serialized_costume_data);
    m_prepared_char_insert->bindValue(":chardata", data.m_character.m_serialized_chardata);
    m_prepared_char_insert->bindValue(":entitydata", data.m_ent_data);
    m_prepared_char_insert->bindValue(":player_data", data.m_character.m_serialized_player_data);
    m_prepared_char_insert->bindValue(":supergroup_id", 0);

    if(!doIt(*m_prepared_char_insert))
        return false;

    int64_t char_id = m_prepared_char_insert->lastInsertId().toLongLong();
    result.m_char_id = char_id;
    result.slot_idx = selected_slot;

    qCDebug(logDB) << "Inserted Character ID" << char_id << "at" << selected_slot;

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
    result.m_supergroup_id = m_prepared_entity_select->value("supergroup_id").toUInt();
    result.m_ent_data = m_prepared_entity_select->value("entitydata").toString();

    qCDebug(logDB) << "Returning Entity" << data.m_char_id;
    return true;
}

bool GameDbSyncContext::getEntityByName(const GetEntityByNameRequestData &data, GetEntityByNameResponseData &result)
{
    m_prepared_entity_select_by_name->bindValue(0, data.m_char_name);

    if(!doIt(*m_prepared_entity_select_by_name))
        return false;
    if(!m_prepared_entity_select_by_name->next())
        return false;

    result.m_supergroup_id = m_prepared_entity_select_by_name->value("supergroup_id").toUInt();
    result.m_ent_data = m_prepared_entity_select_by_name->value("entitydata").toString();

    qCDebug(logDB) << "Returning Entity" << data.m_char_name;

    return true;
}

// Update Client Options/Keybinds
bool GameDbSyncContext::updateClientOptions(const SetClientOptionsData &data)
{
    m_prepared_options_update->bindValue(":id", data.m_client_id); // for WHERE statement only
    m_prepared_options_update->bindValue(":options", data.m_options);
    m_prepared_options_update->bindValue(":keybinds", data.m_keybinds);
    if(!doIt(*m_prepared_options_update))
        return false;

    qCDebug(logDB) << "Updating Client Options Successful" << data.m_client_id;
    return true;
}

bool GameDbSyncContext::createEmail(const EmailCreateRequestData &data, EmailCreateResponseData &result)
{
    //DbTransactionGuard grd(*m_db);

    m_prepared_email_insert->bindValue(":sender_id", data.m_sender_id);
    m_prepared_email_insert->bindValue(":recipient_id", data.m_recipient_id);
    m_prepared_email_insert->bindValue(":email_data", data.m_email_data);

    if(!doIt(*m_prepared_email_insert))
        return false;

    assert(m_db->driver()->hasFeature(QSqlDriver::LastInsertId));

    result.m_email_id = m_prepared_email_insert->lastInsertId().toUInt();
    result.m_sender_id = data.m_sender_id;
    result.m_recipient_id = data.m_recipient_id;
    result.m_recipient_name = data.m_recipient_name;
    result.m_cerealized_email_data = data.m_email_data;

    // grd.commit();
    return true;
}

bool GameDbSyncContext::markEmailAsRead(const EmailMarkAsReadData &data)
{
    m_prepared_email_mark_as_read->bindValue(":id", data.m_email_id);
    m_prepared_email_mark_as_read->bindValue(":email_data", data.m_email_data);
    return doIt(*m_prepared_email_mark_as_read);
}

bool GameDbSyncContext::updateEmailOnCharDelete(const EmailUpdateOnCharDeleteData &data)
{
    m_prepared_email_update_sender_id_on_char_delete->bindValue(":deleted_id", data.m_deleted_char_id);
    m_prepared_email_update_recipient_id_on_char_delete->bindValue(":deleted_id", data.m_deleted_char_id);

    return doIt(*m_prepared_email_update_sender_id_on_char_delete) &&
            doIt(*m_prepared_email_update_recipient_id_on_char_delete);
}

bool GameDbSyncContext::deleteEmail(const EmailRemoveData &data)
{
    m_prepared_email_delete->bindValue(0, data.m_email_id);    
    return doIt(*m_prepared_char_delete);
}

bool GameDbSyncContext::getEmail(const GetEmailRequestData &data, GetEmailResponseData &result)
{
    m_prepared_email_select->bindValue(0, data.m_email_id);

    if(!doIt(*m_prepared_email_select))
        return false;
    if(!m_prepared_email_select->next())
        return false;

    result.m_email_id = m_prepared_email_select->value("id").toUInt();
    result.m_email_data = m_prepared_email_select->value("email_data").toString();

    return true;
}

// GetEmailsRequestData has 0 params
bool GameDbSyncContext::getEmails(const GetEmailsRequestData &/*data*/, GetEmailsResponseData &result)
{
    if(!doIt(*m_prepared_email_select_all))
        return false;

    while (m_prepared_email_select_all->next())
    {
        EmailResponseData emailResponseData;
        emailResponseData.m_email_id = m_prepared_email_select_all->value("id").toUInt();
        emailResponseData.m_sender_id = m_prepared_email_select_all->value("sender_id").toUInt();
        emailResponseData.m_recipient_id = m_prepared_email_select_all->value("recipient_id").toUInt();
        emailResponseData.m_cerealized_email_data = m_prepared_email_select_all->value("email_data").toString();
        result.m_email_response_datas.push_back(emailResponseData);
    }

    return true;
}

bool GameDbSyncContext::fillEmailRecipientId(const FillEmailRecipientIdRequestData &data, FillEmailRecipientIdResponseData &result)
{
    m_prepared_email_fill_recipient_id->bindValue(":recipient_name", data.m_recipient_name);

    if(!doIt(*m_prepared_email_fill_recipient_id))
        return false;
    if(!m_prepared_email_fill_recipient_id->next())
        return false;

    result.m_sender_id = data.m_sender_id;
    result.m_sender_name = data.m_sender_name;
    result.m_recipient_name = data.m_recipient_name;
    result.m_subject = data.m_subject;
    result.m_message = data.m_message;
    result.m_timestamp = data.m_timestamp;

    result.m_recipient_id = m_prepared_email_fill_recipient_id->value(0).toUInt();
    return true;
}

//! @}
