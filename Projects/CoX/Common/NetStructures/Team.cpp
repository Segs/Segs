/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#include "Servers/MapServer/DataHelpers.h"
#include "Team.h"
#include "Entity.h"
#include "Logging.h"

uint32_t Team::m_team_idx_counter = 0;

void Team::addTeamMember(Entity *e)
{
    if(m_team_members.size() >= m_max_team_size)
        return;

    if(e->m_has_team)
        return;

    m_team_members.emplace_back(TeamMember{e->m_db_id, e->name()});
    e->m_has_team = true;

    if(m_team_members.size() <= 1)
        m_team_leader_idx = e->m_db_id;

    qCDebug(logTeams) << "Adding" << e->name() << "to team" << m_team_idx;
    if(logTeams().isDebugEnabled())
        dump();

}

void Team::removeTeamMember(Entity *e)
{
    qCDebug(logTeams) << "Searching team members for" << e->name() << "to remove them.";
    int id_to_find = e->m_db_id;
    auto iter = std::find_if( m_team_members.begin(), m_team_members.end(),
                              [id_to_find](const TeamMember& t)->bool {return id_to_find==t.tm_idx;});
    if(iter!=m_team_members.end())
    {
        if(iter->tm_idx == m_team_leader_idx)
            m_team_leader_idx = m_team_members.front().tm_idx;

        iter = m_team_members.erase(iter);
        e->m_has_team = false;
        e->m_team = nullptr;

        if(e->m_char.m_char_data.m_sidekick.sk_has_sidekick)
            removeSidekick(*e);

        qCDebug(logTeams) << "Removing" << iter->tm_name << "from team" << m_team_idx;
        if(logTeams().isDebugEnabled())
            listTeamMembers();
    }

    if(m_team_members.size() <= 1)
    {
        qCDebug(logTeams) << "One player left on team. Removing last entity and deleting team.";
        if(logTeams().isDebugEnabled())
            listTeamMembers();

        int idx = m_team_members.front().tm_idx;

        Entity *tgt = nullptr;
        if((tgt = getEntityByDBID(e->m_client,idx)) == nullptr)
            return;

        tgt->m_has_team = false;
        tgt->m_team = nullptr;
        m_team_members.clear();
        m_team_leader_idx = 0;

        qCDebug(logTeams) << "After removing all entities.";
        if(logTeams().isDebugEnabled())
            listTeamMembers();
    }
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

bool inviteTeam(Entity &src, Entity &tgt)
{
    if(tgt.m_has_team)
    {
        qCDebug(logTeams) << tgt.name() << "is already on a team.";
        return false;
    }

    if(!src.m_has_team)
    {
        if(!src.m_has_team)
        {
            qCDebug(logTeams) << src.name() << "is forming a team.";
            src.m_team = new Team;
            src.m_team->addTeamMember(&src);
        }

        tgt.m_team = src.m_team;
        src.m_team->addTeamMember(&tgt);
        return true;
    }
    else if (src.m_has_team && src.m_team->isTeamLeader(&src))
    {
        src.m_team->addTeamMember(&tgt);
        return true;
    }

    qCWarning(logTeams) << "how did we get here?";
    return false;
}

bool kickTeam(Entity &tgt)
{
    if (tgt.m_has_team)
    {
        tgt.m_team->removeTeamMember(&tgt);
        return true;
    }
    else
       return false;
}

void leaveTeam(Entity &e)
{
    if(e.m_has_team)
        e.m_team->removeTeamMember(&e);
}

void findTeamMember(Entity &tgt)
{
    // TODO: figure out what the search window is and leverage that.
    qWarning() << "Search Window Not Implemented";
}
