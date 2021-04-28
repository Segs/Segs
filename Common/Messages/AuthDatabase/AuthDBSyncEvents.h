/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Common/Servers/InternalEvents.h"

#include <QDateTime>
namespace SEGSEvents
{

enum AuthDBEventTypes : uint32_t
{
    evRetrieveAccountRequest = Internal_EventTypes::ID_LAST_Internal_EventTypes,
    evRetrieveAccountResponse,
    evValidatePasswordRequest,
    evValidatePasswordResponse,
    evCreateAccountMessage,
    evAuthDbStatusMessage
};


struct CreateAccountData
{
    QString username;
    QString password;
    int access_level;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( username,password,access_level );
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(AuthDBEventTypes,CreateAccount)

struct AuthDbStatusData
{
    QString message;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( message );
    }
};
// [[ev_def:macro]]
ONE_WAY_MESSAGE(AuthDBEventTypes,AuthDbStatus)

struct RetrieveAccountResponseData
{
    static constexpr uint64_t INVALID_ACCOUNT_ID = 0;

    QString m_login;
    QDateTime m_creation_date;
    uint32_t m_acc_server_acc_id;
    uint8_t m_access_level;

    void mark_as_missing() { m_acc_server_acc_id = INVALID_ACCOUNT_ID; }
    bool valid() const { return m_acc_server_acc_id != INVALID_ACCOUNT_ID; }
    bool isBlocked() const { return m_access_level == INVALID_ACCOUNT_ID; }

    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_login,m_creation_date,m_acc_server_acc_id,m_access_level );
    }
};

struct RetrieveAccountRequestData
{
    QString  m_login;
    QString  m_password;
    uint32_t m_id; // if this is 0, the lookup will be done by login, otherwise by id
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_login,m_password,m_id );
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(AuthDBEventTypes,RetrieveAccount)

struct ValidatePasswordRequestData
{
    QString username;
    QString password;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( username,password );
    }
};

struct ValidatePasswordResponseData
{
    bool m_valid_password;
    template <class Archive>
    void serialize( Archive & ar )
    {
        ar( m_valid_password );
    }
};
// [[ev_def:macro]]
TWO_WAY_MESSAGE(AuthDBEventTypes,ValidatePassword)

} // end of SEGSSEvents namespace
