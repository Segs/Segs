/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2016 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
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
};
// Generates instances based on some kind of schema file
class GeneratedMapTemplate : public MapTemplate
{
public:
    GeneratedMapTemplate();
};
