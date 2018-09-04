/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup NetStructures Projects/CoX/Common/NetStructures
 * @{
 */

#include "Team.h"

#include "Servers/MapServer/DataHelpers.h"
#include "CharacterHelpers.h"
#include "Entity.h"
#include "LFG.h"
#include "Logging.h"
#include "Character.h"

/*
 * Team Methods
 */
uint32_t Team::m_team_idx_counter = 0;

void Team::addTeamMember(Entity *e)
{
    if(m_team_members.size() >= m_max_team_size)
        return;

    if(e->m_has_team)
        return;

    if(e->m_char->m_char_data.m_lfg)
        removeLFG(*e);

    m_team_members.emplace_back(TeamMember{e->m_db_id, e->name()});
    e->m_has_team = true;

    if(m_team_members.size() <= 1)
        m_team_leader_idx = e->m_db_id;

    qCDebug(logTeams) << "Adding" << e->name() << "to team" << m_team_idx;
    if(logTeams().isDebugEnabled())
        dump();

}

void Team::dump()
{
    QString output = "Debugging Team: " + QString::number(m_team_idx)
             + "\n\t size: " + QString::number(m_team_members.size())
             + "\n\t leader db_id: " + QString::number(m_team_leader_idx)
             + "\n\t has mission? " + QString::number(m_team_has_mission)
             + "\nTeam Members: ";
    qDebug().noquote() << output;

    listTeamMembers();
}

void Team::listTeamMembers()
{
    QString output = "Team Members:";

    for (auto &member : m_team_members)
        output += "\n\t" + member.tm_name + " db_id: " + QString::number(member.tm_idx);

    qDebug().noquote() << output;
}

bool Team::isTeamLeader(Entity *e)
{
    return m_team_leader_idx == e->m_db_id;
}

bool sameTeam(Entity &src, Entity &tgt)
{
    return src.m_team->m_team_idx == tgt.m_team->m_team_idx;
}

bool makeTeamLeader(Entity &src, Entity &tgt)
{
    if(!src.m_has_team || !tgt.m_has_team
            || !sameTeam(src,tgt)
            || !(src.m_team->isTeamLeader(&src)))
        return false;

    src.m_team->m_team_leader_idx = tgt.m_db_id;
    return true;
}


//! @}
