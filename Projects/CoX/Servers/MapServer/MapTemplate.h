/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "Servers/ServerEndpoint.h"

#include <QString>
#include <vector>

class MapInstance;

// TODO: instances should be uniquely identifiable, so getting a Client/Team specific instances is possible
class MapTemplate
{
static  uint8_t                     s_template_id;
        std::vector<MapInstance *>  m_instances; // switch from vector to priority queue ?
        QString                     m_map_filename;
        uint8_t                     m_game_server_id;
        uint32_t                    m_map_server_id;
        ListenAndLocationAddresses  m_base_loc;

public:
                                    MapTemplate(const QString &template_filename,
                                                uint8_t game_server_id, uint32_t map_server_id,
                                                const ListenAndLocationAddresses &loc);
        MapInstance *               get_instance(); //! If there are no instances, starts a new one.
        size_t                      num_instances();
        void                        shut_down_all();
        QString                     client_filename() const;
        QString                     base_name() const;
};

// Generates instances based on some kind of schema file
class GeneratedMapTemplate : public MapTemplate
{
public:
    GeneratedMapTemplate();
};
