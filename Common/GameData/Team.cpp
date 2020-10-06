/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "Team.h"

#include "CharacterHelpers.h"
#include "Entity.h"
#include "LFG.h"
#include "Components/Logging.h"
#include "Character.h"

/*
 * Team Methods
 */
uint32_t Team::m_team_idx_counter = 0;
bool Team::isNamePending(const QString &name)
{
    for (const TeamMember &tm : m_data.m_team_members) 
    {
        if (tm.tm_name == name && tm.tm_pending) 
        {
            return true;
        }
    }

    return false;
}

bool Team::isFull() 
{
    return m_data.m_team_members.size() >= m_max_team_size;
}

bool Team::containsEntityID(uint32_t entity_id) 
{
    for (const TeamMember &tm : m_data.m_team_members) 
    {
        if (tm.tm_idx == entity_id) 
        {
            return true;
        }
    }

    return false;
}

bool Team::containsEntityName(const QString &name) 
{
    for (const TeamMember &tm : m_data.m_team_members) 
    {
        if (QString::compare(tm.tm_name, name, Qt::CaseInsensitive) == 0)
        {
            return true;
        }
    }

    return false;
}

TeamingError Team::acceptTeamInvite(const QString &name, uint32_t entity_id)
{
    for (TeamMember &_t : m_data.m_team_members)
        if (_t.tm_name == name)
        {
            _t.tm_pending = false;
            _t.tm_idx = entity_id;
            return TeamingError::OK;
        }

    return TeamingError::NOT_ON_TEAM;

}

TeamingError Team::addTeamMember(uint32_t entity_id, const QString &name, bool pending)
{
    if(m_data.m_team_members.size() >= m_max_team_size)
        return TeamingError::TEAM_FULL;

    // create new team member
    TeamMember new_member;
    new_member.tm_idx = entity_id;
    new_member.tm_name = name;
    new_member.tm_pending = pending;

    m_data.m_team_members.emplace_back(new_member);

    if(m_data.m_team_members.size() <= 1)
        m_data.m_team_leader_idx = entity_id;

    return TeamingError::OK;
}

TeamingError Team::removeTeamMember(uint32_t entity_id)
{
    auto iter = std::find_if(m_data.m_team_members.begin(), m_data.m_team_members.end(),
                              [entity_id](const Team::TeamMember& t) -> bool {return entity_id == t.tm_idx;});

    if(iter == m_data.m_team_members.end())
		return TeamingError::NOT_ON_TEAM;

	iter = m_data.m_team_members.erase(iter);
	// TODO: sidekick stuff

    if(m_data.m_team_members.size() < 2)
        return TeamingError::TEAM_DISBANDED;

    if (entity_id == m_data.m_team_leader_idx)
        m_data.m_team_leader_idx = m_data.m_team_members.front().tm_idx;

    return TeamingError::OK;
}

Team::~Team() = default;

void Team::dump()
{
    QString output = "Debugging Team: " + QString::number(m_data.m_team_idx)
             + "\n\t size: " + QString::number(m_data.m_team_members.size())
             + "\n\t leader db_id: " + QString::number(m_data.m_team_leader_idx)
             + "\n\t has mission? " + QString::number(m_data.m_has_taskforce)
             + "\nTeam Members: ";
    qDebug().noquote() << output;

    dumpAllTeamMembers();
}

void Team::dumpAllTeamMembers()
{
    QString output = "Team Members:";

    for (auto &member : m_data.m_team_members)
        output += "\n\t" + member.tm_name + " db_id: " + QString::number(member.tm_idx);

    qDebug().noquote() << output;
}

bool Team::isTeamLeader(uint32_t entity_id)
{
    return m_data.m_team_leader_idx == entity_id;
}


/*
 * Public Team Methods
 */
bool sameTeam(Entity &src, Entity &tgt)
{
    return src.m_team->m_data.m_team_idx == tgt.m_team->m_data.m_team_idx;
}

