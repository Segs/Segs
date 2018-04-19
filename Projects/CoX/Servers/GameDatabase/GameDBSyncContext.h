#pragma once
#include <memory>
class QSqlDatabase;
class QSqlQuery;

struct CharacterUpdateData;
struct CostumeUpdateData;
struct GameAccountRequestData;
struct GameAccountResponseData;
struct RemoveCharacterRequestData;
struct WouldNameDuplicateRequestData;
struct WouldNameDuplicateResponseData;
struct CreateNewCharacterRequestData;
struct CreateNewCharacterResponseData;
struct GetEntityRequestData;
struct GetEntityResponseData;
struct SetClientOptionsData;

///
/// \brief The DbSyncContext class is used as thread local storage for database related objects
///
class GameDbSyncContext
{
    static constexpr int required_db_version = 6;
    std::unique_ptr<QSqlDatabase> m_db;
    std::unique_ptr<QSqlQuery> m_prepared_char_update;
    std::unique_ptr<QSqlQuery> m_prepared_costume_update;
    std::unique_ptr<QSqlQuery> m_prepared_account_select;
    std::unique_ptr<QSqlQuery> m_prepared_account_insert;
    std::unique_ptr<QSqlQuery> m_prepared_entity_select;
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
    bool performUpdate(const CharacterUpdateData &data);
    bool performUpdate(const CostumeUpdateData &data);
    bool getAccount(const GameAccountRequestData &data,GameAccountResponseData &result);
    bool removeCharacter(const RemoveCharacterRequestData &data);
    bool checkNameClash(const WouldNameDuplicateRequestData &data,WouldNameDuplicateResponseData &result);
    bool createNewChar(const  CreateNewCharacterRequestData&data, CreateNewCharacterResponseData &result);
    bool getEntity(const  GetEntityRequestData&data, GetEntityResponseData &result);
    bool updateClientOptions(const SetClientOptionsData &data);
private:
    int64_t getDbVersion(QSqlDatabase &);
};
