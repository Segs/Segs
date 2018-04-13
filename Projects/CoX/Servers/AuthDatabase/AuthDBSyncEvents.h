#pragma once

#include "Servers/InternalEvents.h"

#include <QDateTime>

enum AuthDBEventTypes : uint32_t
{
    evRetrieveAccountRequest = Internal_EventTypes::evLAST_EVENT,
    evRetrieveAccountResponse,
    evValidatePasswordRequest,
    evValidatePasswordResponse,
    evCreateAccount,
    evAuthDbError,
    evLAST_EVENT
};

#define ONE_WAY_MESSAGE(name)\
struct name ## Message final : public InternalEvent\
{\
    name ## Data m_data;\
    name ## Message(name ## Data &&d) :  InternalEvent(AuthDBEventTypes::ev ## name),m_data(d) {}\
};
/// A message without Request having additional data
#define SIMPLE_TWO_WAY_MESSAGE(name)\
struct name ## Request final : public InternalEvent\
{\
    name ## Message(uint64_t token) :  InternalEvent(AuthDBEventTypes::ev ## name ## Request) {session_token(token);}\
};\
struct name ## Response final : public InternalEvent\
{\
    name ## Data m_data;\
    name ## Response(name ## Data &&d,uint64_t token) :  InternalEvent(AuthDBEventTypes::ev ## name ## Response),m_data(d) {session_token(token);}\
};
/// A message with Request having additional data
#define TWO_WAY_MESSAGE(name)\
struct name ## Request final : public InternalEvent\
{\
    name ## RequestData m_data;\
    name ## Request(name ## RequestData &&d,uint64_t token) : InternalEvent(AuthDBEventTypes::ev ## name ## Request),m_data(d) {session_token(token);}\
};\
struct name ## Response final : public InternalEvent\
{\
    name ## ResponseData m_data;\
    name ## Response(name ## ResponseData &&d,uint64_t token) :  InternalEvent(AuthDBEventTypes::ev ## name ## Response),m_data(d) {session_token(token);}\
};



struct CreateAccountData
{
    QString username;
    QString password;
    int access_level;
};
ONE_WAY_MESSAGE(CreateAccount)
struct AuthDbErrorData
{
    QString message;
};
ONE_WAY_MESSAGE(AuthDbError)

struct RetrieveAccountResponseData
{
    uint64_t m_acc_server_acc_id;
    QString m_login;
    QDateTime m_creation_date;
    uint8_t m_access_level;
    void mark_as_missing() { m_acc_server_acc_id = 0; }
};
struct RetrieveAccountRequestData {
    QString  m_login;
    uint32_t m_id; // if this is 0, the lookup will be done by login, otherwise by id
};
TWO_WAY_MESSAGE(RetrieveAccount)

struct ValidatePasswordRequestData
{
    QString username;
    QString password;
};
struct ValidatePasswordResponseData
{
    bool m_valid_password;
};
TWO_WAY_MESSAGE(ValidatePassword)

#undef ONE_WAY_MESSAGE
#undef SIMPLE_TWO_WAY_MESSAGE
#undef TWO_WAY_MESSAGE
