/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <memory>
#include <array>

class QSqlDatabase;
class QSqlQuery;
class QSqlError;
class QString;

namespace SEGSEvents
{
struct CreateAccountData;
struct RetrieveAccountRequestData;
struct RetrieveAccountResponseData;
struct ValidatePasswordRequestData;
struct ValidatePasswordResponseData;
}

///
/// \brief The AuthDbSyncContext class is used as thread local storage for auth database related objects
///
class AuthDbSyncContext
{
public:
	enum QueryId {
		ID_FETCH_DB_VERSION_QUERY=0,
		ID_ADD_ACCOUNT_QUERY,
		ID_SELECT_ACCOUNT_BY_USERNAME_QUERY,
		ID_SELECT_ACCOUNT_BY_ID_QUERY,
		ID_SELECT_ACCOUNT_PASSWORD_QUERY,
		QUERY_COUNT
	};
private:
	std::array<std::unique_ptr<QSqlQuery>,QUERY_COUNT> m_query_mapping;
	std::unique_ptr<QSqlDatabase> m_db;
	std::unique_ptr<QSqlError> last_error;
    bool m_setup_complete = false;

public:
    AuthDbSyncContext();
    ~AuthDbSyncContext();
    QSqlError *getLastError() const { return last_error.get(); }
    bool loadAndConfigure();
    bool addAccount(const SEGSEvents::CreateAccountData &data);
    bool retrieveAccountAndCheckPassword(const SEGSEvents::RetrieveAccountRequestData &data,SEGSEvents::RetrieveAccountResponseData &result);
//    bool retrieveAccount(const RetrieveAccountRequestData &data, RetrieveAccountResponseData &result);
    bool getPasswordValidity(const SEGSEvents::ValidatePasswordRequestData &data, SEGSEvents::ValidatePasswordResponseData &result);
protected:
    bool checkPassword(const QString &login, const QString &password);
};
