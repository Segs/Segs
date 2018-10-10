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

// Supergroups
struct CreateNewSuperGroupRequestData;
struct CreateNewSuperGroupResponseData;
struct GetSuperGroupRequestData;
struct GetSuperGroupResponseData;
struct SuperGroupNameDuplicateRequestData;
struct SuperGroupNameDuplicateResponseData;
struct SuperGroupUpdateData;
struct RemoveSuperGroupRequestData;

// Emails
struct EmailCreateRequestData;
struct EmailCreateResponseData;
struct EmailMarkAsReadData;
struct EmailUpdateOnCharDeleteData;
struct EmailRemoveData;
struct GetEmailRequestData;
struct GetEmailResponseData;
struct GetEmailsRequestData;
struct GetEmailsResponseData;
struct GetEmailBySenderIdRequestData;
struct GetEmailBySenderIdResponseData;
struct FillEmailRecipientIdRequestData;
struct FillEmailRecipientIdResponseData;
}
///
/// \brief The DbSyncContext class is used as thread local storage for database related objects
///
class GameDbSyncContext
{
    static constexpr int required_db_version = 8;
    std::unique_ptr<QSqlDatabase> m_db;
    std::unique_ptr<QSqlQuery> m_prepared_char_update;
    std::unique_ptr<QSqlQuery> m_prepared_costume_update;
    std::unique_ptr<QSqlQuery> m_prepared_player_update;
    std::unique_ptr<QSqlQuery> m_prepared_supergroup_update;
    std::unique_ptr<QSqlQuery> m_prepared_account_select;
    std::unique_ptr<QSqlQuery> m_prepared_account_insert;
    std::unique_ptr<QSqlQuery> m_prepared_entity_select;
    std::unique_ptr<QSqlQuery> m_prepared_entity_select_by_name;
    std::unique_ptr<QSqlQuery> m_prepared_get_char_slots;
    std::unique_ptr<QSqlQuery> m_prepared_char_insert;
    std::unique_ptr<QSqlQuery> m_prepared_char_exists;
    std::unique_ptr<QSqlQuery> m_prepared_char_delete;
    std::unique_ptr<QSqlQuery> m_prepared_char_select;
    std::unique_ptr<QSqlQuery> m_prepared_supergroup_insert;
    std::unique_ptr<QSqlQuery> m_prepared_supergroup_exists;
    std::unique_ptr<QSqlQuery> m_prepared_supergroup_delete;
    std::unique_ptr<QSqlQuery> m_prepared_supergroup_select;
    std::unique_ptr<QSqlQuery> m_prepared_options_update;
    std::unique_ptr<QSqlQuery> m_prepared_fill;
    std::unique_ptr<QSqlQuery> m_prepared_costume_insert;

    // email stuff
    std::unique_ptr<QSqlQuery> m_prepared_email_insert;
    std::unique_ptr<QSqlQuery> m_prepared_email_mark_as_read;
    std::unique_ptr<QSqlQuery> m_prepared_email_update_sender_id_on_char_delete;
    std::unique_ptr<QSqlQuery> m_prepared_email_update_recipient_id_on_char_delete;
    std::unique_ptr<QSqlQuery> m_prepared_email_delete;
    std::unique_ptr<QSqlQuery> m_prepared_email_select;
    std::unique_ptr<QSqlQuery> m_prepared_email_select_all;
    std::unique_ptr<QSqlQuery> m_prepared_email_fill_recipient_id;

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
    bool createNewChar(const  SEGSEvents::CreateNewCharacterRequestData &data, SEGSEvents::CreateNewCharacterResponseData &result);
    bool getEntity(const  SEGSEvents::GetEntityRequestData&data, SEGSEvents::GetEntityResponseData &result);
    bool getEntityByName(const SEGSEvents::GetEntityByNameRequestData &data, SEGSEvents::GetEntityByNameResponseData &result);
    bool updateClientOptions(const SEGSEvents::SetClientOptionsData &data);

    // Supergroups
    bool createNewSuperGroup(const  SEGSEvents::CreateNewSuperGroupRequestData &data, SEGSEvents::CreateNewSuperGroupResponseData &result);
    bool getSuperGroup(const SEGSEvents::GetSuperGroupRequestData &data,SEGSEvents::GetSuperGroupResponseData &result);
    bool checkNameClash(const SEGSEvents::SuperGroupNameDuplicateRequestData &data,SEGSEvents::SuperGroupNameDuplicateResponseData &result);
    bool performUpdate(const SEGSEvents::SuperGroupUpdateData &data);
    bool removeSuperGroup(const SEGSEvents::RemoveSuperGroupRequestData &data);

    // email stuff
    bool createEmail(const SEGSEvents::EmailCreateRequestData &data, SEGSEvents::EmailCreateResponseData &result);
    bool markEmailAsRead(const SEGSEvents::EmailMarkAsReadData &data);
    bool updateEmailOnCharDelete(const SEGSEvents::EmailUpdateOnCharDeleteData &data);
    bool deleteEmail(const SEGSEvents::EmailRemoveData &data);
    bool getEmail(const SEGSEvents::GetEmailRequestData &data, SEGSEvents::GetEmailResponseData &result);
    bool getEmails(const SEGSEvents::GetEmailsRequestData &data, SEGSEvents::GetEmailsResponseData &result);
    bool fillEmailRecipientId(const SEGSEvents::FillEmailRecipientIdRequestData &data, SEGSEvents::FillEmailRecipientIdResponseData &result);

private:
    int64_t getDbVersion(QSqlDatabase &);
};
