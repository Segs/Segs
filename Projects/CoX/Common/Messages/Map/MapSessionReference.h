#pragma once

#include <stdint.h>
struct MapClientSession;
struct SessionReference
{
    uint64_t session_token;
    mutable MapClientSession * m_client;
    MapClientSession * operator->() { return m_client; }
    MapClientSession * operator->() const { return m_client; }
    MapClientSession & operator*() { return *m_client; }
    MapClientSession & operator*() const { return *m_client; }
    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(session_token);
    }
};
