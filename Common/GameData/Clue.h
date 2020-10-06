/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <QString>
#include "cereal/cereal.hpp"
#include "Components/Logging.h"



class Clue
{
public:
     enum : uint32_t {class_version       = 1};

    QString m_name;
    QString m_display_name;
    QString m_detail_text;
    QString m_icon_file;

    // for scripting language access.
    std::string getName() const { return m_name.toStdString();}
    void setName(const char *n) { m_name = n; }

    std::string getDisplayName() const { return m_display_name.toStdString();}
    void setDisplayName(const char *n) { m_display_name = n; }

    std::string getDetailText() const { return m_detail_text.toStdString();}
    void setDetailText(const char *n) { m_detail_text = n; }

    std::string getIconFile() const { return m_icon_file.toStdString();}
    void setIconFile(const char *n) { m_icon_file = n; }


    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
};
using vClueList = std::vector<Clue>;


class Souvenir
{
public:
     enum : uint32_t {class_version       = 1};

    int32_t m_idx;
    QString m_name;
    QString m_icon;
    QString m_description;

    // for scripting language access.
       std::string getName() const { return m_name.toStdString();}
       void setName(const char *n) { m_name = n; }

       std::string getIcon() const { return m_icon.toStdString();}
       void setIcon(const char *n) { m_icon = n; }

       std::string getDescription() const { return m_description.toStdString();}
       void setDescription(const char *n) { m_description = n; }

       template<class Archive>
       void serialize(Archive &archive, uint32_t const version);

};

using vSouvenirList = std::vector<Souvenir>;
