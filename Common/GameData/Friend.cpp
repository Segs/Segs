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

#include "Friend.h"

#include "CharacterHelpers.h"
#include "Components/serialization_common.h"
#include "Components/serialization_types.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/map_definitions.h"
#include "Entity.h"
#include "Components/Logging.h"
#include "Character.h"


/*
 * Friend Methods -- Sidekick system requires teaming.
 */
void toggleFriendList(Entity &src)
{
    GUIWindow *friendlist = &src.m_player->m_gui.m_wnds.at(WindowIDX::wdw_Friends);
    QString msg = "Toggling FriendList visibility.";
    msg += " " + QString::number(friendlist->m_mode);

    // TODO: How to actually change window visibility?
    if(friendlist->m_mode != WindowVisibility::wv_Visible)
        friendlist->setWindowVisibility(WindowVisibility::wv_Visible);
    else
        friendlist->setWindowVisibility(WindowVisibility::wv_DockedOrHidden);

    msg += " to " + QString::number(friendlist->m_mode);
    qCDebug(logFriends).noquote() << msg;
}

void dumpFriends(const Entity &src)
{
    const FriendsList *fl(&src.m_char->m_char_data.m_friendlist);
    QString msg = QString("FriendsList\n  has_friends: %1 \n friends_count: %2 ")
            .arg(fl->m_has_friends)
            .arg(fl->m_friends_count);

    qDebug().noquote() << msg;

    for(const auto &f : fl->m_friends)
        dumpFriendsList(f);
}

void dumpFriendsList(const Friend &f)
{
    qDebug().noquote() << "Friend:" << f.m_name
             << "\n\t" << "online:" << f.m_online_status
             << "\n\t" << "db_id:" << f.m_db_id
             << "\n\t" << "class_id:" << f.m_class_idx
             << "\n\t" << "origin_id:" << f.m_origin_idx
             << "\n\t" << "map_idx:" << f.m_map_idx
             << "\n\t" << "mapname:" << f.m_mapname;
}

FriendListChangeStatus addFriend(Entity &src, const Entity &tgt,const QString &mapname)
{
    QString msg;
    FriendsList &src_data(src.m_char->m_char_data.m_friendlist);

    if(src_data.m_friends_count >= g_max_friends)
    {
        return FriendListChangeStatus::MAX_FRIENDS_REACHED;
    }

    src_data.m_has_friends = true;
    src_data.m_friends_count++;

    Friend f;
    f.m_online_status   = (tgt.m_client != nullptr); // need some other method for this.
    f.m_db_id           = tgt.m_db_id;
    f.m_name            = tgt.name();
    f.m_class_idx       = tgt.m_entity_data.m_class_idx;
    f.m_origin_idx      = tgt.m_entity_data.m_origin_idx;
    f.m_map_idx         = tgt.m_entity_data.m_map_idx;
    f.m_mapname         = mapname;

    // add to friendlist
    src_data.m_friends.emplace_back(f);
    qCDebug(logFriends) << "friendslist size:" << src_data.m_friends_count << src_data.m_friends.size();

    if(logFriends().isDebugEnabled())
        dumpFriends(src);
    return FriendListChangeStatus::FRIEND_ADDED;
}

FriendListChangeStatus removeFriend(Entity &src, const QString& friend_name)
{
    QString msg;
    FriendsList &src_data(src.m_char->m_char_data.m_friendlist);

    qCDebug(logFriends) << "Searching for friend" << friend_name << "to remove them.";

    QString lower_name = friend_name.toLower();
    auto iter = std::find_if( src_data.m_friends.begin(), src_data.m_friends.end(),
                              [lower_name](const Friend& f)->bool {return lower_name==f.m_name.toLower();});

    if(iter==src_data.m_friends.end())
        return FriendListChangeStatus::FRIEND_NOT_FOUND;

    msg = "Removing " + iter->m_name + " from your friends list.";
    iter = src_data.m_friends.erase(iter);

    qCDebug(logFriends) << msg;
    if(logFriends().isDebugEnabled())
        dumpFriends(src);

    if(src_data.m_friends.empty())
        src_data.m_has_friends = false;

    src_data.m_friends_count = src_data.m_friends.size();

    qCDebug(logFriends).noquote() << msg;
    return FriendListChangeStatus::FRIEND_REMOVED;
}

//const QString &getFriendDisplayMapName(const Friend &f)
//{
//    static const QString offline(QStringLiteral("OFFLINE"));
//    if(!f.m_online_status)
//        return offline;
//    return getDisplayMapName(f.m_map_idx);
//}

template<class Archive>
void Friend::serialize(Archive &archive, uint32_t const version)
{
    if(version != Friend::class_version)
    {
        qCritical() << "Failed to serialize Friend, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("FriendOnline", m_online_status));
    archive(cereal::make_nvp("FriendDbId", m_db_id));
    archive(cereal::make_nvp("FriendName", m_name));
    archive(cereal::make_nvp("FriendClass", m_class_idx));
    archive(cereal::make_nvp("FriendOrigin", m_origin_idx));
    archive(cereal::make_nvp("FriendMapIdx", m_map_idx));
    archive(cereal::make_nvp("FriendMapname", m_mapname));
}
CEREAL_CLASS_VERSION(Friend, Friend::class_version)       // register Friend struct version
SPECIALIZE_CLASS_VERSIONED_SERIALIZATIONS(Friend)


template<class Archive>
void serialize(Archive &archive, FriendsList &fl, uint32_t const version)
{
    if(version != FriendsList::class_version)
    {
        qCritical() << "Failed to serialize FriendsList, incompatible serialization format version " << version;
        return;
    }

    archive(cereal::make_nvp("HasFriends",fl.m_has_friends));
    archive(cereal::make_nvp("FriendsCount",fl.m_friends_count));
    archive(cereal::make_nvp("Friends",fl.m_friends));
}

CEREAL_CLASS_VERSION(FriendsList, FriendsList::class_version)       // register FriendList struct version
SPECIALIZE_VERSIONED_SERIALIZATIONS(FriendsList)

//! @}
