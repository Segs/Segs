#pragma once
#include "CommonNetStructures.h"
#include "Powers.h"
#include "Costume.h"
#include "Character.h"
#include "FixedPointValue.h"

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

#include <cmath>
#include <array>
class MapClient;
struct AngleRadians // TODO: Is this intended to be used?
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
        return int((v + float(M_PI)) * 2048.0f / (2* float(M_PI)));
    }
    float fromIntegerForm(/*int v*/) const
    {
        return (float(v)/2048.0f)*(2*M_PI) - M_PI;
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
    InputStateStorage()
    {
        for(int i=0; i<3; ++i)
        {
            pos_delta_valid[i]=false;
            pyr_valid[i]=false;
        }
    }
    uint8_t m_csc_deltabits             = 0;
    bool m_send_deltas                  = 0;
    uint16_t controlBits                = 0;
    uint16_t send_id                    = 0;
    void *current_state_P               = 0;
    glm::vec3 camera_pyr;
    int m_time_diff1                    = 0;
    int m_time_diff2                    = 0;
    uint8_t input_vel_scale             = 0; // TODO: Should be float?
    float m_orientation_pyr[3]          = {}; // Stored in Radians
    uint8_t m_received_server_update_id = 0;
    bool m_no_coll                      = false;
    bool has_input_commit_guess         = 0;
    bool pos_delta_valid[3]             = {};
    bool pyr_valid[3]                   = {};
    glm::vec3 pos_delta;
    glm::quat direction;
    InputStateStorage & operator=(const InputStateStorage &other);
    void processDirectionControl(int dir, int prev_time, int press_release);
};
enum class FadeDirection
{
    In,
    Out
};
class Entity
{
    // only EntityStore can create instances of this class
    friend class EntityStore;
    friend class std::array<Entity,10240>;
private:
                            Entity();
virtual                     ~Entity() = default;
public:
        struct currentInputState
        {
            glm::vec3 pos;
            glm::vec3 pyr; //TODO: convert to quat
        };
        InputStateStorage   inp_state;
        enum EntType
        {
            ENT_PLAYER=2,
            ENT_CRITTER=4
        };
        int                 m_access_level      = 0;
        int                 m_randSeed          = 0; // Sequencer uses this as a seed for random bone scale
        int                 m_SG_id             = 0;
        int                 m_num_titles        = 0;
        int                 m_num_fx            = 0;
        bool                m_is_logging_out    = false;
        int                 m_time_till_logout  = 0; // time in miliseconds untill given entity should be marked as logged out.
        bool                m_has_titles        = false;
        std::vector<uint8_t> m_fx1;
        std::vector<uint32_t> m_fx2;
        std::vector<uint8_t> m_fx3;
        uint8_t             m_costume_type      = 0;
        int                 m_state_mode        = 0;
        bool                m_state_mode_send   = false;
        bool                m_odd_send          = false;
        bool                m_SG_info           = false;
        bool                m_seq_update        = false;
        bool                m_is_villian        = false;
        bool                m_contact           = false;
        int                 m_seq_upd_num1      = 0;
        int                 m_seq_upd_num2      = 0;
        PosUpdate           m_pos_updates[64];
        size_t              m_update_idx        = 0;
        std::vector<PosUpdate> interpResults;
        bool                m_has_the_prefix    = false;
        QString             m_battle_cry;
        QString             m_character_description;

        Character           m_char;

        bool                entReceiveAlwaysCon = false;
        bool                entReceiveSeeThroughWalls = false;
        int                 pkt_id_QrotUpdateVal[3] = {0};
        glm::quat           qrot;
        glm::vec3           pos;
        glm::vec3           vel;
        uint32_t            prev_pos[3]         = {0};
        Vector3_FPV         fixedpoint_pos;
        bool                m_selector1         = false;
        bool                m_pchar_things      = false;
        bool                might_have_rare     = false;
        bool                m_hasname           = false;
        bool                m_hasgroup_name     = false;
        bool                m_classname_override = false;
        bool                m_hasRagdoll        = false;
        bool                m_create_player     = false;
        bool                m_rare_bits         = false;
        int                 current_client_packet_id= {0};
        QString             m_group_name, m_override_name;
        uint8_t             m_origin_idx        = {0};
        uint8_t             m_class_idx         = {0};
        uint8_t             m_type              = {0};
        int32_t             m_idx               = {0};
        uint32_t            m_db_id             = {0};
        uint32_t            m_input_ack         = {0};
        bool                player_type         = false;
        bool                m_player_villain    = false;
        bool                m_destroyed         = false;
        int                 ownerEntityId       = 0;
        int                 creatorEntityId     = 0;
        float               translucency        = 1.f;
        bool                m_is_fading         = true;
        MapClient *         m_client            = nullptr;
        FadeDirection       m_fading_direction  = FadeDirection::In;
        void                dump();
        void                addPosUpdate(const PosUpdate &p);
        void                addInterp(const PosUpdate &p);

        int32_t             getIdx() const {return m_idx;}
static  void                sendAllyID(BitStream &bs);
static  void                sendPvP(BitStream &bs);

        bool                update_rot(int axis) const; // returns true if given axis needs updating;

        void                InsertUpdate(PosUpdate pup);
        const QString &     name() const {return m_char.getName();}
        void                fillFromCharacter(Character *f);
        void                beginLogout(uint16_t time_till_logout=10); // Default logout time is 10 s
};
void initializeNewPlayerEntity(Entity &e);
void fillEntityFromNewCharData(Entity &e,BitStream &src, ColorAndPartPacker *packer);
extern void abortLogout(Entity *e);
