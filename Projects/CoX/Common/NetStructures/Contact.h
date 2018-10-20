/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "CommonNetStructures.h"
#include "glm/vec3.hpp"
#include "cereal/cereal.hpp"
#include "Logging.h"

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

struct Destination // aka waypoint
{
  public:
    static const constexpr uint32_t class_version = 1;

    int point_idx = 0;
    glm::vec3 location;
    QString         m_location_name;
    QString         m_location_map_name;

        template<class Archive>
        void serialize(Archive &archive, uint32_t const version)
        {
            if(version != Destination::class_version)
            {
                qCritical() << "Failed to serialize Destination, incompatible serialization format version " << version;
                return;
            }

        archive(cereal::make_nvp("PointIdx",point_idx));
        archive(cereal::make_nvp("Location",location));
        archive(cereal::make_nvp("LocationName",m_location_name));
        archive(cereal::make_nvp("LocationMapName",m_location_map_name));

        }
};


class Contact
{
public:
    static const constexpr uint32_t class_version = 1;

    // for scripting language access.
    std::string getName() const { return m_name.toStdString();}
    void setName(const char *n) { m_name = n; }

    std::string getLocationDescription() const { return m_location_description.toStdString();}
    void setLocationDescription(const char *n) { m_location_description = n; }

       QString         m_name;
       QString         m_location_description;
       uint32_t        m_npc_id;
       uint8_t         m_contact_idx;                   // most likey
       uint8_t         m_current_standing;
       uint8_t         m_confidant_threshold;
       uint8_t         m_friend_threshold;
       uint8_t         m_complete_threshold;
       uint8_t         m_task_index            = 0;
       bool            m_notify_player         = false;
       bool            m_can_use_cell          = false;
       bool            m_has_location          = false;
       Destination     m_location;



       template<class Archive>
       void serialize(Archive &archive, uint32_t const version)
       {
           if(version != Contact::class_version)
           {
               qCritical() << "Failed to serialize Destination, incompatible serialization format version " << version;
               return;
           }

       archive(cereal::make_nvp("Name",m_name));
       archive(cereal::make_nvp("LocationDescription",m_location_description));
       archive(cereal::make_nvp("npcId",m_npc_id));
       archive(cereal::make_nvp("ContactIdx",m_contact_idx));
       archive(cereal::make_nvp("CurrentStanding",m_current_standing));
       archive(cereal::make_nvp("ConfidantThreshold",m_confidant_threshold));
       archive(cereal::make_nvp("FriendThreshold",m_friend_threshold));
       archive(cereal::make_nvp("CompleteThreshold",m_complete_threshold));
       archive(cereal::make_nvp("TaskIndex",m_task_index));
       archive(cereal::make_nvp("NotifyPlayer",m_notify_player));
       archive(cereal::make_nvp("CanUseCell",m_can_use_cell));
       archive(cereal::make_nvp("HasLocation",m_has_location));
       archive(cereal::make_nvp("Location",m_location));
       }
};
using vContactList = std::vector<Contact>;

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

struct CharacterData;

void addContact(CharacterData &cd, Contact &contact);
