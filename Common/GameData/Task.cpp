/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "Task.h"
#include "cereal/cereal.hpp"
#include "Components/Logging.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"

template<class Archive>
void Task::serialize(Archive &archive, uint32_t const version)
{
    if(version != Task::class_version)
    {
        qCritical() << "Failed to serialize Task, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("DbId",m_db_id));
    archive(cereal::make_nvp("Description",m_description));
    archive(cereal::make_nvp("Owner",m_owner));
    archive(cereal::make_nvp("State",m_state));
    archive(cereal::make_nvp("Detail",m_detail));
    archive(cereal::make_nvp("IsComplete",m_is_complete));
    archive(cereal::make_nvp("InProgressMaybe",m_in_progress_maybe));
    archive(cereal::make_nvp("IsAbandoned",m_is_abandoned));
    archive(cereal::make_nvp("HasLocation",m_has_location));
    archive(cereal::make_nvp("DetailInvalid",m_detail_invalid));
    archive(cereal::make_nvp("Location",m_location));
    archive(cereal::make_nvp("FinishTime",m_finish_time));
    archive(cereal::make_nvp("BoardTrain",m_board_train));

    //unknown
    archive(cereal::make_nvp("Unknown1",m_unknown_1));
    archive(cereal::make_nvp("Unknown2",m_unknown_2));


    //Not listed but expected
    archive(cereal::make_nvp("TaskIdx",m_task_idx));
}
CEREAL_CLASS_VERSION(Task, Task::class_version)  // register Task class version
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(Task)

template<class Archive>
void TaskEntry::serialize(Archive &archive, uint32_t const version)
{
   if(version != Task::class_version)
    {
        qCritical() << "Failed to serialize TaskEntry, incompatible serialization format version " << version;
        return;
    }

   archive(cereal::make_nvp("DbId",m_db_id));
   archive(cereal::make_nvp("TaskList",m_task_list));
   archive(cereal::make_nvp("resetSelectedTask",m_reset_selected_task));
}
CEREAL_CLASS_VERSION(TaskEntry, TaskEntry::class_version)  // register TaskEntry class version
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(TaskEntry)
