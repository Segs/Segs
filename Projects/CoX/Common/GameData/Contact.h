/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <QString>

static QHash<QString,int> contactLinkHash = {
    {"CONTACTLINK_HELLO"                ,1},
    {"CONTACTLINK_MAIN"                 ,2},
    {"CONTACTLINK_BYE"                  ,3},
    {"CONTACTLINK_MISSIONS"             ,4},
    {"CONTACTLINK_LONGMISSION"          ,5},
    {"CONTACTLINK_SHORTMISSION"         ,6},
    {"CONTACTLINK_ACCEPTLONG"           ,7},
    {"CONTACTLINK_ACCEPTSHORT"          ,8},
    {"CONTACTLINK_INTRODUCE"            ,9},
    {"CONTACTLINK_INTRODUCE_CONTACT1"   ,0x0A},
    {"CONTACTLINK_INTRODUCE_CONTACT2"   ,0x0B},
    {"CONTACTLINK_ACCEPT_CONTACT2"      ,0x0D},
    {"CONTACTLINK_ACCEPT_CONTACT2"      ,0x0D},
    {"CONTACTLINK_GOTOSTORE"            ,0x0E},
    {"CONTACTLINK_TRAIN"                ,0x0F},
    {"CONTACTLINK_WRONGMODE"            ,0x10},
    {"CONTACTLINK_DONTKNOW"             ,0x11},
    {"CONTACTLINK_NOTLEADER"            ,0x12},
    {"CONTACTLINK_BADCELLCALL"          ,0x13},
    {"CONTACTLINK_ABOUT"                ,0x14},
    {"CONTACTLINK_IDENTIFYCLUE"         ,0x15},
    {"CONTACTLINK_NEWPLAYERTELEPORT_AP" ,0x16},
    {"CONTACTLINK_NEWPLAYERTELEPORT_GC" ,0x17},
    {"CONTACTLINK_FORMTASKFORCE"        ,0x18},
    {"CONTACTLINK_CHOOSE_TITLE"         ,0x19},
    {"CONTACTLINK_GOTOTAILOR"           ,0x1A},
};

struct Contact
{
    QString m_name;
    QString m_display_name;

    // for scripting language access.
    std::string getName() const { return m_name.toStdString();}
    void setName(const char *n) { m_name = n; }
};

struct ContactEntry
{
    QString     m_response_text; // char[100]
    uint32_t    m_link = 0;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_response_text);
        ar(m_link);
    }
};

struct ContactEntryBulk
{
    QString     m_msgbody; // char[20000]
    QVector<ContactEntry> m_responses; // must be size 11, or cannot exceed 11?
    // size_t num_active_contacts; // we can use size()
};
