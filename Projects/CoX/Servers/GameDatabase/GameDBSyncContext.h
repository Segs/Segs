/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <memory>
class QSqlDatabase;
class QSqlQuery;

namespace SEGSEvents
{
struct CharacterUpdateData;
struct CostumeUpdateData;
struct PlayerUpdateData;
struct GameAccountRequestData;
struct GameAccountResponseData;
struct RemoveCharacterRequestData;
struct WouldNameDuplicateRequestData;
struct WouldNameDuplicateResponseData;
struct CreateNewCharacterRequestData;
struct CreateNewCharacterResponseData;
struct GetEntityRequestData;
struct GetEntityResponseData;
struct GetEntityByNameRequestData;
struct GetEntityByNameResponseData;
struct SetClientOptionsData;

}
///
/// \brief The DbSyncContext class is used as thread local storage for database related objects
///
class GameDbSyncContext
{
    static constexpr int required_db_version = 7;
    std::unique_ptr<QSqlDatabase> m_db;
    std::unique_ptr<QSqlQuery> m_prepared_char_update;
    std::unique_ptr<QSqlQuery> m_prepared_costume_update;
    std::unique_ptr<QSqlQuery> m_prepared_player_update;
    std::unique_ptr<QSqlQuery> m_prepared_account_select;
    std::unique_ptr<QSqlQuery> m_prepared_account_insert;
    std::unique_ptr<QSqlQuery> m_prepared_entity_select;
    std::unique_ptr<QSqlQuery> m_prepared_entity_select_by_name;
    std::unique_ptr<QSqlQuery> m_prepared_get_char_slots;
    std::unique_ptr<QSqlQuery> m_prepared_char_insert;
    std::unique_ptr<QSqlQuery> m_prepared_char_exists;
    std::unique_ptr<QSqlQuery> m_prepared_char_delete;
    std::unique_ptr<QSqlQuery> m_prepared_char_select;
    std::unique_ptr<QSqlQuery> m_prepared_options_update;
    std::unique_ptr<QSqlQuery> m_prepared_fill;
    std::unique_ptr<QSqlQuery> m_prepared_costume_insert;

    bool m_setup_complete = false;
public:
    GameDbSyncContext();
    ~GameDbSyncContext();
    bool loadAndConfigure();
    bool performUpdate(const SEGSEvents::CharacterUpdateData &data);
    bool performUpdate(const SEGSEvents::CostumeUpdateData &data);
    bool performUpdate(const SEGSEvents::PlayerUpdateData &data);
    bool performUpdate(const SEGSEvents::SetClientOptionsData &data);
    bool getAccount(const SEGSEvents::GameAccountRequestData &data,SEGSEvents::GameAccountResponseData &result);
    bool removeCharacter(const SEGSEvents::RemoveCharacterRequestData &data);
    bool checkNameClash(const SEGSEvents::WouldNameDuplicateRequestData &data,SEGSEvents::WouldNameDuplicateResponseData &result);
    bool createNewChar(const  SEGSEvents::CreateNewCharacterRequestData&data, SEGSEvents::CreateNewCharacterResponseData &result);
    bool getEntity(const  SEGSEvents::GetEntityRequestData&data, SEGSEvents::GetEntityResponseData &result);
    bool getEntityByName(const SEGSEvents::GetEntityByNameRequestData &data, SEGSEvents::GetEntityByNameResponseData &result);
    bool updateClientOptions(const SEGSEvents::SetClientOptionsData &data);
private:
    int64_t getDbVersion(QSqlDatabase &);
};
