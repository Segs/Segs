/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "CommonNetStructures.h"
#include "Contact.h"
#include "glm/vec3.hpp"
#include "cereal/cereal.hpp"
#include "Components/Logging.h"

class Task
{
public:
    //static const constexpr uint32_t class_version = 1;
    enum : uint32_t {class_version = 2};

    int             m_db_id;
    QString         m_description;
    QString         m_owner;
    QString         m_state;
    QString         m_detail;
    bool            m_is_complete = false;
    bool            m_in_progress_maybe = false;
    bool            m_is_abandoned = false;
    bool            m_has_location = false;
    bool            m_detail_invalid = true;
    bool            m_board_train = false;

    Destination     m_location;
    int             m_finish_time;

    //unknown
    int             m_unknown_1;
    int             m_unknown_2;


    //Not listed in Class but expected
    int             m_task_idx;

    // for scripting language access
    std::string getDescription() const { return m_description.toStdString();}
    void setDescription(const char *n) { m_description = n; }
    std::string getOwner() const { return m_owner.toStdString();}
    void setOwner(const char *n) { m_owner = n; }
    std::string getState() const { return m_state.toStdString();}
    void setState(const char *n) { m_state = n; }
    std::string getDetail() const { return m_detail.toStdString();}
    void setDetail(const char *n) { m_detail = n; }

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);

};

using vTaskList = std::vector<Task>;

class TaskEntry
{
public:
    //static const constexpr uint32_t class_version = 1;
    enum : uint32_t {class_version = 2};

    uint32_t m_db_id;
    vTaskList m_task_list;
    bool m_reset_selected_task = true;

    template<class Archive>
    void serialize(Archive &archive, uint32_t const version);
};

using vTaskEntryList = std::vector<TaskEntry>;

class TaskObjectiveTimer // Shouldn't serialze to DB?
{
public:
    QString m_message;
    float m_mission_time;
};
