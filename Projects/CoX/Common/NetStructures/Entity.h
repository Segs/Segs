#pragma once
#include <osg/Vec3>
#include <osg/Quat>
#include "CommonNetStructures.h"
#include "Powers.h"
#include "Costume.h"
#include "Character.h"
//#include "Events/InputState.h"
class PosUpdate
{
public:
    osg::Vec3 posvec;
    osg::Quat quat;
    int a;
    int b;
};
class InputStateStorage
{
public:
    InputStateStorage() {
        for(int i=0; i<3; ++i) {
            pos_delta_valid[i]=false;
            pyr_valid[i]=false;
        }
    }
    uint8_t m_csc_deltabits;
    bool m_send_deltas;
    uint16_t controlBits;
    uint16_t someOtherbits;
    void *current_state_P;
    osg::Vec3 camera_pyr;
    int m_t1,m_t2;
    int m_A_ang11_probably,m_B_ang11_probably;

    bool has_input_commit_guess;
    bool pos_delta_valid[3];
    bool pyr_valid[3];
    osg::Vec3 pos_delta;
    osg::Quat direction;
    InputStateStorage & operator=(const InputStateStorage &other);
    void processDirectionControl(int dir, int prev_time, int press_release);
};

class Entity : public NetStructure
{
public:
        struct currentInputState
        {
            osg::Vec3 pos;
            osg::Vec3 pyr; //TODO: convert to quat
        };
        InputStateStorage inp_state;
        enum
        {
            ENT_PLAYER=2,
            ENT_CRITTER=4
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
        osg::Quat   qrot;
        osg::Vec3   pos;
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
        void                    fillFromCharacter(Character *f);
private:
        void                    storeCreation(BitStream & bs) const;
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
