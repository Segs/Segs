#pragma once
#include "CommonNetStructures.h"
#include "Powers.h"
#include "Costume.h"
#include "Character.h"

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>
struct AngleRadians
{
    static AngleRadians fromDeg(float deg) { return AngleRadians(deg*float(M_PI)/180.0f);}
    float toDeg() { return AngleRadians((v*180.0f)/ float(M_PI)).v;}
    explicit AngleRadians(float x=0.0f) : v(x) {}
    AngleRadians operator-(const AngleRadians&ot) const
    {
        AngleRadians result(v);
        return result-=ot;
    }
    AngleRadians operator-() const {
        return AngleRadians(-v);
    }
    float operator/(AngleRadians &other) const {
        return v/other.v;
    }
    AngleRadians operator+(const AngleRadians &ot) const
    {
        AngleRadians result(v);
        result+=ot;
        result.fixup();
        return result;
    }
    AngleRadians operator*(float scale) const
    {
        return AngleRadians(v*scale);
    }
    AngleRadians &operator*=(float scale)
    {
        v*=scale;
        return *this;
    }
    AngleRadians &operator+=(const AngleRadians &ot)
    {
        v += ot.v;
        fixup();
        return *this;
    }
    AngleRadians &operator-=(const AngleRadians &ot)
    {
        v -= ot.v;
        fixup();
        return *this;
    }
    bool operator==(float other) const { return v == other; }
    bool operator==(const AngleRadians &other) const { return v == other.v; }
    bool operator!=(const AngleRadians &other) const { return v != other.v; }
    AngleRadians &fixup() {
        if ( v > float(M_PI))
            v -= 2* float(M_PI);
        if ( v <= -float(M_PI))
            v += 2* float(M_PI);
        return *this;
    }
    bool operator<( const AngleRadians &o) const {
        return v<o.v;
    }
    bool operator>( const AngleRadians &o) const {
        return v>o.v;
    }
    AngleRadians lerp(AngleRadians towards,float factor) const {

        float v3(towards.v - v);
        if ( v3 > float(M_PI))
            v3 = v3 - 2 * float(M_PI);
        if ( v3 <= -float(M_PI))
            v3 = v3 + 2 * float(M_PI);
        return AngleRadians(v3 * factor + v);

    }
    //    operator float()
    //    { return v;}
    float v;
    int toIntegerForm() const
    {
        return int((v + float(M_PI)) * 2048.0f / (2* float(M_PI)) + 0.5f);
    }
    explicit operator float() const {
        return v;
    }
};

class PosUpdate
{
public:
    glm::vec3 posvec;
    AngleRadians PitchYawRoll[3];
    //glm::quat quat;
    int m_timestamp;
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
    glm::vec3 camera_pyr;
    int m_t1,m_t2;
    uint8_t field_20;
    int m_A_ang11_probably,m_B_ang11_probably;

    bool has_input_commit_guess;
    bool pos_delta_valid[3];
    bool pyr_valid[3];
    glm::vec3 pos_delta;
    glm::quat direction;
    InputStateStorage & operator=(const InputStateStorage &other);
    void processDirectionControl(int dir, int prev_time, int press_release);
};
enum class FadeDirection {
    In,
    Out
};
class Entity : public NetStructure
{
public:
        struct currentInputState
        {
            glm::vec3 pos;
            glm::vec3 pyr; //TODO: convert to quat
        };
        InputStateStorage   inp_state;
        enum
        {
            ENT_PLAYER=2,
            ENT_CRITTER=4
        };
        int                 m_access_level;
        int                 m_randSeed; // Sequencer uses this as a seed for random bone scale
        int                 field_68;
        int                 field_78;
        int                 m_num_titles=0;
        int                 m_num_fx;
        bool                m_is_logging_out = false;
mutable bool                m_logout_sent;
        int                 m_time_till_logout; // time in miliseconds untill given entity should be marked as logged out.
        bool                m_has_titles = false;
        std::vector<uint8_t> m_fx1;
        std::vector<uint32_t> m_fx2;
        std::vector<uint8_t> m_fx3;
        uint8_t             m_costume_type;
        int                 m_state_mode=0;
        bool                m_state_mode_send=false;
        bool                m_odd_send=false;
        bool                m_SG_info = false;
        bool                m_seq_update=false;
        bool                m_is_villian = false;
        bool                m_contact = false;
        int                 m_seq_upd_num1=0;
        int                 m_seq_upd_num2=0;
        PosUpdate           m_pos_updates[64];
        size_t              m_update_idx;
        std::vector<PosUpdate> interpResults;

