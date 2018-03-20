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

//#define DEBUG_TEAMS

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

#ifdef DEBUG_TEAMS
    qDebug() << "Adding" << e->name() << "to team" << m_team_idx;
    dump();
#endif
}

void Team::removeTeamMember(Entity *e)
{
    qDebug() << "Searching team members for" << e->name() << "to remove them.";
    for(auto iter = m_team_members.begin(); iter != m_team_members.end();)
    {
        if(iter->tm_idx == e->m_db_id)
        {
            if(iter->tm_idx == m_team_leader_idx)
                m_team_leader_idx = m_team_members.front().tm_idx;

            iter = m_team_members.erase(iter);
            e->m_has_team = false;
            e->m_team = nullptr;
#ifdef DEBUG_TEAMS
            qDebug() << "Removing" << iter->tm_name << "from team" << m_team_idx;
            dump();
#endif
            break;
        }
        iter++;
    }

    if(m_team_members.size() <= 1)
    {
        qDebug() << "One player left on team. Removing last entity and deleting team.";
#ifdef DEBUG_TEAMS
        dump();
#endif
        int idx = m_team_members.front().tm_idx;

        Entity *tgt = nullptr;
        if((tgt = getEntityByDBID(e->m_client,idx)) == nullptr)
            return;
        tgt->m_has_team = false;
        tgt->m_team = nullptr;
        m_team_members.clear();
        m_team_leader_idx = 0;

#ifdef DEBUG_TEAMS
        qDebug() << "After removing all entities.";
        dump();
#endif

        //delete this; // TODO: how to delete this Team instance if we're done with it?
    }
}

void Team::dump()
{
    QString output = "Debugging Team: " + QString::number(m_team_idx)
             + "\n\t name: " + m_team_name
             + "\n\t rank (?): " + QString::number(m_team_rank)
             + "\n\t size: " + QString::number(m_team_members.size())
             + "\n\t leader db_id: " + QString::number(m_team_leader_idx)
             + "\n\t has mission? " + QString::number(m_team_has_mission)
             + "\nTeam Members: ";

    for (auto &member : m_team_members)
        output += "\n\t" + member.tm_name + " db_id: " + QString::number(member.tm_idx);

    qDebug().noquote() << output;
}

bool Team::isTeamLeader(Entity *e)
{
    if(m_team_leader_idx == e->m_db_id)
        return true;

    return false;
}

bool sameTeam(Entity &src, Entity &tgt)
{
    return (src.m_team->m_team_idx == tgt.m_team->m_team_idx) ? true : false;
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
        qDebug() << tgt.name() << "is already on a team.";
        return false;
    }

    if(!src.m_has_team || !tgt.m_has_team)
    {
        if(!src.m_has_team)
        {
            qDebug() << src.name() << "is forming a team.";
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

    qDebug() << "how did we get here?";
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
