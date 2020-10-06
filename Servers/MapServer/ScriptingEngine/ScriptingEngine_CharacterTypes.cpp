/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "DataHelpers.h"
#include "Common/GameData/Character.h"
#include "Common/GameData/CharacterHelpers.h"
#include "Common/GameData/Contact.h"
#include "Common/GameData/Entity.h"
#include "Common/GameData/Contact.h"
#include "Common/GameData/Task.h"
#include "ScriptingEngine.h"
#include "ScriptingEnginePrivate.h"

#include "MapInstance.h"

#include <sol/sol.hpp>


void ScriptingEngine::register_CharacterTypes()
{
    m_private->m_lua.new_usertype<Clue>("Clue",
        sol::constructors<Clue()>(),
        "name", sol::property(&Clue::getName, &Clue::setName),
        "displayName", sol::property(&Clue::getDisplayName, &Clue::setDisplayName),
        "detail", sol::property(&Clue::getDetailText, &Clue::setDetailText),
        "iconFile", sol::property(&Clue::getIconFile, &Clue::setIconFile)
    );

    m_private->m_lua.new_usertype<Contact>( "Contact",
        // 3 constructors
        sol::constructors<Contact()>(),
        // gets or set the value using member variable syntax
        "name", sol::property(&Contact::getName, &Contact::setName),
        "locationDescription", sol::property(&Contact::getLocationDescription, &Contact::setLocationDescription),
        "npcId", &Contact::m_npc_id,
        "currentStanding", &Contact::m_current_standing,
        "contactIdx", &Contact::m_contact_idx,
        "notifyPlayer", &Contact::m_notify_player,
        "taskIndex", &Contact::m_task_index,
        "hasLocation", &Contact::m_has_location,
        "location", &Contact::m_location,
        "confidantThreshold", &Contact::m_confidant_threshold,
        "friendThreshold", &Contact::m_friend_threshold,
        "completeThreshold", &Contact::m_complete_threshold,
        "canUseCell", &Contact::m_can_use_cell,
        "contactId", &Contact::m_contact_idx,
        "dialogScreenIdx", &Contact::m_dlg_screen,
        "settingTitle", &Contact::m_setting_title
    );

    m_private->m_lua.new_usertype<Destination>("Destination",
        sol::constructors<Destination()>(),
        "pointIdx", &Destination::point_idx,
        "location", &Destination::location,
        "name", sol::property(&Destination::getLocationName, &Destination::setLocationName),
        "mapName", sol::property(&Destination::getLocationMapName, &Destination::setLocationMapName)
    );

    m_private->m_lua.new_usertype<HideAndSeek>( "HideAndSeek",
        sol::constructors<HideAndSeek()>(),
        "foundCount", &HideAndSeek::m_found_count
    );

    m_private->m_lua.new_usertype<Hunt>( "Hunt",
        sol::constructors<Hunt()>(),
        "type", sol::property(&Hunt::getTypeString, &Hunt::setTypeString),
        "count", &Hunt::m_count
    );


    m_private->m_lua.new_usertype<RelayRaceResult>( "RelayRaceResult",
        sol::constructors<RelayRaceResult()>(),
        "segment", &RelayRaceResult::m_segment,
        "lastTime", &RelayRaceResult::m_last_time,
        "bestTime", &RelayRaceResult::m_best_time
    );

    m_private->m_lua.new_usertype<Souvenir>("Souvenir",
        sol::constructors<Souvenir()>(),
        "name", sol::property(&Souvenir::getName, &Souvenir::setName),
        "description", sol::property(&Souvenir::getDescription, &Souvenir::setDescription),
        "icon", sol::property(&Souvenir::getIcon, &Souvenir::setIcon)
    );

    m_private->m_lua.new_usertype<TaskEntry>("TaskEntry",
          sol::constructors<TaskEntry()>(),
          "dbId", &TaskEntry::m_db_id,
          "taskList", &TaskEntry::m_task_list,
          "resetSelectedTask", &TaskEntry::m_task_list);

      m_private->m_lua.new_usertype<Task>("Task",
          sol::constructors<Task()>(),
          "dbId", &Task::m_db_id,
          "description", sol::property(&Task::getDescription, &Task::setDescription),
          "owner", sol::property(&Task::getOwner, &Task::setOwner),
          "detail", sol::property(&Task::getDetail, &Task::setDetail),
          "state", sol::property(&Task::getState, &Task::setState),
          "isComplete", &Task::m_is_complete,
          "inProgressMaybe", &Task::m_in_progress_maybe,
          "hasLocation", &Task::m_has_location,
          "isDetailInvalid", &Task::m_detail_invalid,
          "location", &Task::m_location,
          "finishTime", &Task::m_finish_time,
          "taskIdx", &Task::m_task_idx,
          "isAbandoned", &Task::m_is_abandoned,
          "unknownInt1", &Task::m_unknown_1,
          "unknownInt2", &Task::m_unknown_2,
          "boardTrain", &Task::m_board_train
      );

      // Player Object
      m_private->m_lua["Player"] = m_private->m_lua.create_table(); // Empty Table aka Object
      m_private->m_lua["Player"]["ClearTarget"] = [this]()
      {
          setTarget(*cl->m_ent, cl->m_ent->m_idx);
      };

      m_private->m_lua["Player"]["CloseContactDialog"] = [this]()
      {
          sendContactDialogClose(*cl);
      };

      m_private->m_lua["Player"]["SetDebt"] = [this](const uint32_t debt)
      {
          setDebt(*cl->m_ent->m_char, debt);
      };

      m_private->m_lua["Player"]["GiveDebt"] = [this](const int debt)
      {
          giveDebt(*cl, debt);
      };

      m_private->m_lua["Player"]["GiveEnhancement"] = [this](const char* name, int level)
      {
          QString e_name = QString::fromUtf8(name);
          giveEnhancement(*cl, e_name, level);
      };

      m_private->m_lua["Player"]["GiveEnd"] = [this](const float end)
      {
          giveEnd(*cl, end);
      };

      m_private->m_lua["Player"]["SetEnd"] = [this](const float end)
      {
          setEnd(*cl->m_ent->m_char, end);
      };

      m_private->m_lua["Player"]["SetHp"] = [this](const float hp)
      {
          setHP(*cl->m_ent->m_char, hp);
      };

      m_private->m_lua["Player"]["GiveHp"] = [this](const float hp)
      {
          giveHp(*cl, hp);
      };

      m_private->m_lua["Player"]["SetInf"] = [this](const uint32_t inf)
      {
          setInf(*cl->m_ent->m_char, inf);
      };

      m_private->m_lua["Player"]["GiveInf"] = [this](const int inf)
      {
          modifyInf(*cl, inf);
      };

      m_private->m_lua["Player"]["GiveInsp"] = [this](const char* name)
      {
          QString e_name = QString::fromUtf8(name);
          giveInsp(*cl, e_name);
      };

      m_private->m_lua["Player"]["SetXp"] = [this](const uint32_t xp)
      {
          setXP(*cl->m_ent->m_char, xp);
      };

      m_private->m_lua["Player"]["GiveXp"] = [this](const uint32_t xp)
      {
          giveXp(*cl, xp);
      };

      m_private->m_lua["Player"]["SendFloatingDamage"] = [this](const uint32_t tgt_idx, const int amount)
      {
          sendFloatingNumbers(*cl, tgt_idx, amount);
      };

      m_private->m_lua["Player"]["FaceEntity"] = [this](const uint32_t tgt_idx)
      {
          sendFaceEntity(*cl, tgt_idx);
      };

      m_private->m_lua["Player"]["FaceLocation"] = [this](glm::vec3 &loc)
      {
          sendFaceLocation(*cl, loc);
      };

      m_private->m_lua["Player"]["AddUpdateContact"] = [this](const Contact &contact)
      {
          updateContactStatusList(*cl, contact);
      };

      m_private->m_lua["Player"]["SetActiveDialogCallback"] = [this](std::function<void(int)> callback)
      {
          cl->m_ent->setActiveDialogCallback(callback);
      };

      m_private->m_lua["Player"]["RemoveContact"] = [this](const Contact &contact)
      {
          removeContact(*cl, contact);
      };

      m_private->m_lua["Player"]["UpdateTaskDetail"] = [this](const Task &task)
      {
          updateTaskDetail(*cl, task);
      };

      m_private->m_lua["Player"]["AddListOfTasks"] = [this](const sol::as_table_t<std::vector<Task>> task_list)
      {
          const auto& listMap = task_list.value();
          vTaskList listToSend;
          for (const auto& kvp : listMap)
          {
              listToSend.push_back(kvp);
          }
          addListOfTasks(cl, listToSend);
      };

      m_private->m_lua["Player"]["AddUpdateTask"] = [this](const Task &task)
      {
          sendUpdateTaskStatusList(*cl, task);
      };

      m_private->m_lua["Player"]["RemoveTask"] = [this](const Task &task)
      {
          removeTask(*cl, task);
      };

      m_private->m_lua["Player"]["SelectTask"] = [this](const Task &task)
      {
          selectTask(*cl, task);
      };

      m_private->m_lua["Player"]["StartMissionTimer"] = [this](const char* message, float timer)
      {
          QString mess = QString::fromUtf8(message);
          sendMissionObjectiveTimer(*cl, mess, timer);
      };

      m_private->m_lua["Player"]["SetWaypoint"] = [this](const int point_idx, glm::vec3 loc)
      {
          sendWaypoint(*cl, point_idx, loc);
      };

      m_private->m_lua["Player"]["LevelUp"] = [this]()
      {
          playerTrain(*cl);
      };

      m_private->m_lua["Player"]["OpenTitleMenu"] = [this]()
      {
          QString origin = getOriginTitle(*cl->m_ent->m_char);
          setTitle(*cl, origin);
      };

      m_private->m_lua["Player"]["GiveTempPower"] = [this](const char* power)
      {
          giveTempPower(cl, power);
      };

      m_private->m_lua["Player"]["AddClue"] = [this](const Clue clue)
      {
          addClue(*cl, clue);
      };

      m_private->m_lua["Player"]["RemoveClue"] = [this](const Clue clue)
      {
          removeClue(*cl, clue);
      };

      m_private->m_lua["Player"]["AddSouvenir"] = [this](const Souvenir souvenir)
      {
          addSouvenir(*cl, souvenir);
      };

      m_private->m_lua["Player"]["RemoveSouvenir"] = [this](const Souvenir souvenir)
      {
          removeSouvenir(*cl, souvenir);
      };

      m_private->m_lua["Player"]["Revive"] = [this](const int revive_lvl)
      {
          revive(cl, revive_lvl);
      };

      m_private->m_lua["Player"]["Respawn"] = [this](const char* loc_name)
      {
          respawn(*cl, loc_name);
      };

      m_private->m_lua["Player"]["AddHideAndSeekPoint"] = [this](int points)
      {
          addHideAndSeekResult(*cl, points);
      };

      m_private->m_lua["Player"]["AddRelayRaceResult"] = [this](RelayRaceResult *raceResult)
      {
          addRelayRaceResult(*cl, *raceResult);
      };

      m_private->m_lua["Player"]["GetRelayRaceResult"] = [this](int segment)
      {
          return getRelayRaceResult(*cl, segment);
      };

      m_private->m_lua["Player"]["GetLevel"] = [this]()
      {
        return getLevel(*cl->m_ent->m_char);
      };
}
