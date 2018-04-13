#pragma once
#include <memory>
class QSqlDatabase;
class QSqlQuery;

///
/// \brief The DbSyncContext class is used as thread local storage for database related objects
///
class GameDbSyncContext
{
    static constexpr int required_db_version = 4;
    std::unique_ptr<QSqlDatabase> m_db;
    std::unique_ptr<QSqlQuery> m_prepared_char_update;
    std::unique_ptr<QSqlQuery> m_prepared_costume_update;
    std::unique_ptr<QSqlQuery> m_prepared_account_select;
    std::unique_ptr<QSqlQuery> m_prepared_account_insert;
    std::unique_ptr<QSqlQuery> m_prepared_entity_select;
    std::unique_ptr<QSqlQuery> m_prepared_char_insert;
    std::unique_ptr<QSqlQuery> m_prepared_char_exists;
    std::unique_ptr<QSqlQuery> m_prepared_char_delete;
    std::unique_ptr<QSqlQuery> m_prepared_char_select;
    std::unique_ptr<QSqlQuery> m_prepared_fill;
    std::unique_ptr<QSqlQuery> m_prepared_costume_insert;

    bool m_setup_complete = false;
public:
    GameDbSyncContext();
    ~GameDbSyncContext();
    bool loadAndConfigure();
    bool performUpdate(const struct CharacterUpdateData &data);
    bool performUpdate(const struct CostumeUpdateData &data);
    bool getAccount(const struct GameAccountRequestData &data,struct GameAccountResponseData &result);
    bool removeCharacter(const struct RemoveCharacterData &data);
private:
    int64_t getDbVersion(QSqlDatabase &);
};