bool makeTeamLeader(Entity &src, Entity &tgt)
{
    if(!src.m_has_team || !tgt.m_has_team
            || !sameTeam(src,tgt)
            || !(src.m_team->isTeamLeader(src.m_db_id)))
        return false;

    src.m_team->m_data.m_team_leader_idx = tgt.m_db_id;
    return true;
}

bool inviteTeam(Entity &src, Entity &tgt)
{
    if(src.name() == tgt.name())
    {
        qCDebug(logTeams) << "You cannot invite yourself to a team.";
        return false;
    }
    //TODO: this has to be reworked when TeamingServices come along
    if(!src.m_has_team)
    {
        qCDebug(logTeams) << src.name() << "is forming a team.";
        src.m_team = new Team;
        src.m_team->addTeamMember(src.m_db_id,src.name(),0);

        tgt.m_team = src.m_team;
        src.m_team->addTeamMember(tgt.m_db_id,tgt.name(),0);
        return true;
    }
    if(src.m_has_team && src.m_team->isTeamLeader(src.m_db_id))
    {
        src.m_team->addTeamMember(tgt.m_db_id,tgt.name(),0);
        return true;
    }
    qCDebug(logTeams) << src.name() << "is not team leader.";
    return false;
}

bool kickTeam(Entity &tgt)
{
    if(!tgt.m_has_team)
        return false;

    removeTeamMember(*tgt.m_team, &tgt);
    return true;
}

void leaveTeam(Entity &e)
{
    if(!e.m_team)
    {
        qCWarning(logTeams) << "Trying to leave a team, but Entity has no team!?";
        return;
    }

    removeTeamMember(*e.m_team, &e);
}

