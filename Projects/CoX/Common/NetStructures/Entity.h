/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CommonNetStructures.h"
#include "Costume.h"
#include "Movement.h"
#include "Powers.h"
#include "Team.h"
#include "InputStates.h"
#include "FixedPointValue.h"
#include "Common/GameData/entitydata_definitions.h"
#include "Common/GameData/chardata_definitions.h"
#include "Common/GameData/CoHMath.h"

//#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <memory>

struct MapClientSession;
class Team;
class Character;
struct PlayerData;
using Parse_AllKeyProfiles = std::vector<struct Keybind_Profiles>;

enum class FadeDirection
{
    In,
    Out
};

// returned by getEntityFromDB()
struct CharacterFromDB
{
    QString         name;
    EntityData      entity_data;
    CharacterData   char_data;
    float           hitpoints;
    float           endurance;
    uint32_t        sg_id;
    uint32_t        m_db_id;
};

enum class EntType : uint8_t
{
    Invalid = 0,
    NPC     = 1,
    PLAYER  = 2,
    HERO    = 3,
    CRITTER = 4,
    CAR     = 5,
    DELIVERYTARGET = 6,
    MOBILEGEOMETRY = 7,
    MISSION_ITEM   = 8,
    MAPXFERDOOR    = 9,
    DOOR           = 10,
    COUNT          = 11
};

enum class AppearanceType : uint8_t
{
    None          = 0,
    WholeCostume  = 1,
    NpcCostume    = 2,
    VillainIndex  = 3,
    SequencerName = 4
};

struct SuperGroup
{
    int             m_SG_id         = {0};
    QString         m_SG_name       = "Supergroup"; // 64 chars max
    //QString         m_SG_motto;
    //QString         m_SG_costume;                 // 128 chars max -> hash table key from the CostumeString_HTable
    uint32_t        m_SG_color1     = 0;            // supergroup color 1
    uint32_t        m_SG_color2     = 0;            // supergroup color 2
    int             m_SG_rank       = 1;
};

struct NPCData
{
    bool m_is_owned = false;
    const struct Parse_NPC *src_data;
    int npc_idx=0;
    int costume_variant=0;
};

struct TriggeredMove
{
    uint32_t m_num_moves = 0;
    uint32_t m_move_idx = 0;
    uint32_t m_ticks_to_delay = 0;
    uint32_t m_trigger_fx_idx = 0;
};

struct NetFx
{
    uint8_t command;
    uint32_t net_id;
    uint32_t handle;
    bool pitch_to_target;
    uint8_t bone_id;
};

class Entity
{
    // only EntityStore can create instances of this class
    friend class EntityStore;
    friend std::array<Entity,10240>;
    using CharacterPtr = std::unique_ptr<Character>;
    using PlayerPtr = std::unique_ptr<PlayerData>;
    using EntityPtr = std::unique_ptr<EntityData>;
    using NPCPtr = std::unique_ptr<NPCData>;
private:
                            Entity();
virtual                     ~Entity();
public:
        StateStorage        m_states;
        InputState          *m_cur_state            = nullptr;
        InputState          *m_prev_state           = nullptr;
        MotionState         m_motion_state;
        // Some entities might not have a character data ( doors, cars )
        // Making it an unique_ptr<Character> makes it clear that Entity 'owns'
        // and takes care of this data, at the same time it can be missing
        CharacterPtr        m_char;
        // And not all entities are players
        PlayerPtr           m_player;
        EntityPtr           m_entity;
        NPCPtr              m_npc;

        EntityData          m_entity_data;
        Team *              m_team                  = nullptr;  // we might want t move this to Character class, but maybe Baddies use teams?
        SuperGroup          m_supergroup;                       // client has this in entity class, but maybe move to Character class?
        bool                m_has_team              = false;
        bool                m_has_supergroup        = true;

