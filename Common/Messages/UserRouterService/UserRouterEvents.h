/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Servers/InternalEvents.h"
#include "GameData/chardata_serializers.h"
#include "Messages/Map/MessageChannels.h"
#include "cereal/archives/memory_binary.hpp"

namespace SEGSEvents
{

    
enum UserRouterEventTypes : uint32_t
{
    evUserRouterInfoMessage = Internal_EventTypes::ID_LAST_Internal_EventTypes + 100,
    evUserRouterOpaqueRequest,
    evUserRouterOpaqueResponse,
	evUserRouterQueryRequest,
	evUserRouterQueryResponse,
};

enum class UserRouterError : uint32_t
{
	OK,
	USER_OFFLINE,
};

struct UserRouterInfoData
{
    QString m_message;
    MessageChannel m_info_channel;
    uint32_t m_sender_id;
    uint32_t m_target_id;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_message, m_info_channel, m_sender_id, m_target_id);
    }
};

//[[ev_def:macro]]
ONE_WAY_MESSAGE(UserRouterEventTypes,UserRouterInfo)

struct UserRouterOpaqueRequestData
{
    std::vector<uint8_t> m_payload;

    uint32_t m_sender_id;
	QString m_sender_name;

    std::vector<uint32_t>   m_target_id_list;
    std::vector<QString>    m_target_name_list;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_payload, m_sender_id, m_target_id_list, m_sender_name, m_target_name_list);
    }
};


struct UserRouterOpaqueResponseData
{
	UserRouterOpaqueRequestData m_req;
	UserRouterError m_error;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_req, m_error);
    }
};

//[[ev_def:macro]]
TWO_WAY_MESSAGE(UserRouterEventTypes,UserRouterOpaque)

struct UserRouterQueryRequestData
{
    uint32_t m_query_id;
	QString m_query_name;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_query_id, m_query_name);
    }
};

struct UserRouterQueryResponseData
{
    uint32_t m_request_id;
	QString m_request_name;

    uint32_t m_response_id;
	QString m_response_name;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_request_id, m_request_name, m_response_id, m_response_name);
    }
};

//[[ev_def:macro]]
TWO_WAY_MESSAGE(UserRouterEventTypes,UserRouterQuery)

} // end of namespace SEGSEvents
