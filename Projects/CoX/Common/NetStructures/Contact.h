/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <QString>

struct Contact
{
    QString m_name;
    QString m_display_name;

    // for scripting language access.
    std::string getName() const { return m_name.toStdString();}
    void setName(const char *n) { m_name = n; }
};
