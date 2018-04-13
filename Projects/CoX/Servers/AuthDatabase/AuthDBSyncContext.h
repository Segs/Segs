#pragma once
#include <memory>

class QSqlDatabase;
class QSqlQuery;
class QSqlError;

struct CreateAccountData;
struct RetrieveAccountRequestData;
struct RetrieveAccountResponseData;
struct ValidatePasswordRequestData;
struct ValidatePasswordResponseData;
///
/// \brief The AuthDbSyncContext class is used as thread local storage for auth database related objects
///
class AuthDbSyncContext
{
    static constexpr int required_db_version = 0;
    std::unique_ptr<QSqlDatabase> m_db;
    std::unique_ptr<QSqlQuery> m_add_account_query;
    std::unique_ptr<QSqlQuery> m_prepared_select_account_by_id;
    std::unique_ptr<QSqlQuery> m_prepared_select_account_passw;
    std::unique_ptr<QSqlQuery> m_prepared_select_account_by_username;
    std::unique_ptr<QSqlError> last_error;
    bool m_setup_complete = false;

public:
    AuthDbSyncContext();
    ~AuthDbSyncContext();
    QSqlError *getLastError() const { return last_error.get(); }
    bool loadAndConfigure();
    bool addAccount(const struct CreateAccountData &data);
    bool retrieveAccount(const RetrieveAccountRequestData &data, RetrieveAccountResponseData &result);
    bool getPasswordValidity(const ValidatePasswordRequestData &data, ValidatePasswordResponseData &result);
private:
    int64_t getDbVersion(QSqlDatabase &db);
};