void removeTeamMember(Team &self, Entity *e)
{
    qCDebug(logTeams) << "Searching team members for" << e->name() << "to remove them.";
    uint32_t id_to_find = e->m_db_id;
    auto iter = std::find_if( self.m_data.m_team_members.begin(), self.m_data.m_team_members.end(),
                              [id_to_find](const Team::TeamMember& t)->bool {return id_to_find==t.tm_idx;});
    if(iter!=self.m_data.m_team_members.end())
    {
        if(iter->tm_idx == self.m_data.m_team_leader_idx)
            self.m_data.m_team_leader_idx = self.m_data.m_team_members.front().tm_idx;

        iter = self.m_data.m_team_members.erase(iter);
        e->m_has_team = false;
        e->m_team = nullptr;

        if(e->m_char->m_char_data.m_sidekick.m_has_sidekick)
        {
            uint32_t sidekick_id = getSidekickId(*e->m_char);
            removeSidekick(*e, sidekick_id);
        }

        qCDebug(logTeams) << "Removing" << iter->tm_name << "from team" << self.m_data.m_team_idx;
        if(logTeams().isDebugEnabled())
            self.dumpAllTeamMembers();
    }

    if(self.m_data.m_team_members.size() > 1)
        return;

    qCDebug(logTeams) << "One player left on team. Removing last entity and deleting team.";
    if(logTeams().isDebugEnabled())
        self.dumpAllTeamMembers();

    qWarning() << "Team should post an Team-removal event to the target entity.";
    // TODO: this should post an Team-removal event to the target entity, since we can't access other server's
    // Entity lists

    //int idx = self.m_team_members.front().tm_idx;
    Entity *tgt = nullptr;// getEntityByDBID(e->m_client, idx);
    if(tgt == nullptr)
        return;

    tgt->m_has_team = false;
    tgt->m_team = nullptr;
    self.m_data.m_team_members.clear();
    self.m_data.m_team_leader_idx = 0;

    qCDebug(logTeams) << "After removing all entities.";
    if(logTeams().isDebugEnabled())
        self.dumpAllTeamMembers();
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

SidekickChangeStatus inviteSidekick(Entity &src, Entity &tgt)
{
    Sidekick    &src_sk = src.m_char->m_char_data.m_sidekick;
    Sidekick    &tgt_sk = tgt.m_char->m_char_data.m_sidekick;
    uint32_t    src_lvl = getLevel(*src.m_char);
    uint32_t    tgt_lvl = getLevel(*tgt.m_char);

    // Only a mentor may invite a sidekick
    if(src_lvl < tgt_lvl+g_max_sidekick_level_difference)
        return SidekickChangeStatus::MENTOR_LEVEL_TOO_LOW;
    if(src_lvl < g_min_sidekick_mentor_level)
        return SidekickChangeStatus::CANNOT_MENTOR_YET;
    if(src_sk.m_has_sidekick)
        return SidekickChangeStatus::HAVE_SIDEKICK_ALREADY;
    if(tgt_sk.m_has_sidekick)
        return SidekickChangeStatus::TARGET_IS_SIDEKICKING_ALREADY;
    if(!src.m_has_team || !tgt.m_has_team || src.m_team == nullptr || tgt.m_team == nullptr)
        return SidekickChangeStatus::NO_TEAM_OR_SAME_TEAM_REQUIRED;
    if(src.m_team->m_data.m_team_idx != tgt.m_team->m_data.m_team_idx)
        return SidekickChangeStatus::NO_TEAM_OR_SAME_TEAM_REQUIRED;

    {
        // Store this here now for sidekick_accept / decline
        tgt_sk.m_db_id = src.m_db_id;

        return SidekickChangeStatus::SUCCESS;
    }

}

void addSidekick(Entity &tgt, Entity &src)
{
    QString     msg;
    Sidekick    &src_sk = src.m_char->m_char_data.m_sidekick;
    Sidekick    &tgt_sk = tgt.m_char->m_char_data.m_sidekick;
    uint32_t    src_lvl = getLevel(*src.m_char);

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
}

/**
 * @brief getSidekickId will return the db_id of the given Character's sidekick if any
 * @param src
 * @return db_id or 0 when character has no sidekick
 */
uint32_t getSidekickId(const Character &src)
{
    const Sidekick &src_sk(src.m_char_data.m_sidekick);
    return src_sk.m_db_id;
}

SidekickChangeStatus removeSidekick(Entity &src, uint32_t /*sidekick_id*/)
{
    //TODO: this function should actually post messages related to de-sidekicking to our target entity.
    QString     msg = "Unable to remove sidekick.";
    Sidekick    &src_sk = src.m_char->m_char_data.m_sidekick;

    if(!src_sk.m_has_sidekick || src_sk.m_db_id == 0)
        return SidekickChangeStatus::GENERIC_FAILURE;

    //TODO: just send a message to the SidekickHandler about this removal.
    qWarning() << "Sidekick needs to send a message to the SidekickHandler about this removal";
    Entity *tgt = nullptr; //getEntityByDBID(src.m_client, sidekick_id);
    if(tgt == nullptr)
    {
        msg = "Your sidekick is not currently online.";
        qCDebug(logTeams).noquote() << msg;

        // reset src Sidekick relationship
        src_sk.m_has_sidekick = false;
        src_sk.m_type         = SidekickType::NoSidekick;
        src_sk.m_db_id        = 0;
        setCombatLevel(*src.m_char, getLevel(*src.m_char)); // reset CombatLevel

        return SidekickChangeStatus::SUCCESS;
    }
    Sidekick &tgt_sk = tgt->m_char->m_char_data.m_sidekick;

    // Anyone can terminate a Sidekick relationship
    if(!tgt_sk.m_has_sidekick || (tgt_sk.m_db_id != src.m_db_id))
    {
        // tgt doesn't know it's sidekicked with src. So clear src sidekick info.
        src_sk.m_has_sidekick = false;
        src_sk.m_type         = SidekickType::NoSidekick;
        src_sk.m_db_id        = 0;
        setCombatLevel(*src.m_char,getLevel(*src.m_char)); // reset CombatLevel
        return SidekickChangeStatus::NOT_SIDEKICKED_CURRENTLY;
    }

    // Send message to each player
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

    return SidekickChangeStatus::SUCCESS;
}

//! @}
