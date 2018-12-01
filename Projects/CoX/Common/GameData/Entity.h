/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "CharacterData.h"
#include "CommonNetStructures.h"
#include "Costume.h"
#include "FixedPointValue.h"
#include "Movement.h"
#include "Powers.h"
#include "Team.h"
#include "StateInterpolator.h"
#include "StateStorage.h"
#include "Common/GameData/ClientStates.h"
#include "Common/GameData/entitydata_definitions.h"
#include "Common/GameData/seq_definitions.h"
#include "Common/GameData/CoHMath.h"
#include "Common/GameData/Contact.h"
#include "Common/GameData/Store.h"

#include <glm/gtc/constants.hpp>

#include <QQueue>
#include <array>
#include <memory>

struct MapClientSession;
class Team;
class Trade;
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
    QString         m_SG_motto;
    QString         m_SG_motd;
    QString         m_SG_emblem;         // 128 chars max -> hash table key from the CostumeString_HTable
    uint32_t        m_SG_color1     = 0; // supergroup color 1
    uint32_t        m_SG_color2     = 0; // supergroup color 2
    int             m_SG_rank       = 1;
};

struct FactionData
{
    bool    m_has_faction = false;  // send Faction info
    int     m_rank = 0;             // iRank
    QString m_faction_name;         // group_name
};

struct NPCData
{
    bool m_is_owned = false;
    const struct Parse_NPC *src_data;
    int npc_idx=0;
    int costume_variant=0;
};

struct NetFxTarget
{
    bool        type_is_location = false;
    uint32_t    ent_idx = 0;
    glm::vec3   pos;
};

struct NetFx
{
    uint8_t     command;
    uint32_t    net_id;
    uint32_t    handle;
    bool        pitch_to_target     = false;
    uint8_t     bone_id;
    float       client_timer        = 0;
    int         client_trigger_fx   = 0;
    float       duration            = 0;
    float       radius              = 0;
    int         power               = 0; // char.toInt()
    int         debris              = 0;
    NetFxTarget target;
    NetFxTarget origin;
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
    using TradePtr = std::shared_ptr<Trade>;
private:
                            Entity();
                            ~Entity();
public:
        StateStorage        m_states;
        MotionState         m_motion_state;
        // Some entities might not have a character data ( doors, cars )
        // Making it an unique_ptr<Character> makes it clear that Entity 'owns'
        // and takes care of this data, at the same time it can be missing
        CharacterPtr        m_char;
        // And not all entities are players
        PlayerPtr           m_player;
        EntityPtr           m_entity;
        NPCPtr              m_npc;

        SuperGroup          m_supergroup;                       // client has this in entity class, but maybe move to Character class?
        bool                m_has_supergroup        = true;
        bool                m_has_team              = false;
        Team *              m_team                  = nullptr;  // we might want t move this to Character class, but maybe Baddies use teams?
        TradePtr            m_trade;
        EntityData          m_entity_data;
        FactionData         m_faction_data;

        uint32_t            m_idx                   = {0};
        uint32_t            m_db_id                 = {0};
        EntType             m_type                  = {EntType::Invalid};
        glm::quat           m_direction;
        int32_t             m_target_idx            = -1;
        int32_t             m_assist_target_idx     = -1;

        std::vector<Buffs>          m_buffs;
        QQueue<QueuedPowers>        m_queued_powers;
        std::vector<QueuedPowers>   m_recharging_powers;
        PowerStance                 m_stance;
        bool                        m_update_buffs  = false;


        // Animations: Sequencers, NetFx, and TriggeredMoves
        std::vector<NetFx>  m_net_fx;
        std::vector<TriggeredMove> m_triggered_moves;
        SeqBitSet           m_seq_state;                    // Should be part of SeqState
        ClientStates        m_state_mode            = ClientStates::SIMPLE;
        int                 m_seq_move_idx          = 0;
        int                 m_randSeed              = 0;     // Sequencer uses this as a seed for random bone scale
        bool                m_has_state_mode        = false;
        bool                m_seq_update            = false;
        uint8_t             m_seq_move_change_time  = 0;
        uint8_t             m_move_type             = 0;

        int                 m_time_till_logout      = 0;    // time in miliseconds untill given entity should be marked as logged out.
        AppearanceType      m_costume_type          = AppearanceType::None;
        bool                m_is_logging_out        = false;
        bool                m_odd_send              = false;
        bool                m_no_draw_on_client     = false;
        bool                m_force_camera_dir      = false; // used to force the client camera direction in sendClientData()
        bool                m_is_hero               = false;
        bool                m_is_villian            = false;
        bool                m_contact               = false;
        uint8_t             m_update_id             = 1;
         bool               m_update_part_1         = true;     // EntityResponse sendServerControlState
        bool                m_force_pos_and_cam     = true;     // EntityResponse sendServerControlState
        bool                m_full_update           = true;     // EntityReponse sendServerPhysicsPositions
        bool                m_has_control_id        = true;     // EntityReponse sendServerPhysicsPositions
        bool                m_has_interp            = false;    // EntityUpdateCodec storePosUpdate
        bool                m_move_instantly        = false;    // EntityUpdateCodec storePosUpdate
        bool                m_in_training           = false;
        bool                m_has_input_on_timeframe= false;
        bool                m_is_using_mapmenu      = false;

        int                 u1 = 0; // used for live-debugging

        std::array<PosUpdate, 64> m_pos_updates;
        std::array<BinTreeEntry, 7> m_interp_bintree;
        size_t              m_update_idx                = 0;
        bool                m_pchar_things              = false;
        bool                m_update_anims              = false;
        bool                m_hasname                   = false;
        bool                m_classname_override        = false;
        bool                m_hasRagdoll                = false;
        bool                m_has_owner                 = false;
        bool                m_create_player             = false;
        bool                m_rare_update               = false;
        int                 m_input_pkt_id              = {0};
        uint32_t            m_input_ack                 = {0};
        uint32_t            ownerEntityId               = 0;
        uint32_t            creatorEntityId             = 0;
        MapClientSession *  m_client                    = nullptr;
        FadeDirection       m_fading_direction          = FadeDirection::In;
        uint32_t            m_db_store_flags            = 0;
        Destination         m_cur_destination;
        float               translucency                = 1.0f;
        bool                player_type                 = false;
        bool                m_destroyed                 = false;
        bool                m_is_fading                 = true;
        bool                m_is_store                  = false;
        vStoreItems         m_store_items;

        std::function<void(int)>  m_active_dialog      = NULL;

        void                dump();

static  void                sendAllyID(BitStream &bs);
static  void                sendPvP(BitStream &bs);

        const QString &     name() const;
        void                fillFromCharacter(const GameDataStore &data);
        void                beginLogout(uint16_t time_till_logout=10); // Default logout time is 10 s
        void                setActiveDialogCallback(std::function<void(int)> callback);
};
