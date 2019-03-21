/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
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
        ~Team();

        struct TeamMember {
            uint32_t    tm_idx  = 0;
            QString     tm_name; // stored here for quick lookup.
            QString     tm_map; // stored here for quick lookup.
            // this value is transient, and should be updated by TeamingService
            uint32_t    tm_map_idx = 0;
        };

        // Member Vars
        std::vector<TeamMember> m_team_members;

const   uint32_t    m_team_idx          = 0;
        uint32_t    m_max_team_size     = 8;        // max is always 8
        uint32_t    m_team_leader_idx   = 0;
        bool        m_has_taskforce  = false;    // it's possible that this belongs to entity or char instead

        // Methods
        void        dump();
        void        dumpAllTeamMembers();
        void        addTeamMember(Entity *e, uint32_t teammate_map_idx);
        bool        isTeamLeader(Entity *e);

private:
static  uint32_t    m_team_idx_counter;
};


/*
 * Team Methods
 */
bool sameTeam(Entity &src, Entity &tgt);
bool makeTeamLeader(Entity &src, Entity &tgt);
bool inviteTeam(Entity &src, Entity &tgt);
bool kickTeam(Entity &tgt);
void leaveTeam(Entity &e);
void removeTeamMember(Team &self,Entity *e);
enum class SidekickChangeStatus
{
    SUCCESS,
    GENERIC_FAILURE=1,
    LEVEL_DIFFERENCE_TOO_HIGH,
    MENTOR_LEVEL_TOO_LOW, // player level is not high enough
    CANNOT_MENTOR_YET, // player level must be higher to have a sidekick
    HAVE_SIDEKICK_ALREADY,
    TARGET_IS_SIDEKICKING_ALREADY,
    NO_TEAM_OR_SAME_TEAM_REQUIRED,
    NOT_SIDEKICKED_CURRENTLY,
};

/*
 * Sidekick Methods -- Sidekick system requires teaming.
 */

uint32_t getSidekickId(const class Character &src);
bool isSidekickMentor(const Entity &e);
SidekickChangeStatus inviteSidekick(Entity &src, Entity &tgt);
void addSidekick(Entity &tgt, Entity &src);
SidekickChangeStatus removeSidekick(Entity &src, uint32_t sidekick_id);
