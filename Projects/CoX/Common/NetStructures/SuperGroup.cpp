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


// global variable -- we probably don't want to do this
std::vector<SuperGroup> g_all_supergroups;

int64_t getTimeSince2000Epoch()
{
    QDateTime base_date(QDate(2000,1,1));
    return base_date.secsTo(QDateTime::currentDateTime());
}


/*
 * SG Methods
 */
uint32_t SuperGroup::m_sg_idx_counter = 0;

void SuperGroup::addSGMember(Entity *e)
{
    CharacterData *cd = &e->m_char->m_char_data;

    if(m_sg_members.size() >= m_max_sg_size)
        return;

    if(cd->m_supergroup.m_has_supergroup)
        return;

    SuperGroupStats sgs;
    sgs.m_name          = e->name();
    sgs.m_db_id         = e->m_db_id;
    sgs.m_rank          = 3;
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

    // TODO: This goes away, because SG sizes of 1 are permitted?
    if(m_sg_members.size() <= 1)
    {
        qCDebug(logSuperGroups) << "One player left on SuperGroup. Removing last entity and deleting SuperGroup.";
        if(logSuperGroups().isDebugEnabled())
            listSGMembers();

        int idx = m_sg_members.front().m_db_id;

        Entity *tgt = nullptr;
        if((tgt/* = getEntityByDBID(e->m_client->m_current_map,idx)*/) == nullptr)
            return;

        cd->m_supergroup.m_has_supergroup = false;
        cd->m_supergroup.m_sg_idx = 0;
        m_sg_members.clear();
        m_data.m_sg_leader_db_id = 0;

        qCDebug(logSuperGroups) << "After removing all entities.";
        if(logSuperGroups().isDebugEnabled())
            listSGMembers();
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

bool SuperGroup::isSGLeader(Entity *e)
{
    return m_data.m_sg_leader_db_id == e->m_db_id;
}

bool sameSG(Entity &src, Entity &tgt)
{
    return src.m_char->m_char_data.m_supergroup.m_sg_idx == tgt.m_char->m_char_data.m_supergroup.m_sg_idx;
}

bool SuperGroup::makeSGLeader(Entity &src, Entity &tgt)
{
    CharacterData *src_cd = &src.m_char->m_char_data;
    CharacterData *tgt_cd = &tgt.m_char->m_char_data;

    if(!src_cd->m_supergroup.m_has_supergroup || !tgt_cd->m_supergroup.m_has_supergroup
            || !sameSG(src,tgt)
            || !isSGLeader(&src))
        return false;

    m_data.m_sg_leader_db_id = tgt.m_db_id;

    return true;
}

QString inviteSG(Entity &src, Entity &tgt)
{
    QString msg;

    SuperGroupStats *src_sg = &src.m_char->m_char_data.m_supergroup;
    SuperGroupStats *tgt_sg = &tgt.m_char->m_char_data.m_supergroup;

    if(tgt_sg->m_has_supergroup)
        return msg = tgt.name() + " already has a SuperGroup.";

    if(tgt.name() == src.name())
        return msg = "You cannot invite yourself to a supergroup.";

    if(src_sg->m_has_supergroup && src_sg->m_rank <= 1)
        return msg = "You do not have high enough rank to invite players to the SuperGroup.";

    SuperGroup * sg = getSuperGroupByIdx(src_sg->m_sg_idx);
    if(sg == nullptr)
        qFatal("getSuperGroupByIdx returned nullptr");

    sg->addSGMember(&tgt);
    return msg;
}

bool kickSG(Entity &tgt)
{
    CharacterData *cd = &tgt.m_char->m_char_data;

    if (!cd->m_supergroup.m_has_supergroup)
        return false;

    SuperGroup * sg = getSuperGroupByIdx(cd->m_supergroup.m_sg_idx);
    if(sg == nullptr)
    {
        qFatal("getSuperGroupByIdx returned nullptr");
        return false; // if somehow qFatal doesn't do it
    }

    sg->removeSGMember(&tgt);
    return true;
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
        qFatal("getSuperGroupByIdx returned nullptr");
        return; // if somehow qFatal doesn't do it
    }

    sg->removeSGMember(&e);
    // if there are no members left, delete SG
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

SuperGroup* getSuperGroupByIdx(uint32_t sg_idx)
{
    qCDebug(logSuperGroups) << "Seaching for SuperGroup" << sg_idx;
    if(sg_idx != 0)
        return &g_all_supergroups.at(sg_idx-1);

    return nullptr;
}

void addSuperGroup(Entity &e, SuperGroupData data)
{
    CharacterData *cd = &e.m_char->m_char_data;
    SuperGroup sg;

    sg.m_data = data;
    sg.addSGMember(&e);
    g_all_supergroups.push_back(sg);

    if(logSuperGroups().isDebugEnabled())
        g_all_supergroups.back().dump();

    cd->m_supergroup.m_sg_idx           = sg.m_sg_idx;
    cd->m_supergroup.m_has_supergroup   = true;
    cd->m_supergroup.m_rank             = 3;
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

//! @}
