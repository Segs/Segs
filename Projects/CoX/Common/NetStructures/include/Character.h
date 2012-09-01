/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

// Inclusion guards
#pragma once

//* Another small step toward a real Character server.
#include <cassert>
#include <string>

#include "types.h"
#include "CommonNetStructures.h"
#include "BitStream.h"
#include "Powers.h"
#define MAX_CHARACTER_SLOTS 8
struct ClientOption
{
    struct Arg
    {
        Arg(int t,void *v) : type(t),tgt(v){}
        int type;
        void *tgt;
    };
    std::vector<Arg> m_args;
};
class ClientOptions
{
    std::vector<ClientOption> m_opts;
    void init();
public:
    ClientOptions()
    {
        init();
    }
    bool    mouse_invert;
    float   mouselook_scalefactor;
    float   degrees_for_turns;
    int32_t control_debug;
    int32_t no_strafe;
    int32_t alwaysmobile;// 1- player is always mobile (can't be immobilized by powers)
    int32_t repredict;   //1 - client is out of sync with server, asking for new physics state info.
    int32_t neterrorcorrection;
    float   speed_scale;
    int32_t svr_lag,svr_lag_vary,svr_pl,svr_oo_packets,client_pos_id;
    int32_t atest0,atest1,atest2,atest3,atest4,atest5,atest6,atest7,atest8,atest9;
    int32_t predict,notimeout,selected_ent_server_index;
    ClientOption *get(int idx)
    {
        if(idx<0)
            return 0;
        if(idx>=0)
            assert(!"Unknown option requested!!");
        return &m_opts[idx];
    }

};
class Costume;

class Character : public NetStructure
{
        friend class CharacterDatabase;
        typedef std::vector<PowerPool_Info> vPowerPool;
        typedef std::vector<Costume *> vCostume;
        PowerPool_Info	get_power_info(BitStream &src);

        vPowerPool		m_powers;
        PowerTrayGroup  m_trays;
        std::string     m_class_name;
        std::string     m_origin_name;
        bool            m_full_options;
        ClientOptions   m_options;
        bool            m_first_person_view_toggle;
        u8              m_player_collisions;
        float			m_unkn1,m_unkn2;
        u32				m_unkn3,m_unkn4;
public:
                        Character();
//////////////////////////////////////////////////////////////////////////
// Getters and setters
        u32				getLevel() const { return m_level; }
        void			setLevel(u32 val) { m_level = val; }
const	std::string &	getName() const { return m_name; }
        void			setName(const std::string &val);
const	std::string &	getMapName() const { return m_mapName; }
        void			setMapName(const std::string &val) { m_mapName = val; }
        u8				getIndex() const { return m_index; }
        void			setIndex(u8 val) { m_index = val; }
        u64				getAccountId() const { return m_owner_account_id; }
        void			setAccountId(u64 val) { m_owner_account_id = val; }
        u64				getLastCostumeId() const { return m_last_costume_id; }
        void			setLastCostumeId(u64 val) { m_last_costume_id = val; }
//
//////////////////////////////////////////////////////////////////////////
        void			reset();
        bool			isEmpty();
        bool			serializeFromDB(u64 user_id,u32 slot_index);
        void			serializefrom(BitStream &buffer);
        void			serializeto(BitStream &buffer) const;
        void			serialize_costumes(BitStream &buffer,bool all_costumes=true) const;
        void			serializetoCharsel(BitStream &bs);
        void			GetCharBuildInfo(BitStream &src); // serialize from char creation
        void            SendCharBuildInfo(BitStream &bs) const;
        void            recv_initial_costume(BitStream &src);
        Costume *		getCurrentCostume() const;
        void			DumpPowerPoolInfo( const PowerPool_Info &pool_info );
        void			DumpBuildInfo();
        void            face_bits(u32){}
        void            dump();
        void            sendFullStats(BitStream &bs) const;
        //TODO: move these to some kind of Player info class
        void            sendTray(BitStream &bs);
        void            sendTrayMode(BitStream &bs) const;
        void            sendWindows(BitStream &bs) const;
        void            sendWindow(BitStream &bs) const;
        void            sendTeamBuffMode(BitStream &bs) const;
        void            sendDockMode(BitStream &bs) const;
        void            sendChatSettings(BitStream &bs) const;
        void            sendDescription(BitStream &bs) const;
        void            sendTitles(BitStream &bs) const;
        void            sendKeybinds(BitStream &bs) const;
        void            sendFriendList(BitStream &bs) const;
        void            sendOptions( BitStream &bs ) const;
        void            sendOptionsFull(BitStream &bs) const;
protected:
        u64                 m_owner_account_id;
        u64                 m_last_costume_id;
        u8                  m_index;
        u32                 m_level;
        std::string         m_name;
        std::string         m_mapName;
        bool				m_villain;
        vCostume			m_costumes;
        Costume *			m_sg_costume;
        u32					m_current_costume_idx;
        bool				m_current_costume_set;
        u32					m_num_costumes;
        bool				m_multiple_costumes; // has more then 1 costume
        bool				m_supergroup_costume; // player has a sg costume
        bool				m_using_sg_costume; // player uses sg costume currently
        typedef enum _CharBodyType
        {
            TYPE_MALE,
            TYPE_FEMALE,
            TYPE_UNUSED1,
            TYPE_UNUSED2,
            TYPE_HUGE,
            TYPE_NOARMS
        } CharBodyType, *pCharBodyType;

};
