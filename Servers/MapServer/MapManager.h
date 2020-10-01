/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <map>
#include <vector>
#include <stdint.h>
#include <QHash>

class MapTemplate;
class QString;
/**
  \class MapManger
  \brief Central map server class, responsible for map template management.
*/
class MapManager
{
    std::map<uint32_t,MapTemplate *> m_templates;
    QHash<QString,MapTemplate *> m_name_to_template;
    size_t                      m_max_instances; // how many maps can we instantiate
public:
                    MapManager();
                    ~MapManager();
    bool            load_templates(const QString &template_directory, uint8_t game_id, uint32_t map_id,
                                       const struct ListenAndLocationAddresses &loc);
    MapTemplate *   get_template(QString id);
    size_t          num_templates();
    size_t          max_instances();
    void            shut_down_all();
};
