/*
 * Super Entity Game Server Project
 * https://github.com/Segs/Segs
 * Copyright (c) 2006 - 2018 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */

#pragma once
#include "CommonNetStructures.h"

class Entity;

class Team
{
public:
        Team()
            : m_team_idx( ++m_team_idx_counter )
        { }
virtual ~Team() = default;

        struct TeamMember {
            uint32_t    tm_idx;
            QString     tm_name; // stored here for quick lookup.
        };

        // Member Vars
        std::vector<TeamMember> m_team_members;

const   uint32_t    m_team_idx;
        int         m_max_team_size     = 8;        // max is always 8
        int         m_team_leader_idx   = 0;
        bool        m_team_has_mission  = false;    // it's possible that this belongs to entity or char instead

        // Methods
        void        dump();
        void        listTeamMembers();
        void        addTeamMember(Entity *e);
        void        removeTeamMember(Entity *e);
        bool        isTeamLeader(Entity *e);

private:
static  uint32_t  m_team_idx_counter;
};


/*
 * Team Methods
 */
bool sameTeam(Entity &src, Entity &tgt);
bool makeTeamLeader(Entity &src, Entity &tgt);
bool inviteTeam(Entity &src, Entity &tgt);
bool kickTeam(Entity &tgt);
void leaveTeam(Entity &e);
void findTeamMember(Entity &tgt);

/*
 * Sidekick Methods -- Sidekick system requires teaming.
 */
bool isSidekickMentor(const Entity &e);
void addSidekick(Entity &src, Entity &tgt);
void removeSidekick(Entity &src);
