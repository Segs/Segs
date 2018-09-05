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
#include "Entity.h"
#include "Character.h"
#include "Logging.h"

// global variable -- we probably don't want to do this
SuperGroupStorage *g_AllSuperGroups = nullptr;

SuperGroupStorage::SuperGroupStorage()
{
    assert(g_AllSuperGroups==nullptr && "Only one SuperGroupStorage instance per process allowed");
    g_AllSuperGroups = this;
}

/*
 * SG Methods
 */
uint32_t SuperGroup::m_sg_idx_counter = 0;

void SuperGroup::addSGMember(Entity *e)
{
    CharacterData cd = e->m_char->m_char_data;

    if(m_sg_members.size() >= m_max_sg_size)
        return;

    if(cd.m_supergroup.m_has_supergroup)
        return;

    m_sg_members.emplace_back(SGMember{e->m_db_id, e->name()});
    cd.m_supergroup.m_has_supergroup = true;
    cd.m_supergroup.m_sg_idx = m_sg_idx;

    if(m_sg_members.size() <= 1)
        m_data.m_sg_leader_idx = e->m_db_id;

    qCDebug(logSuperGroups) << "Adding" << e->name() << "to supergroup" << m_sg_idx;
    if(logSuperGroups().isDebugEnabled())
        dump();

}

void SuperGroup::removeSGMember(Entity *e)
{
    CharacterData cd = e->m_char->m_char_data;

    qCDebug(logSuperGroups) << "Searching SuperGroup members for" << e->name() << "to remove them.";
    uint32_t id_to_find = e->m_db_id;
    auto iter = std::find_if( m_sg_members.begin(), m_sg_members.end(),
                              [id_to_find](const SGMember& m)->bool {return id_to_find==m.idx;});
    if(iter!=m_sg_members.end())
    {
        if(iter->idx == m_data.m_sg_leader_idx)
            m_data.m_sg_leader_idx = m_sg_members.front().idx;

        iter = m_sg_members.erase(iter);
        cd.m_supergroup.m_has_supergroup = false;
        cd.m_supergroup.m_sg_idx = 0;

        qCDebug(logSuperGroups) << "Removing" << iter->name << "from SuperGroup" << m_sg_idx;
        if(logSuperGroups().isDebugEnabled())
            listSGMembers();
    }

    // TODO: This goes away, because SG sizes of 1 are permitted?
    if(m_sg_members.size() <= 1)
    {
        qCDebug(logSuperGroups) << "One player left on SuperGroup. Removing last entity and deleting SuperGroup.";
        if(logSuperGroups().isDebugEnabled())
            listSGMembers();

        int idx = m_sg_members.front().idx;

        Entity *tgt = nullptr;
        if((tgt = getEntityByDBID(e->m_client,idx)) == nullptr)
            return;

        cd.m_supergroup.m_has_supergroup = false;
        cd.m_supergroup.m_sg_idx = 0;
        m_sg_members.clear();
        m_data.m_sg_leader_idx = 0;

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
            + "\n\t leader db_id: " + QString::number(m_data.m_sg_leader_idx)
            + "\n\t size: " + QString::number(m_sg_members.size());
    qDebug().noquote() << output;

    listSGMembers();
}

void SuperGroup::listSGMembers()
{
    QString output = "SG Members:";

    for (auto &member : m_sg_members)
        output += "\n\t" + member.name + " db_id: " + QString::number(member.idx);

    qDebug().noquote() << output;
}

bool SuperGroup::isSGLeader(Entity *e)
{
    return m_data.m_sg_leader_idx == e->m_db_id;
}

bool sameSG(Entity &src, Entity &tgt)
{
    return src.m_char->m_char_data.m_supergroup.m_sg_idx == tgt.m_char->m_char_data.m_supergroup.m_sg_idx;
}

bool SuperGroup::makeSGLeader(Entity &src, Entity &tgt)
{
    CharacterData src_cd = src.m_char->m_char_data;
    CharacterData tgt_cd = tgt.m_char->m_char_data;

    if(!src_cd.m_supergroup.m_has_supergroup || !tgt_cd.m_supergroup.m_has_supergroup
            || !sameSG(src,tgt)
            || !isSGLeader(&src))
        return false;

    m_data.m_sg_leader_idx = tgt.m_db_id;

    return true;
}

bool inviteSG(Entity &src, Entity &tgt)
{
    CharacterData src_cd = src.m_char->m_char_data;

    if(src.name() == tgt.name())
    {
        qCDebug(logSuperGroups) << "You cannot invite yourself to a SuperGroup.";
        return false;
    }

    if (src_cd.m_supergroup.m_has_supergroup && src_cd.m_supergroup.getSuperGroup()->isSGLeader(&src))
    {
        src_cd.m_supergroup.getSuperGroup()->addSGMember(&tgt);
        return true;
    }
    else
    {
        qCDebug(logSuperGroups) << src.name() << "is not SG leader.";
        return false;
    }

    qCWarning(logSuperGroups) << "How did we get here in inviteSG?";
    return false;
}

bool kickSG(Entity &tgt)
{
    CharacterData cd = tgt.m_char->m_char_data;

    if (!cd.m_supergroup.m_has_supergroup)
        return false;

    cd.m_supergroup.getSuperGroup()->removeSGMember(&tgt);
    return true;
}

void leaveSG(Entity &e)
{
    CharacterData cd = e.m_char->m_char_data;

    if(!cd.m_supergroup.m_has_supergroup)
    {
        qCWarning(logSuperGroups) << "Trying to leave a SuperGroup, but Entity has no SG!?";
        return;
    }

    cd.m_supergroup.getSuperGroup()->removeSGMember(&e);
}

SuperGroup* SuperGroupStorage::getSuperGroupByIdx(uint32_t sg_idx)
{
    qCDebug(logSuperGroups) << "Seaching for SuperGroup" << sg_idx;
    if(sg_idx != 0)
        return &m_supergroups[sg_idx];

    return nullptr;
}

void SuperGroupStorage::addSuperGroup(Entity &e, SuperGroupData &data)
{
    CharacterData cd = e.m_char->m_char_data;
    SuperGroup *sg = new SuperGroup;

    sg->m_data = data;
    sg->addSGMember(&e);
    m_supergroups.push_back(*sg); // TODO: This segfaults? :shrug:

    cd.m_supergroup.m_sg_idx                = sg->m_sg_idx;
    cd.m_supergroup.m_has_supergroup        = true;
    cd.m_supergroup.m_sg_rank               = 3;
    cd.m_supergroup.m_supergroup_costume    = true;
}


/*
 * SuperGroupStats
 */
SuperGroup* SuperGroupStats::getSuperGroup()
{
    if(m_has_supergroup && m_sg_idx != 0)
        return g_AllSuperGroups->getSuperGroupByIdx(m_sg_idx);
}

void SuperGroupStats::dump()
{
    QString msg = QString("SuperGroup Info\n  has_supergroup: %1 \n  db_id: %2 \n  rank: %3 ")
            .arg(m_has_supergroup)
            .arg(m_sg_idx)
            .arg(m_sg_rank);

    qDebug().noquote() << msg;
}

//! @}