        uint32_t            m_idx                   = {0};
        uint32_t            m_db_id                 = {0};
        EntType             m_type                  = {EntType::Invalid};
        glm::quat           m_direction;
        glm::vec3           m_speed                 = {1,1,1};
        uint32_t            m_target_idx            = 0;
        uint32_t            m_assist_target_idx     = 0;

        int                 m_randSeed              = 0;    // Sequencer uses this as a seed for random bone scale
        int                 m_num_fx                = 0;
        bool                m_is_logging_out        = false;
        int                 m_time_till_logout      = 0;    // time in miliseconds untill given entity should be marked as logged out.
        std::vector<NetFx>  m_fx1;
        TriggeredMove       m_triggered_moves[20];
        AppearanceType      m_costume_type          = AppearanceType::None;
        int                 m_state_mode            = 0;
        bool                m_state_mode_send       = false;
        bool                m_odd_send              = false;
        bool                m_no_draw_on_client     = false;
        bool                m_force_camera_dir      = false; // used to force the client camera direction in sendClientData()
        bool                m_is_hero               = false;
        bool                m_is_villian            = false;
        bool                m_contact               = false;
        bool                m_seq_update            = false;
        int                 m_seq_move_idx          = 0;
        uint8_t             m_seq_move_change_time  = 0;

        bool                m_update_pos_and_cam    = false;     // EntityResponse sendServerControlState
        bool                m_full_update           = true;     // EntityReponse sendServerPhysicsPositions
        bool                m_has_control_id        = true;     // EntityReponse sendServerPhysicsPositions
        bool                m_extra_info            = false;    // EntityUpdateCodec storePosUpdate
        bool                m_move_instantly        = false;    // EntityUpdateCodec storePosUpdate

        float               u1 = 24.0f; // used for live-debugging

        PosUpdate           m_pos_updates[64];
        std::vector<PosUpdate> m_interp_results;
        size_t              m_update_idx                = 0;
        glm::vec3           m_velocity;
        glm::vec3           m_prev_pos;
        Vector3_FPV         fixedpoint_pos;
        bool                m_pchar_things              = false;
        bool                might_have_rare             = false;
        bool                m_hasname                   = false;
        bool                m_classname_override        = false;
        bool                m_hasRagdoll                = false;
        bool                m_has_owner                 = false;
        bool                m_create_player             = false;
        bool                m_rare_bits                 = false;
        int                 current_client_packet_id    = {0};
        QString             m_override_name;
        uint32_t            m_input_ack                 = {0};
        bool                player_type                 = false;
        bool                m_destroyed                 = false;
        uint32_t            ownerEntityId               = 0;
        uint32_t            creatorEntityId             = 0;
        float               translucency                = 1.0f;
        bool                m_is_fading                 = true;
        MapClientSession *  m_client                    = nullptr;
        FadeDirection       m_fading_direction          = FadeDirection::In;
        uint32_t            m_db_store_flags            = 0;

        void                dump();
        void                addPosUpdate(const PosUpdate &p);
        void                addInterp(const PosUpdate &p);

static  void                sendAllyID(BitStream &bs);
static  void                sendPvP(BitStream &bs);

        bool                update_rot(int axis) const; // returns true if given axis needs updating;

        const QString &     name() const;
        void                fillFromCharacter();
        void                beginLogout(uint16_t time_till_logout=10); // Default logout time is 10 s
};

enum class DbStoreFlags : uint32_t
{
    PlayerData = 1,
    Full       = ~0U,
};

void markEntityForDbStore(Entity *e,DbStoreFlags f);
void unmarkEntityForDbStore(Entity *e, DbStoreFlags f);
void initializeNewPlayerEntity(Entity &e);
void initializeNewNpcEntity(Entity &e, const Parse_NPC *src, int idx, int variant);
void fillEntityFromNewCharData(Entity &e, BitStream &src, const ColorAndPartPacker *packer, const Parse_AllKeyProfiles &default_profiles);
void forcePosition(Entity &e,glm::vec3 pos);
extern void abortLogout(Entity *e);
