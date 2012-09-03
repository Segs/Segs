/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

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
        uint8_t         m_player_collisions;
        float		m_unkn1,m_unkn2;
        uint32_t	m_unkn3,m_unkn4;
public:
                        Character();
//////////////////////////////////////////////////////////////////////////
// Getters and setters
        uint32_t		getLevel() const { return m_level; }
        void			setLevel(uint32_t val) { m_level = val; }
const	std::string &	getName() const { return m_name; }
        void			setName(const std::string &val);
const	std::string &	getMapName() const { return m_mapName; }
        void			setMapName(const std::string &val) { m_mapName = val; }
        uint8_t				getIndex() const { return m_index; }
        void			setIndex(uint8_t val) { m_index = val; }
        uint64_t			getAccountId() const { return m_owner_account_id; }
        void			setAccountId(uint64_t val) { m_owner_account_id = val; }
        uint64_t			getLastCostumeId() const { return m_last_costume_id; }
        void			setLastCostumeId(uint64_t val) { m_last_costume_id = val; }
//
//////////////////////////////////////////////////////////////////////////
        void			reset();
        bool			isEmpty();
        bool			serializeFromDB(uint64_t user_id,uint32_t slot_index);
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
        void            face_bits(uint32_t){}
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
        uint64_t            m_owner_account_id;
        uint64_t            m_last_costume_id;
        uint8_t             m_index;
        uint32_t            m_level;
        std::string         m_name;
        std::string         m_mapName;
        bool				m_villain;
        vCostume			m_costumes;
        Costume *			m_sg_costume;
        uint32_t			m_current_costume_idx;
        bool				m_current_costume_set;
        uint32_t			m_num_costumes;
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