        QString             m_battle_cry;
        QString             m_character_description;
        bool                var_B4;

        Character           m_char;

        bool                entReceiveAlwaysCon = false;
        bool                entReceiveSeeThroughWalls = false;
        int                 pkt_id_QrotUpdateVal[3];
        glm::quat           qrot;
        glm::vec3           pos;
        uint32_t            prev_pos[3];
        bool                m_selector1 = false;
        bool                m_pchar_things = false;
        bool                might_have_rare = false;
        bool                m_hasname = false;
        bool                m_hasgroup_name = false;
        bool                m_classname_override = false;
        bool                m_change_existence_state = false;
        bool                m_hasRagdoll = false;
        bool                m_create_player = false;
        bool                m_rare_bits = false;
        int                 current_client_packet_id;
        QString             m_group_name, m_override_name;
        uint8_t             m_origin_idx,m_class_idx;
        uint8_t             m_type;
        int32_t             m_idx;
        uint32_t            m_db_id;
        uint32_t            m_input_ack;
        bool                player_type = false;
        bool                m_player_villain = false;
        bool                m_destroyed = false;
        int                 ownerEntityId = 0;
        int                 creatorEntityId = 0;
        float               translucency = 1.f;
        bool                m_is_fading = true;
        FadeDirection       m_fading_direction=FadeDirection::In;
                            Entity();
virtual                     ~Entity() = default;
        void                dump();
        void                addPosUpdate(const PosUpdate &p);
        void                addInterp(const PosUpdate &p);

        int32_t             getIdx() const {return m_idx;}
virtual void                serializeto(BitStream &bs)const;
virtual void                sendCostumes(BitStream &bs) const;
static  void                sendAllyID(BitStream &bs);
static  void                sendPvP(BitStream &bs);

virtual void                serializefrom(BitStream &) {assert(false);}
        bool                update_rot(int axis) const; // returns true if given axis needs updating;

        void                InsertUpdate(PosUpdate pup);
        const QString &     name() const {return m_char.getName();}
        void                sendBuffs(BitStream &bs) const;
        void                fillFromCharacter(Character *f);
        void                beginLogout(uint16_t time_till_logout=10); // Default logout time is 10 s
private:
        int                 getOrientation(BitStream &bs);
        void                sendAFK(BitStream &bs)const;
        void                sendBuffsConditional(BitStream &bs) const;
        void                sendCharacterStats(BitStream &bs) const;
        void                sendContactOrPnpc(BitStream &bs)const;
        void                sendEntCollision(BitStream &bs) const;
        void                sendLogoutUpdate(BitStream &bs)const;
        void                sendNetFx(BitStream &bs) const;
        void                sendNoDrawOnClient(BitStream &bs)const;
        void                sendOnOddSend(BitStream &bs,bool is_odd=true) const;
        void                sendOtherSupergroupInfo(BitStream &bs)const;
        void                sendPetName(BitStream &bs)const;
        void                sendRagDoll(BitStream &bs) const;
        void                sendSeqMoveUpdate(BitStream &bs) const;
        void                sendSeqTriggeredMoves(BitStream &bs) const;
        void                sendStateMode(BitStream &bs) const;
        void                sendTargetUpdate(BitStream &bs) const;
        void                sendTitles(BitStream &bs) const;
        void                sendWhichSideOfTheForce(BitStream &bs) const;
        void                sendXLuency(BitStream &bs,float xluency) const;
        void                storeCreation(BitStream & bs) const;
        void                storeOrientation(BitStream &bs) const;
        void                storePosition(BitStream &bs) const;
        void                storePosUpdate(BitStream &bs) const;
        void                storeUnknownBinTree(BitStream &bs) const;
};
class NpcEntity : public Entity
{
    NpcEntity()
    {
        m_costume_type=2;
    }
};
class PlayerEntity : public Entity
{
public:
                            PlayerEntity();
        void                serializefrom_newchar(BitStream &src);
        void                serialize_full(BitStream &bs );
};
