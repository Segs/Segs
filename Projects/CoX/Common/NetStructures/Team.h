/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
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
            uint32_t    tm_idx  = 0;
            QString     tm_name; // stored here for quick lookup.
        };

        // Member Vars
        std::vector<TeamMember> m_team_members;

const   uint32_t    m_team_idx          = 0;
        uint32_t    m_max_team_size     = 8;        // max is always 8
        uint32_t    m_team_leader_idx   = 0;
        bool        m_team_has_mission  = false;    // it's possible that this belongs to entity or char instead

        // Methods
        void        dump();
        void        listTeamMembers();
        void        addTeamMember(Entity *e);
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
void removeTeamMember(Team &self,Entity *e);

