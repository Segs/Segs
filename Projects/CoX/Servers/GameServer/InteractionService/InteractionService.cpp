/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "InteractionService.h"
#include "GameData/Entity.h"
#include "GameData/Character.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/EntityHelpers.h"
#include "GameData/CharacterHelpers.h"
#include "Messages/Map/MapEvents.h"
#include "Messages/Map/ContactList.h"
#include "Messages/Map/Tasks.h"
#include "Messages/Map/ClueList.h"
#include "Servers/MapServer/DataHelpers.h"
#include <QtCore/QDebug>

using namespace SEGSEvents;

std::unique_ptr<ServiceToClientData> InteractionService::on_entity_info_request(Entity* ent, Event* ev)
{
    EntityInfoRequest* casted_ev = static_cast<EntityInfoRequest *>(ev);
    Entity *tgt = getEntity(ent->m_client, casted_ev->entity_idx);

    if(tgt == nullptr)
    {
        qCDebug(logMapEvents) << "No target active, doing nothing";
        return nullptr;
    }

    QString description = getDescription(*tgt->m_char);

    GameCommandVector commands;
    commands.emplace_back(std::make_unique<EntityInfoResponse>(description));

    qCDebug(logDescription) << "Entity info requested" << casted_ev->entity_idx << description;
    return std::make_unique<ServiceToClientData>(ent, std::move(commands), QString());
}

std::unique_ptr<ServiceToClientData> InteractionService::on_interact_with(Entity* ent, Event *ev)
{
    InteractWithEntity* casted_ev = static_cast<InteractWithEntity *>(ev);
    // Entity *entity = getEntity(ent->m_client, casted_ev->m_srv_idx);

    qCDebug(logMapEvents) << "Entity: " << ent->m_idx << "wants to interact with" << casted_ev->m_srv_idx;
    ScriptingServiceToClientData* scriptData = new ScriptingServiceToClientData();
    scriptData->flags |= uint32_t(ScriptingServiceFlags::CallFuncWithClientContext);
    scriptData->funcName = "entity_interact";
    scriptData->intArg = casted_ev->m_srv_idx;
    scriptData->locArg = ent->m_entity_data.m_pos;

    ScriptVector scripts {scriptData};
    return std::make_unique<ServiceToClientData>(ent, scripts, QString());
}

std::unique_ptr<ServiceToClientData> InteractionService::on_receive_contact_status(Entity* ent, Event* ev)
{
    ReceiveContactStatus* casted_ev = static_cast<ReceiveContactStatus* >(ev);

    qCDebug(logMapEvents) << "ReceiveContactStatus Entity: " << ent->m_idx << "wants to interact with" << casted_ev->m_srv_idx;
    ScriptingServiceToClientData* scriptData = new ScriptingServiceToClientData();
    scriptData->flags |= uint32_t(ScriptingServiceFlags::CallFuncWithClientContext);
    scriptData->funcName = "contact_call";
    scriptData->intArg = casted_ev->m_srv_idx;

    ScriptVector scripts {scriptData};
    return std::make_unique<ServiceToClientData>(ent, scripts, QString());
}

std::unique_ptr<ServiceToClientData> InteractionService::on_receive_task_detail_request(Entity* ent, Event *ev)
{
    ReceiveTaskDetailRequest* casted_ev = static_cast<ReceiveTaskDetailRequest *>(ev);

    qCDebug(logMapEvents) << "ReceiveTaskDetailRequest Entity: " << ent->m_idx << "wants detail for task " << casted_ev->m_task_idx;
    QString detail = "Testing Task Detail Request";

    TaskDetail test_task;
    test_task.m_task_idx = casted_ev->m_task_idx;
    test_task.m_db_id = casted_ev->m_db_id;

    vTaskEntryList task_entry_list = ent->m_player->m_tasks_entry_list;
    //find task
    bool found = false;

    for (uint32_t i = 0; i < task_entry_list.size(); ++i)
    {
       for (uint32_t t = 0; t < task_entry_list[i].m_task_list.size(); ++t)
       {
           if (task_entry_list[i].m_task_list[t].m_task_idx != casted_ev->m_task_idx)
                continue;

            found = true;

            //contact already in list, update task;
            test_task.m_task_detail = task_entry_list[i].m_task_list[t].m_detail;
            break;
       }

       if(found)
           break;
    }

    if(!found)
    {
       qCDebug(logMapEvents) << "ReceiveTaskDetailRequest m_task_idx: " << casted_ev->m_task_idx << " not found.";
       test_task.m_task_detail = "Not found";
    }

    GameCommandVector commands;
    commands.emplace_back(std::make_unique<TaskDetail>(test_task.m_db_id, test_task.m_task_idx, test_task.m_task_detail));

    return std::make_unique<ServiceToClientData>(ent, std::move(commands), QString());
}

std::unique_ptr<ServiceToClientData> InteractionService::on_dialog_button(Entity* ent, Event *ev)
{
    DialogButton* casted_ev = static_cast<DialogButton *>(ev);

    if(casted_ev->success) // only sent by contactresponse
        qCDebug(logMapEvents) << "Dialog success" << casted_ev->success;

    switch(casted_ev->button_id)
    {
    case 0:
        // cancel?
        break;
    case 1:
        // accept?
        if(ent->m_char->m_in_training) // if training, raise level
            increaseLevel(*ent);

        break;
    case 2:
        // no idea
        break;
    case 3:
        sendContactDialogClose(*ent->m_client);
        break;
    default:
        // close all windows?
        break;
    }

    qCDebug(logMapEvents) << "Entity: " << ent->m_idx << "has received DialogButton" << casted_ev->button_id << casted_ev->success;

    ScriptingServiceToClientData* scriptData = new ScriptingServiceToClientData();
    if(ent->m_active_dialog != NULL)
    {
        scriptData->flags |= uint32_t(ScriptingServiceFlags::UpdateClientContext);
        ent->m_active_dialog(casted_ev->button_id);
    }
    else
    {
        scriptData->flags |= uint32_t(ScriptingServiceFlags::CallFuncWithClientContext);
        scriptData->funcName = "dialog_button";
        scriptData->intArg = casted_ev->button_id;
    }

    ScriptVector scripts {scriptData};
    return std::make_unique<ServiceToClientData>(ent, scripts, QString());
}

std::unique_ptr<ServiceToClientData> InteractionService::on_souvenir_detail_request(Entity* ent, Event* ev)
{
    SouvenirDetailRequest* casted_ev = static_cast<SouvenirDetailRequest* >(ev);
    vSouvenirList sl = ent->m_player->m_souvenirs;

    Souvenir souvenir_detail;
    bool found = false;
    for(const Souvenir &s: sl)
    {
        if(s.m_idx != (uint32_t)casted_ev->m_souvenir_idx)
            continue;
\
        souvenir_detail = s;
        found = true;
        qCDebug(logScripts) << "SouvenirDetail Souvenir " << casted_ev->m_souvenir_idx << " found";
        break;
    }

    if(!found)
    {
        qCDebug(logScripts) << "SouvenirDetail Souvenir " << casted_ev->m_souvenir_idx << " not found";
        souvenir_detail.m_idx = 0; // Should always be found?
        souvenir_detail.m_description = "Data not found";
    }

    GameCommandVector commands;
    commands.emplace_back(std::make_unique<SouvenirDetail>(souvenir_detail));

    return std::make_unique<ServiceToClientData>(ent, std::move(commands), QString());
}
