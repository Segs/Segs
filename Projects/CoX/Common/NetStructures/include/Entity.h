#pragma once
#include "CommonNetStructures.h"
#include "Powers.h"
#include "Costume.h"
#include "Character.h"
#include "math_constants.h"



class PosUpdate
{
public:
    Vector3 posvec;
    Quaternion quat;
    int a;
    int b;
};
class InputState;
class Entity;
class Avatar : public NetStructure //! not to be confuzzled with GameServer's Character :)
{
public:
        Entity *m_ent;
        uint32_t field_5A8;
        uint32_t field_498;
        std::string char_arr_10_5AC;
virtual			~Avatar()
                {
                    m_ent = NULL; // we only borrowed this pointer
                }
                Avatar(Entity *m_ent);
virtual void	serializeto(BitStream &bs) const;

        void	send_character(BitStream &bs)const;
        void	sendFullStats(BitStream &bs)const;
        void	sendBuffs(BitStream &bs)const;
        void	sendTray(BitStream &bs)const;
        void	sendTrayMode(BitStream &bs)const;
        void	sendEntStrings(BitStream &bs)const;
        void	sendWindow(BitStream &bs)const;
        void	sendTeamBuffMode(BitStream &bs)const;
        void	sendDockMode(BitStream &bs)const;
        void	sendChatSettings(BitStream &bs)const;
        void	sendTitles(BitStream &bs)const;
        void	sendDescription(BitStream &bs)const;
        void	sendKeybinds(BitStream &bs)const;
        void	sendOptions(BitStream &bs)const;
        void	sendFriendList(BitStream &bs)const;
        void	GetCharBuildInfo(BitStream &src);
        void	DumpPowerPoolInfo(const PowerPool_Info &pool_info);
        void	DumpBuildInfo();

};

class Entity : public NetStructure
{
public:
        struct currentInputState
        {
            Vector3 pos;
            Vector3 pyr;
        };
        currentInputState inp_state;
        enum
        {
            ENT_PLAYER=2,
            ENT_CRITTER=4,
        };
        int field_64;
        int field_60;
        int field_68;
        int field_78;
        int m_num_titles;
        int m_num_fx;
        bool m_has_titles;
        std::vector<uint8_t> m_fx1;
        std::vector<uint32_t> m_fx2;
        std::vector<uint8_t> m_fx3;
        uint8_t		m_costume_type;
        int		m_state_mode;
        bool	m_state_mode_send;
        bool	m_odd_send;
        bool	m_SG_info;
        bool	m_seq_update;
        bool	m_is_villian;
        bool	m_contact;
        int m_seq_upd_num1;
        int m_seq_upd_num2;
        PosUpdate m_pos_updates[64];
        size_t m_update_idx;
        std::string m_battle_cry;
        std::string m_character_description;
        bool var_B4;

        Character   m_char;

        bool        entReceiveAlwaysCon;
        bool        entReceiveSeeThroughWalls;
        int         pkt_id_QrotUpdateVal[3];
        Quaternion  qrot;
        Vector3     pos;
        uint32_t    prev_pos[3];
        bool        m_selector1,m_pchar_things,might_have_rare,
                    m_hasname  ,m_hasgroup_name,m_classname_override;
        bool        m_create   ,m_hasRagdoll  ,m_create_player,m_rare_bits;
        int         current_client_packet_id;
        std::string m_group_name, m_override_name;
        uint8_t                 m_origin_idx,m_class_idx;
        uint8_t                 m_type;
        uint32_t                m_idx;
        uint32_t                m_input_ack;
        bool                    player_type;
        bool                    m_player_villain;
        bool                    var_129C;

                                Entity();
virtual                         ~Entity(){}
        void                    dump();
        uint32_t                getIdx() const {return m_idx;}
virtual void                    serializeto(BitStream &bs)const;
        void                    sendStateMode(BitStream &bs) const;
        void                    sendOnOddSend(BitStream &bs,bool is_odd=true) const;
        void                    sendSeqMoveUpdate(BitStream &bs) const;
        void                    sendSeqTriggeredMoves(BitStream &bs) const;
        void                    sendNetFx(BitStream &bs) const;
virtual	void                    sendCostumes(BitStream &bs) const;
        void                    sendXLuency(BitStream &bs,float xluency) const;
        void                    sendTitles(BitStream &bs) const;
        void                    sendRagDoll(BitStream &bs) const;
static	void                    sendAllyID(BitStream &bs);
static	void                    sendPvP(BitStream &bs);
        void                    sendEntCollision(BitStream &bs) const;
        void                    sendNoDrawOnClient(BitStream &bs)const;
        void                    sendContactOrPnpc(BitStream &bs)const;
        void                    sendPetName(BitStream &bs)const;
        void                    sendAFK(BitStream &bs)const;
        void                    sendOtherSupergroupInfo(BitStream &bs)const;
        void                    sendLogoutUpdate(BitStream &bs)const;

        void                    storePosition(BitStream &bs) const;
        void                    storeOrientation(BitStream &bs) const;
        void                    storeUnknownBinTree(BitStream &bs) const;
        void                    storePosUpdate(BitStream &bs) const;
        int                     getOrientation(BitStream &bs);
virtual void                    serializefrom(BitStream &){}
        bool                    update_rot(int axis) const; // returns true if given axis needs updating;

        void                    InsertUpdate(PosUpdate pup);
        void                    sendCharacterStats(BitStream &bs) const;
        void                    sendTargetUpdate(BitStream &bs) const;
        void                    sendWhichSideOfTheForce(BitStream &bs) const;
        void                    sendBuffsConditional(BitStream &bs) const;
        const std::string &     name() {return m_char.getName();}
        void                    sendBuffs(BitStream &bs) const;
        void                    setInputState(Vector3 &pos, Vector3 pyr);
};
class MobEntity : public Entity
{
        std::string     m_costume_seq;
public:
                        MobEntity();
virtual                 ~MobEntity(){}
virtual	void            sendCostumes(BitStream &bs) const;


};
class PlayerEntity : public MobEntity
{
public:
                            PlayerEntity();
virtual                     ~PlayerEntity(){}
        void                serializefrom_newchar(BitStream &src);
        void                sendCostumes( BitStream &bs ) const;
        void                serialize_full( BitStream &tgt );
};
