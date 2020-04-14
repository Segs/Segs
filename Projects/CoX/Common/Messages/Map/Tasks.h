/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "GameCommand.h"
#include "MapEventTypes.h"
#include "Common/GameData/Task.h"

namespace SEGSEvents
{
    // [[ev_def:type]]
    class TaskStatusList : public GameCommandEvent
    {
    public:
        // [[ev_def:field]
        vTaskEntryList m_task_entry_list;
        explicit TaskStatusList() : GameCommandEvent(MapEventTypes::evTaskStatusList){}
        TaskStatusList(vTaskEntryList task_entry_list) : GameCommandEvent(MapEventTypes::evTaskStatusList),
             m_task_entry_list(task_entry_list)
        {
        }

        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 45
            bs.StorePackedBits(1, m_task_entry_list.size());

            uint32_t loop_count_2 = 0;
            for(const TaskEntry &task_entry: m_task_entry_list)
            {

                bs.StorePackedBits(1, task_entry.m_db_id); // Player m_db_id?
                bs.StoreBits(1, task_entry.m_reset_selected_task);
                bs.StorePackedBits(1,  task_entry.m_task_list.size());
                bs.StorePackedBits(1,  task_entry.m_task_list.size());

                for(const Task &task : task_entry.m_task_list)
                {
                    bs.StorePackedBits(1, loop_count_2); // task index in array

                    //TaskStatusReceive
                    bs.StoreString(task.m_description);
                    bs.StoreString(task.m_owner);
                    bs.StorePackedBits(1,task.m_db_id);
                    bs.StorePackedBits(1, task.m_unknown_1);
                    bs.StorePackedBits(1, task.m_unknown_2);
                    bs.StoreString(task.m_state);
                    bs.StorePackedBits(1, task.m_finish_time);
                    bs.StoreBits(1, task.m_is_complete);
                    bs.StoreBits(1, task.m_in_progress_maybe);
                    bs.StoreBits(1, task.m_is_abandoned);
                    bs.StoreBits(1, task.m_board_train);
                    bs.StoreBits(1, task.m_has_location);

                    if(task.m_has_location)
                    {
                        bs.StoreFloat(task.m_location.location.x);
                        bs.StoreFloat(task.m_location.location.y);
                        bs.StoreFloat(task.m_location.location.z);
                        bs.StoreString(task.m_location.m_location_map_name);
                        bs.StoreString(task.m_location.m_location_name);
                    }

                     qCDebug(logTasks) << "taskEntry Task: " << loop_count_2 << " Entered ";
                    ++loop_count_2;
                }
            }
        }
        EVENT_IMPL(TaskStatusList)
    };


    // [[ev_def:type]]
    class TaskSelect : public GameCommandEvent
    {
    public:
        // [[ev_def:field]
        Task m_selected_task;
        explicit TaskSelect() : GameCommandEvent(MapEventTypes::evTaskSelect){}
        TaskSelect(Task task) : GameCommandEvent(MapEventTypes::evTaskSelect),
           m_selected_task(task){}

        void serializeto(BitStream &bs) const override
        {
            qCDebug(logMapEvents) << "Sending packet 46";
            bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 46
            qCDebug(logTasks) << "Sending m_selected_task.m_task_idx: " << m_selected_task.m_task_idx;
            bs.StorePackedBits(1, m_selected_task.m_task_idx);
        }

        EVENT_IMPL(TaskSelect)
    };

    // [[ev_def:type]]
    class TaskListRemoveTeammates : public GameCommandEvent
    {
    public:

        explicit TaskListRemoveTeammates() : GameCommandEvent(MapEventTypes::evTaskListRemoveTeammates){}
        void serializeto(BitStream &bs) const override
        {
            bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 47
        }

        EVENT_IMPL(TaskListRemoveTeammates)
    };

    // [[ev_def:type]]
    class TaskDetail : public GameCommandEvent
    {
    public:
        // [[ev_def:field]
        uint32_t m_db_id;
        // [[ev_def:field]
        uint32_t m_task_idx;
        // [[ev_def:field]
        QString m_task_detail;

        explicit TaskDetail() : GameCommandEvent(MapEventTypes::evTaskDetail){}
        TaskDetail(uint32_t db_id, uint32_t task_idx, QString task_detail) : GameCommandEvent(MapEventTypes::evTaskDetail)
        {
            m_db_id = db_id;
            m_task_idx = task_idx;
            m_task_detail = task_detail;
        }

        void serializeto(BitStream &bs) const override
        {
            qCDebug(logMapEvents) << "Sending packet 75";
            bs.StorePackedBits(1, type()-evFirstServerToClient); // packet 75
            bs.StorePackedBits(1, m_db_id);
            bs.StorePackedBits(1, m_task_idx);
            bs.StoreString(m_task_detail);
        }

        EVENT_IMPL(TaskDetail)

    };

    // [[ev_def:type]]
    class ReceiveTaskDetailRequest : public MapLinkEvent
    {
    public:
        // [[ev_def:field]]
        int32_t m_db_id = 0;
        // [[ev_def:field]]
        int32_t m_task_idx = 0;
        explicit ReceiveTaskDetailRequest() : MapLinkEvent(MapEventTypes::evReceiveTaskDetailRequest){}

        void serializeto(BitStream &/*bs*/) const final
        {
            assert(!"ReceiveContactStatus serializeto");
        }

        void serializefrom(BitStream &bs) final
        {
            m_db_id = bs.GetPackedBits(1);
            m_task_idx = bs.GetPackedBits(1);
            qCDebug(logMapEvents) << "ReceiveContactStatus Event";
        }

        EVENT_IMPL(ReceiveTaskDetailRequest)
    };

    // [[ev_def:type]]
    class MissionObjectiveTimer : public GameCommandEvent
    {
    public:
        // [[ev_def:field]]
        QString m_message;
        // [[ev_def:field]]
        float m_mission_time = 0;

        explicit MissionObjectiveTimer() : GameCommandEvent(MapEventTypes::evMissionObjectiveTimer){}
        MissionObjectiveTimer(QString message, float mission_time) : GameCommandEvent(MapEventTypes::evMissionObjectiveTimer){
            m_message = message;
            m_mission_time = mission_time;
        }

        void serializeto(BitStream &bs) const final // packet 73
        {
            bs.StorePackedBits(1, type()-evFirstServerToClient);
            bs.StoreString(m_message);
            bs.StoreFloat(m_mission_time);
        }

        void serializefrom(BitStream &/*bs*/) final
        {
            assert(!"SendMissionObjectTimer serializefrom");
        }

        EVENT_IMPL(MissionObjectiveTimer)
    };
}
