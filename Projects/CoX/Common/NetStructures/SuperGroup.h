/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CommonNetStructures.h"
#include "Costume.h"

class Entity;
class SuperGroup;

struct SGResponse
{
    bool        is_success = false;
    QString     msgfrom;
    QString     msgtgt;
};

struct SuperGroupData
{
    QString     m_sg_name;                  // 64 chars max
    QString     m_sg_motto;
    QString     m_sg_motd;
    QString     m_sg_emblem;                // 128 chars max -> hash table key from the CostumeString_HTable. Maybe Emblem?
    uint32_t    m_sg_colors[2] = {0};
    QString     m_sg_titles[3];             // Eventually we'll need to support additional titles (I23+)
    uint32_t    m_sg_leader_db_id = 0;

    template<class Archive>
    void serialize(Archive &ar)
    {
        ar(m_sg_name);
        ar(m_sg_motto);
        ar(m_sg_motd);
        ar(m_sg_emblem);
        ar(m_sg_colors);
        ar(m_sg_titles);
        ar(m_sg_leader_db_id);
    }
};


/*
 * SuperGroupStats
 */
struct SuperGroupStats
{
    static const constexpr  uint32_t        class_version   = 1;
    bool        m_has_supergroup    = false;
    uint32_t    m_sg_idx            = 0; // db_id of supergroup
    uint32_t    m_rank              = 0; // This character's rank in the SG
    bool        m_has_sg_costume    = false; // player has a sg costume
    bool        m_sg_mode           = false; // player uses sg costume currently
    const Costume * m_sg_costume;

    // The below are needed by /sgstats
    uint32_t    m_db_id         = 0;
    uint32_t    m_date_joined   = 0; // date joined as ms since Jan 1 2000
    uint32_t    m_hours_logged  = 0; // hours logged in sg_mode
    uint32_t    m_last_online   = 0; // last online as ms since Jan 1 2000
    bool        m_is_online     = false;
    QString     m_name;
    QString     m_sg_name;
    QString     m_origin_icon;
    QString     m_class_icon;

    SuperGroup *getSuperGroup();
    void        dump();
};


class SuperGroup
{
public:
        SuperGroup()
            : m_sg_idx( ++m_sg_idx_counter )
        { }
virtual ~SuperGroup() = default;

        // Member Vars
        std::vector<SuperGroupStats> m_sg_members;
        SuperGroupData m_data;

const   uint32_t    m_sg_idx            = 0;
const   uint32_t    m_max_sg_size       = 150;  // max is always 150

        // Methods
        void        dump();
        void        listSGMembers();
        void        addSGMember(Entity *e, int rank = 0);
        void        removeSGMember(Entity *e);
        bool        isSGLeader(Entity *e);
        bool        makeSGLeader(Entity &src, Entity &tgt);

private:
static  uint32_t  m_sg_idx_counter;
};


/*
 * SG Methods
 */
bool sameSG(Entity &src, Entity &tgt);
SGResponse inviteSG(Entity &src, Entity &tgt);
QString kickSG(Entity &src, Entity &tgt);
void leaveSG(Entity &e);
bool toggleSGMode(Entity &e);
QString setSGMOTD(Entity &e, QString &motd);
QString setSGMotto(Entity &e, QString &motto);
QString setSGTitle(Entity &e, int idx, QString &title);
QString modifySGRank(Entity &src, Entity &tgt, int rank_mod);
QString demoteSG(Entity &src, Entity &tgt);

SuperGroupStats *getSGMember(Entity &tgt, uint32_t sg_idx);
SuperGroup* getSuperGroupByIdx(uint32_t sg_idx);
void addSuperGroup(Entity &e, SuperGroupData data);

extern std::vector<SuperGroup> g_all_supergroups;
