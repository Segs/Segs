/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

/*!
 * @addtogroup NetStructures Projects/CoX/Common/NetStructures
 * @{
 */

#include "Team.h"

#include "Servers/MapServer/DataHelpers.h"
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

void Team::removeTeamMember(Entity *e)
{
    qCDebug(logTeams) << "Searching team members for" << e->name() << "to remove them.";
    uint32_t id_to_find = e->m_db_id;
    auto iter = std::find_if( m_team_members.begin(), m_team_members.end(),
                              [id_to_find](const TeamMember& t)->bool {return id_to_find==t.tm_idx;});
    if(iter!=m_team_members.end())
    {
        if(iter->tm_idx == m_team_leader_idx)
            m_team_leader_idx = m_team_members.front().tm_idx;

        iter = m_team_members.erase(iter);
        e->m_has_team = false;
        e->m_team = nullptr;

        if(e->m_char->m_char_data.m_sidekick.m_has_sidekick)
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
    if(src.name() == tgt.name())
    {
        qCDebug(logTeams) << "You cannot invite yourself to a team.";
        return false;
    }

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
    if (!tgt.m_has_team)
        return false;

    tgt.m_team->removeTeamMember(&tgt);
    return true;
}

void leaveTeam(Entity &e)
{
    if(!e.m_team)
    {
        qCWarning(logTeams) << "Trying to leave a team, but Entity has no team!?";
        return;
    }

    e.m_team->removeTeamMember(&e);
}

/*
 * Sidekick Methods -- Sidekick system requires teaming.
 */
// TODO: expose these to config and fail-test
static const int g_max_sidekick_level_difference = 3;
static const int g_min_sidekick_mentor_level = 10;

bool isSidekickMentor(const Entity &e)
{
    return (e.m_char->m_char_data.m_sidekick.m_type == SidekickType::IsMentor);
}

void inviteSidekick(Entity &src, Entity &tgt)
{
    const QString possible_messages[] = {
        QStringLiteral("Unable to add sidekick."),
        QStringLiteral("To Mentor another player, you must be at least 3 levels higher than them."),
        QStringLiteral("To Mentor another player, you must be at least level 10."),
        QStringLiteral("You are already Mentoring someone."),
        tgt.name() + QStringLiteral("is already a sidekick."),
        QStringLiteral("To Mentor another player, you must be on the same team."),
    };

    QString     msg = possible_messages[0];
    Sidekick    &src_sk = src.m_char->m_char_data.m_sidekick;
    Sidekick    &tgt_sk = tgt.m_char->m_char_data.m_sidekick;
    uint32_t    src_lvl = getLevel(*src.m_char);
    uint32_t    tgt_lvl = getLevel(*tgt.m_char);

    // Only a mentor may invite a sidekick
    if(src_lvl < tgt_lvl+g_max_sidekick_level_difference)
        msg = possible_messages[1];
    else if(src_lvl < g_min_sidekick_mentor_level)
        msg = possible_messages[2];
    else if(src_sk.m_has_sidekick)
        msg = possible_messages[3];
    else if (tgt_sk.m_has_sidekick)
        msg = possible_messages[4];
    else if(!src.m_has_team || !tgt.m_has_team || src.m_team == nullptr || tgt.m_team == nullptr)
        msg = possible_messages[5];
    else if(src.m_team->m_team_idx != tgt.m_team->m_team_idx)
        msg = possible_messages[5];
    else
    {
        // Store this here now for sidekick_accept / decline
        tgt_sk.m_db_id = src.m_db_id;

        // sendSidekickOffer
        sendSidekickOffer(&tgt, src.m_db_id); // tgt gets dialog, src.db_id is named.
        return; // break early
    }

    qCDebug(logTeams).noquote() << msg;
    messageOutput(MessageChannel::USER_ERROR, msg, src);
}

void addSidekick(Entity &tgt, Entity &src)
{
    QString     msg;
    Sidekick    &src_sk = src.m_char->m_char_data.m_sidekick;
    Sidekick    &tgt_sk = tgt.m_char->m_char_data.m_sidekick;
    uint32_t    src_lvl = getLevel(*src.m_char);
    uint32_t    tgt_lvl = getLevel(*tgt.m_char);
    Q_UNUSED(tgt_lvl);

    src_sk.m_has_sidekick = true;
    tgt_sk.m_has_sidekick = true;
    src_sk.m_db_id = tgt.m_db_id;
    tgt_sk.m_db_id = src.m_db_id;
    src_sk.m_type = SidekickType::IsMentor;
    tgt_sk.m_type = SidekickType::IsSidekick;
    setCombatLevel(*tgt.m_char, src_lvl - 1);
    // TODO: Implement 225 feet "leash" for sidekicks.

    msg = QString("%1 is now Mentoring %2.").arg(src.name(),tgt.name());
    qCDebug(logTeams).noquote() << msg;

    // Send message to each player
    msg = QString("You are now Mentoring %1.").arg(tgt.name()); // Customize for src.
    messageOutput(MessageChannel::TEAM, msg, src);
    msg = QString("%1 is now Mentoring you.").arg(src.name()); // Customize for src.
    messageOutput(MessageChannel::TEAM, msg, tgt);
}

void removeSidekick(Entity &src)
{
    QString     msg = "Unable to remove sidekick.";
    Sidekick    &src_sk = src.m_char->m_char_data.m_sidekick;

    if(!src_sk.m_has_sidekick || src_sk.m_db_id == 0)
    {
        msg = "You are not sidekicked with anyone.";
        qCDebug(logTeams).noquote() << msg;
        messageOutput(MessageChannel::USER_ERROR, msg, src);
        return; // break early
    }

    Entity      *tgt            = getEntityByDBID(src.m_client, src_sk.m_db_id);
    Sidekick    &tgt_sk         = tgt->m_char->m_char_data.m_sidekick;

    if(tgt == nullptr)
    {
        msg = "Your sidekick is not currently online.";
        qCDebug(logTeams).noquote() << msg;

        // reset src Sidekick relationship
        src_sk.m_has_sidekick = false;
        src_sk.m_type         = SidekickType::NoSidekick;
        src_sk.m_db_id        = 0;
        setCombatLevel(*src.m_char,getLevel(*src.m_char)); // reset CombatLevel

        return; // break early
    }

    // Anyone can terminate a Sidekick relationship
    if(!tgt_sk.m_has_sidekick || (tgt_sk.m_db_id != src.m_db_id))
    {
        // tgt doesn't know it's sidekicked with src. So clear src sidekick info.
        src_sk.m_has_sidekick = false;
        src_sk.m_type         = SidekickType::NoSidekick;
        src_sk.m_db_id        = 0;
        setCombatLevel(*src.m_char,getLevel(*src.m_char)); // reset CombatLevel
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

        src_sk.m_has_sidekick = false;
        src_sk.m_type         = SidekickType::NoSidekick;
        src_sk.m_db_id        = 0;
        setCombatLevel(*src.m_char,getLevel(*src.m_char)); // reset CombatLevel

        tgt_sk.m_has_sidekick = false;
        tgt_sk.m_type         = SidekickType::NoSidekick;
        tgt_sk.m_db_id        = 0;
        setCombatLevel(*tgt->m_char,getLevel(*tgt->m_char)); // reset CombatLevel

        msg = QString("%1 and %2 are no longer sidekicked.").arg(src.name(),tgt->name());
        qCDebug(logTeams).noquote() << msg;

        return; // break early
    }

    qCDebug(logTeams).noquote() << msg;
    messageOutput(MessageChannel::USER_ERROR, msg, src);
}

//! @}
