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
    if(!src.m_has_team)
    {
        qCDebug(logTeams) << src.name() << "is forming a team.";
        src.m_team = new Team;
        src.m_team->addTeamMember(&src);

        tgt.m_team = src.m_team;
        src.m_team->addTeamMember(&tgt);
        return true;
    }
    else if (src.m_has_team && src.m_team->isTeamLeader(&src))
    {
        src.m_team->addTeamMember(&tgt);
        return true;
    }
    else
    {
        qCDebug(logTeams) << src.name() << "is not team leader.";
        return false;
    }

    qCWarning(logTeams) << "How did we get here in inviteTeam?";
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


/*
 * Sidekick Methods -- Sidekick system requires teaming.
 */
// TODO: expose these to config and fail-test
static const int g_max_sidekick_level_difference = 3;
static const int g_min_sidekick_mentor_level = 10;

bool isSidekickMentor(const Entity &e)
{
    return (e.m_char.m_char_data.m_sidekick.sk_type == SidekickType::IsMentor);
}

void inviteSidekick(Entity &src, Entity &tgt)
{
    QString     msg = "Unable to add sidekick.";
    Sidekick    &src_sk = src.m_char.m_char_data.m_sidekick;
    Sidekick    &tgt_sk = tgt.m_char.m_char_data.m_sidekick;
    uint32_t    src_lvl = getLevel(src.m_char);
    uint32_t    tgt_lvl = getLevel(tgt.m_char);

    // Only a mentor may invite a sidekick
    if(src_lvl < tgt_lvl+g_max_sidekick_level_difference)
        msg = "To Mentor another player, you must be at least 3 levels higher than them.";
    else if(src_lvl < g_min_sidekick_mentor_level)
        msg = "To Mentor another player, you must be at least level 10.";
    else if(src_sk.sk_has_sidekick)
        msg = "You are already Mentoring someone.";
    else if (tgt_sk.sk_has_sidekick)
        msg = tgt.name() + "is already a sidekick.";
    else if(!src.m_has_team && !tgt.m_has_team && src.m_team == nullptr && tgt.m_team == nullptr)
            msg = "To Mentor another player, you must be on the same team.";
    else if(src.m_team->m_team_idx != tgt.m_team->m_team_idx)
        msg = "To Mentor another player, you must be on the same team.";
    else
    {
        // Store this here now for sidekick_accept / decline
        tgt_sk.sk_db_id = src.m_db_id;

        // sendSidekickOffer
        sendSidekickOffer(&tgt, src.m_db_id); // tgt gets dialog, src.db_id is named.
        return; // break early
    }

    qDebug().noquote() << msg;
    messageOutput(MessageChannel::USER_ERROR, msg, src);
}

void addSidekick(Entity &src, Entity &tgt)
{
    QString     msg;
    Sidekick    &src_sk = src.m_char.m_char_data.m_sidekick;
    Sidekick    &tgt_sk = tgt.m_char.m_char_data.m_sidekick;
    uint32_t    src_lvl = getLevel(src.m_char);
    uint32_t    tgt_lvl = getLevel(tgt.m_char);

    src_sk.sk_has_sidekick = true;
    tgt_sk.sk_has_sidekick = true;
    src_sk.sk_db_id = tgt.m_db_id;
    tgt_sk.sk_db_id = src.m_db_id;
    src_sk.sk_type = SidekickType::IsMentor;
    tgt_sk.sk_type = SidekickType::IsSidekick;
    setCombatLevel(tgt.m_char, src_lvl - 1);
    // TODO: Implement 225 feet "leash" for sidekicks.

    msg = QString("%1 is now Mentoring %2.").arg(src.name(),tgt.name());
    qDebug().noquote() << msg;

    // Send message to each player
    msg = QString("You are now Mentoring %1.").arg(tgt.name()); // Customize for src.
    messageOutput(MessageChannel::TEAM, msg, src);
    msg = QString("%1 is now Mentoring you.").arg(src.name()); // Customize for src.
    messageOutput(MessageChannel::TEAM, msg, tgt);
}

void removeSidekick(Entity &src)
{
    QString     msg = "Unable to remove sidekick.";
    Sidekick    &src_sk = src.m_char.m_char_data.m_sidekick;

    if(!src_sk.sk_has_sidekick || src_sk.sk_db_id == 0)
    {
        msg = "You are not sidekicked with anyone.";
        qDebug().noquote() << msg;
        messageOutput(MessageChannel::USER_ERROR, msg, src);
        return; // break early
    }

    Entity      *tgt            = getEntityByDBID(src.m_client, src_sk.sk_db_id);
    Sidekick    &tgt_sk         = tgt->m_char.m_char_data.m_sidekick;

    if(tgt == nullptr)
    {
        msg = "Your sidekick is not currently online.";
        qDebug().noquote() << msg;

        // reset src Sidekick relationship
        src_sk.sk_has_sidekick = false;
        src_sk.sk_type         = SidekickType::NoSidekick;
        src_sk.sk_db_id        = 0;
        setCombatLevel(src.m_char,getLevel(src.m_char)); // reset CombatLevel

        return; // break early
    }

    // Anyone can terminate a Sidekick relationship
    if(!tgt_sk.sk_has_sidekick || (tgt_sk.sk_db_id != src.m_db_id))
    {
        // tgt doesn't know it's sidekicked with src. So clear src sidekick info.
        src_sk.sk_has_sidekick = false;
        src_sk.sk_type         = SidekickType::NoSidekick;
        src_sk.sk_db_id        = 0;
        setCombatLevel(src.m_char,getLevel(src.m_char)); // reset CombatLevel
        msg = QString("You are no longer sidekicked with anyone.");
    }
    else {

        // Send message to each player
        if(isSidekickMentor(src))
        {
            // src is mentor, tgt is sidekick
            msg = QString("You are no longer mentoring %1.").arg(tgt->name());
            messageOutput(MessageChannel::TEAM, msg, src);
            msg = QString("%1 is no longer mentoring you.").arg(src.name());
            messageOutput(MessageChannel::TEAM, msg, *tgt);
        }
        else
        {
            // src is sidekick, tgt is mentor
            msg = QString("You are no longer mentoring %1.").arg(src.name());
            messageOutput(MessageChannel::TEAM, msg, *tgt);
            msg = QString("%1 is no longer mentoring you.").arg(tgt->name());
            messageOutput(MessageChannel::TEAM, msg, src);
        }

        src_sk.sk_has_sidekick = false;
        src_sk.sk_type         = SidekickType::NoSidekick;
        src_sk.sk_db_id        = 0;
        setCombatLevel(src.m_char,getLevel(src.m_char)); // reset CombatLevel

        tgt_sk.sk_has_sidekick = false;
        tgt_sk.sk_type         = SidekickType::NoSidekick;
        tgt_sk.sk_db_id        = 0;
        setCombatLevel(tgt->m_char,getLevel(tgt->m_char)); // reset CombatLevel

        msg = QString("%1 and %2 are no longer sidekicked.").arg(src.name(),tgt->name());
        qDebug().noquote() << msg;

        return; // break early
    }

    qDebug().noquote() << msg;
    messageOutput(MessageChannel::USER_ERROR, msg, src);
}
