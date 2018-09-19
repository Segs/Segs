/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup NetStructures Projects/CoX/Common/NetStructures
 * @{
 */

#include "SuperGroup.h"

#include "Servers/MapServer/DataHelpers.h"
#include "Servers/MapServer/Events/MessageChannels.h"
#include "Entity.h"
#include "Character.h"
#include "Logging.h"
#include <QDateTime>


// global variable -- a supergroup service is probably better
std::vector<SuperGroup> g_all_supergroups;
// Max size is probably hardcoded, but I can't get 75 friends together to check =P
static const uint32_t s_max_sg_size = 75;  // max is 75 in I1 client, later raised to 150

int64_t getTimeSince2000Epoch()
{
    QDateTime base_date(QDate(2000,1,1));
    return base_date.secsTo(QDateTime::currentDateTime());
}


/*
 * SG Methods
 */
uint32_t SuperGroup::m_sg_idx_counter = 0;

void SuperGroup::addSGMember(Entity *e, int rank)
{
    CharacterData *cd = &e->m_char->m_char_data;

    if(m_sg_members.size() >= s_max_sg_size)
        return;

    if(cd->m_supergroup.m_has_supergroup)
        return;

    SuperGroupStats sgs;
    sgs.m_name          = e->name();
    sgs.m_db_id         = e->m_db_id;
    sgs.m_rank          = rank;
    sgs.m_hours_logged  = 0;
    sgs.m_date_joined   = getTimeSince2000Epoch();
    sgs.m_last_online   = getTimeSince2000Epoch();
    sgs.m_is_online     = true;
    sgs.m_class_icon    = e->m_char->m_char_data.m_class_name;
    sgs.m_origin_icon   = e->m_char->m_char_data.m_origin_name;

    m_sg_members.push_back(sgs);
    cd->m_supergroup.m_has_supergroup = true;
    cd->m_supergroup.m_sg_idx = m_sg_idx;

    if(m_sg_members.size() <= 1)
        m_data.m_sg_leader_db_id = e->m_db_id;

    qCDebug(logSuperGroups) << "Adding" << e->name() << "to supergroup" << m_sg_idx;
    if(logSuperGroups().isDebugEnabled())
        dump();

}

void SuperGroup::removeSGMember(Entity *e)
{
    CharacterData *cd = &e->m_char->m_char_data;

    qCDebug(logSuperGroups) << "Searching SuperGroup members for" << e->name() << "to remove them.";
    uint32_t id_to_find = e->m_db_id;
    auto iter = std::find_if( m_sg_members.begin(), m_sg_members.end(),
                              [id_to_find](const SuperGroupStats& m)->bool {return id_to_find==m.m_db_id;});
    if(iter!=m_sg_members.end())
    {
        if(iter->m_db_id == m_data.m_sg_leader_db_id)
            m_data.m_sg_leader_db_id = m_sg_members.front().m_db_id;

        iter = m_sg_members.erase(iter);
        cd->m_supergroup.m_has_supergroup = false;
        cd->m_supergroup.m_sg_idx = 0;

        qCDebug(logSuperGroups) << "Removing" << iter->m_name << "from SuperGroup" << m_sg_idx;
        if(logSuperGroups().isDebugEnabled())
            listSGMembers();
    }

    if(m_sg_members.size() == 1)
    {
        qCDebug(logSuperGroups) << "One player left on SuperGroup. Making them leader.";
        if(logSuperGroups().isDebugEnabled())
            listSGMembers();

        m_data.m_sg_leader_db_id = m_sg_members.front().m_db_id;
        // TODO: fix entity's rank somehow
    }
}

void SuperGroup::dump()
{
    QString output = "Debugging SuperGroups: " + QString::number(m_sg_idx)
            + "\n\t name: " + m_data.m_sg_name
            + "\n\t db_id: " + QString::number(m_sg_idx)
            + "\n\t leader db_id: " + QString::number(m_data.m_sg_leader_db_id)
            + "\n\t size: " + QString::number(m_sg_members.size());
    qDebug().noquote() << output;

    listSGMembers();
}

void SuperGroup::listSGMembers()
{
    QString output = "SG Members:";

    for (auto &member : m_sg_members)
        output += "\n\t" + member.m_name + " db_id: " + QString::number(member.m_db_id);

    qDebug().noquote() << output;
}

uint32_t SuperGroup::getSGLeaderDBID()
{
    return m_data.m_sg_leader_db_id;
}

bool sameSG(Entity &src, Entity &tgt)
{
    return src.m_char->m_char_data.m_supergroup.m_sg_idx == tgt.m_char->m_char_data.m_supergroup.m_sg_idx;
}

bool SuperGroup::makeSGLeader(Entity &tgt)
{
    CharacterData *tgt_cd = &tgt.m_char->m_char_data;

    if(!tgt_cd->m_supergroup.m_has_supergroup
            || m_sg_idx != tgt_cd->m_supergroup.m_sg_idx
            || getSGLeaderDBID() == tgt.m_db_id)
        return false;

    m_data.m_sg_leader_db_id = tgt.m_db_id;
    tgt_cd->m_supergroup.m_rank = 2;

    return true;
}

SGResponse inviteSG(Entity &src, Entity &tgt)
{
    SGResponse response;
    response.is_success = false;

    SuperGroupStats *src_sg = &src.m_char->m_char_data.m_supergroup;
    SuperGroupStats *tgt_sg = &tgt.m_char->m_char_data.m_supergroup;

    if(tgt_sg->m_has_supergroup)
    {
        // NOTE: we should never get to this point
        response.msgfrom = tgt.name() + " already has a SuperGroup.";
        return response;
    }

    if(tgt.name() == src.name())
    {
        response.msgfrom = "You cannot invite yourself to a supergroup.";
        return response;
    }

    if(src_sg->m_has_supergroup && src_sg->m_rank < 1)
    {
        response.msgfrom = "You do not have high enough rank to invite players to the SuperGroup.";
        return response;
    }

    SuperGroup * sg = getSuperGroupByIdx(src_sg->m_sg_idx);
    if(sg == nullptr)
        qFatal("getSuperGroupByIdx returned nullptr for inviteSG");

    response.is_success = true;
    response.msgfrom = "Inviting " + tgt.name() + " to SuperGroup.";
    response.msgtgt = src.name() + " has invited you to join their SuperGroup.";

    sg->addSGMember(&tgt, 0);
    markSuperGroupForDbStore(sg, SuperGroupDbStoreFlags::Full);
    return response;
}

QString kickSG(Entity &src, Entity &tgt)
{
    QString msg = "Kicking " + tgt.name() + " from SuperGroup.";

    SuperGroupStats *src_sg = &src.m_char->m_char_data.m_supergroup;
    SuperGroupStats *tgt_sg = &tgt.m_char->m_char_data.m_supergroup;

    if(!tgt_sg->m_has_supergroup || !sameSG(src, tgt))
        return msg = tgt.name() + " is not in your SuperGroup.";

    if(src_sg->m_rank < 1)
        return msg = src.name() + " does not have authority to kick SuperGroup members.";

    SuperGroup * sg = getSuperGroupByIdx(tgt_sg->m_sg_idx);
    if(sg == nullptr)
        qFatal("getSuperGroupByIdx returned nullptr for kickSG");

    sg->removeSGMember(&tgt);
    markSuperGroupForDbStore(sg, SuperGroupDbStoreFlags::Full);
    return msg;
}

void leaveSG(Entity &e)
{
    CharacterData *cd = &e.m_char->m_char_data;

    if(!cd->m_supergroup.m_has_supergroup)
    {
        qCWarning(logSuperGroups) << "Trying to leave a SuperGroup, but Entity has no SG!?";
        return;
    }

    SuperGroup * sg = getSuperGroupByIdx(cd->m_supergroup.m_sg_idx);
    if(sg == nullptr)
    {
        qFatal("getSuperGroupByIdx returned nullptr for leaveSG");
        return; // if somehow qFatal doesn't do it
    }

    sg->removeSGMember(&e);
    markSuperGroupForDbStore(sg, SuperGroupDbStoreFlags::Full);

    // if there are no members left, delete SG
    //if(sg->m_sg_members.size == 0)
        //removeSuperGroup(sg);
}

bool toggleSGMode(Entity &e)
{
    CharacterData *cd = &e.m_char->m_char_data;

    if(!cd->m_supergroup.m_has_supergroup)
    {
        qCWarning(logSuperGroups) << "Trying to change SuperGroup mode, but Entity has no SG!?";
        return false;
    }

    cd->m_supergroup.m_sg_mode = !cd->m_supergroup.m_sg_mode;

    return cd->m_supergroup.m_sg_mode;
}

QString setSGMOTD(Entity &e, QString &motd)
{
    QString msg = "Successfully changed SuperGroup MOTD.";
    SuperGroupStats *sgs = &e.m_char->m_char_data.m_supergroup;

    if(!sgs->m_has_supergroup)
        return msg = "You are not in a SuperGroup.";

    if(sgs->m_rank < 1)
        return msg = "You do not have the authority to set the SuperGroup MOTD.";

    if(motd.isEmpty())
        return msg = "You must provide a new MOTD.";

    SuperGroup * sg = getSuperGroupByIdx(sgs->m_sg_idx);
    if(sg == nullptr)
        qFatal("getSuperGroupByIdx returned nullptr for setSGMOTD");

    sg->m_data.m_sg_motd = motd;
    markSuperGroupForDbStore(sg, SuperGroupDbStoreFlags::Full);
    return msg;
}

QString setSGMotto(Entity &e, QString &motto)
{
    QString msg = "Successfully changed SuperGroup Motto.";
    SuperGroupStats *sgs = &e.m_char->m_char_data.m_supergroup;

    if(!sgs->m_has_supergroup)
        return msg = "You are not in a SuperGroup.";

    if(sgs->m_rank < 1)
        return msg = "You do not have the authority to set the SuperGroup Motto.";

    if(motto.isEmpty())
        return msg = "You must provide a new Motto.";

    SuperGroup * sg = getSuperGroupByIdx(sgs->m_sg_idx);
    if(sg == nullptr)
        qFatal("getSuperGroupByIdx returned nullptr for setSGMotto");

    sg->m_data.m_sg_motto = motto;
    markSuperGroupForDbStore(sg, SuperGroupDbStoreFlags::Full);
    return msg;
}

QString setSGTitle(Entity &e, int idx, QString &title)
{
    SuperGroupStats *sgs = &e.m_char->m_char_data.m_supergroup;
    SuperGroup * sg = getSuperGroupByIdx(sgs->m_sg_idx);
    if(sg == nullptr)
        qFatal("getSuperGroupByIdx returned nullptr for setSGTitle");

    // limit idx to 0...2
    idx = std::min(std::max(idx,0),2);
    QString msg = "Setting SuperGroup rank titles for " + sg->m_data.m_sg_name;

    switch(idx)
    {
    case 0:
        msg = "Setting Leader title to ";
        break;
    case 1:
        msg = "Setting Captain title to ";
        break;
    case 2:
        msg = "Setting Member title to ";
        break;
    }

    msg += title;
    sg->m_data.m_sg_titles[idx] = title;
    markSuperGroupForDbStore(sg, SuperGroupDbStoreFlags::Full);
    return msg;
}

QString modifySGRank(Entity &src, Entity &tgt, int rank_mod)
{
    QString msg = "Modifying " + tgt.name() + "'s rank in SuperGroup by " + QString::number(rank_mod);

    SuperGroupStats *src_sg = &src.m_char->m_char_data.m_supergroup;
    SuperGroupStats *tgt_sg = &tgt.m_char->m_char_data.m_supergroup;

    if(!tgt_sg->m_has_supergroup || !sameSG(src, tgt))
        return msg = tgt.name() + " is not in your SuperGroup.";

    if(src_sg->m_rank < 2)
        return msg = src.name() + " does not have authority to promote/demote SuperGroup members.";

    if(rank_mod == 0)
        return msg = "SuperGroup rank unmodified.";

    if((tgt_sg->m_rank == 2 && rank_mod > 0) || (tgt_sg->m_rank == 0 && rank_mod < 0))
        return msg = tgt.name() + " cannot modify range outside of range 1 to 3.";

    SuperGroupStats *tgt_revised = getSGMember(tgt, src_sg->m_sg_idx);
    if(tgt_revised == nullptr)
        return msg = "getSGMember returned nullptr for promoteSG";

    uint32_t new_rank = tgt_sg->m_rank + rank_mod;
    new_rank = std::min(std::max(rank_mod,0),2); // if somehow we messed up, limit range from 1 to 3.

    tgt_sg->m_rank = tgt_revised->m_rank = new_rank;

    SuperGroup * sg = getSuperGroupByIdx(tgt_sg->m_sg_idx);
    if(sg == nullptr)
        qFatal("getSuperGroupByIdx returned nullptr for modifySGRank");

    markSuperGroupForDbStore(sg, SuperGroupDbStoreFlags::Full);
    return msg;
}


SuperGroupStats* getSGMember(Entity &tgt, uint32_t sg_idx)
{
    int idx = -1;

    qCDebug(logSuperGroups) << "Searching for SuperGroup Member" << tgt.name();
    if(sg_idx == 0 || !tgt.m_char->m_char_data.m_supergroup.m_has_supergroup)
        return nullptr;

    SuperGroup *sg = getSuperGroupByIdx(sg_idx);
    if(sg == nullptr)
        qFatal("getSuperGroupByIdx returned nullptr for getSGMember");

    uint32_t id_to_find = tgt.m_char->m_db_id;
    auto iter = std::find_if( sg->m_sg_members.begin(), sg->m_sg_members.end(),
                              [id_to_find](const SuperGroupStats& m)->bool {return id_to_find==m.m_db_id;});
    if(iter != sg->m_sg_members.end())
    {
        qCDebug(logSuperGroups) << "Returning SuperGroup Member" << iter->m_name;
        idx = std::distance(sg->m_sg_members.begin(), iter);
    }

    if(idx == -1)
        return nullptr;

    return &sg->m_sg_members.at(idx);
}

SuperGroup* getSuperGroupByIdx(uint32_t sg_idx)
{
    //qCDebug(logSuperGroups) << "Searching for SuperGroup" << sg_idx;
    if(sg_idx != 0)
        return &g_all_supergroups.at(sg_idx-1);

    return nullptr;
}

void addSuperGroup(Entity &e, SuperGroupData data)
{
    CharacterData *cd = &e.m_char->m_char_data;
    SuperGroup sg;

    sg.m_data = data;
    sg.addSGMember(&e, 2);
    g_all_supergroups.push_back(sg);

    if(logSuperGroups().isDebugEnabled())
        g_all_supergroups.back().dump();

    cd->m_supergroup.m_sg_idx           = sg.m_sg_idx;
    cd->m_supergroup.m_has_supergroup   = true;
    cd->m_supergroup.m_rank             = 2; // first member should be leader
    cd->m_supergroup.m_has_sg_costume   = true;
    cd->m_supergroup.m_sg_mode          = false;
    cd->m_supergroup.m_sg_costume       = e.m_char->getCurrentCostume();
}


/*
 * SuperGroupStats
 */
SuperGroup* SuperGroupStats::getSuperGroup()
{
    if(m_has_supergroup && m_sg_idx != 0)
        return getSuperGroupByIdx(m_sg_idx);
}

void SuperGroupStats::dump()
{
    QString msg = QString("SuperGroup Info\n  has_supergroup: %1 \n  db_id: %2 \n  rank: %3 ")
            .arg(m_has_supergroup)
            .arg(m_sg_idx)
            .arg(m_rank);

    qDebug().noquote() << msg;
}


/*
 * Mark for db update
 */
void markSuperGroupForDbStore(SuperGroup *sg, SuperGroupDbStoreFlags f)
{
    sg->m_db_store_flags |= uint32_t(f);
}

void unmarkSuperGroupForDbStore(SuperGroup *sg, SuperGroupDbStoreFlags f)
{
    sg->m_db_store_flags &= ~uint32_t(f);
}

//! @}
