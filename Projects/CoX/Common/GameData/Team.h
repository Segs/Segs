/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CommonNetStructures.h"

class Entity;

enum class TeamingError
{
    OK,
    TEAM_FULL,
    INVITEE_HAS_TEAM,
	NOT_ON_TEAM,
	TEAM_DISBANDED,
};

class Team
{
public:
        Team(bool transient=false) : m_transient(transient)
        {
            m_data.m_team_idx = ++m_team_idx_counter;
        }
        ~Team();

        struct TeamMember {
            uint32_t    tm_idx  = 0;
            QString     tm_name; // stored here for quick lookup.
            QString     tm_map; // stored here for quick lookup.
            // these values are transient, and should be updated by TeamingService
            uint32_t    tm_map_idx = 0;
            bool        tm_pending = false; // if true, user has not responded to invite yet

            template<class Archive>
            void serialize(Archive &ar)
            {
                ar(tm_idx, tm_name, tm_map, tm_map_idx, tm_pending); //, tm_data
            }
        };


        struct TeamData {
            uint32_t    m_team_idx          = 0;
            bool        m_has_taskforce  = false;    // it's possible that this belongs to entity or char instead
            uint32_t    m_team_leader_idx   = 0;
            std::vector<TeamMember> m_team_members;
            template<class Archive>
            void serialize(Archive &ar)
            {
                ar(m_team_idx, m_has_taskforce, m_team_leader_idx, m_team_members);
            }
        };
        TeamData m_data;
        // Member Vars

        // indicates that the team is still being formed
        // i.e. an invite has been sent by a player NOT YET on a team
        bool        m_transient;
        uint32_t    m_max_team_size     = 8;        // max is always 8

        // Methods
        void        dump();
        void        dumpAllTeamMembers();
		bool				isTeamLeader(uint32_t entity_id);
		bool				isTeamLeader(const QString &name);

        TeamingError        acceptTeamInvite(const QString &name, uint32_t entity_id);

        TeamingError        addTeamMember(uint32_t entity_id, const QString &name, bool pending);

        TeamingError        removeTeamMember(uint32_t entity_id);

        bool                containsEntityID(uint32_t entity_id);
        bool                containsEntityName(const QString &name);
        bool                isFull();

		bool 				isNamePending(const QString &name);

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
