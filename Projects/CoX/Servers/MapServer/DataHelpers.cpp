#include "DataHelpers.h"

#include "MapServer.h"
#include "MapServerData.h"
#include "MapInstance.h"
#include "MapClient.h"
#include "AdminServer/AdminServer.h"
#include "AdminServer/CharacterDatabase.h"
#include "Team.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>

/*
 * Entity Methods
 */
// Getters
uint32_t    getIdx(const Entity &e) { return e.m_idx; }
uint32_t    getDbId(const Entity &e) { return e.m_db_id; }
uint32_t    getAccessLevel(const Entity &e) { return e.m_entity_data.m_access_level; }
uint32_t    getTargetIdx(const Entity &e) { return e.m_target_idx; }
uint32_t    getAssistTargetIdx(const Entity &e) { return e.m_assist_target_idx; }
glm::vec3   getSpeed(const Entity &e) { return e.m_spd; }
float       getBackupSpd(const Entity &e) { return e.m_backup_spd; }
float       getJumpHeight(const Entity &e) { return e.m_jump_height; }
uint8_t     getUpdateId(const Entity &e) { return e.m_update_id; }

// Setters
void    setDbId(Entity &e, uint8_t val) { e.m_char.m_db_id = val; e.m_db_id = val; }
void    setSpeed(Entity &e, float v1, float v2, float v3) { e.m_spd = {v1,v2,v3}; }
void    setBackupSpd(Entity &e, float val) { e.m_backup_spd = val; }
void    setJumpHeight(Entity &e, float val) { e.m_jump_height = val; }
void    setUpdateID(Entity &e, uint8_t val) { e.m_update_id = val;}

void    setTeamID(Entity &e, uint8_t team_id)
{
    if(team_id == 0)
    {
        e.m_has_team            = false;
        delete e.m_team;
    }
    else
        e.m_has_team            = true;

    qDebug().noquote() << "Team Info:"
             << "\n  Has Team:" << e.m_has_team
             << "\n  ID:" << e.m_team->m_team_idx
             << "\n  Size:" << e.m_team->m_team_members.size()
             << "\n  Members:" << e.m_team->m_team_members.data();
}

void    setSuperGroup(Entity &e, uint8_t sg_id, QString sg_name, uint32_t sg_rank)
{
    // TODO: provide method for updating SuperGroup Colors
    if(sg_id == 0)
    {
        e.m_has_supergroup          = false;
        e.m_supergroup.m_SG_id      = sg_id;
        e.m_supergroup.m_SG_name    = "";
        e.m_supergroup.m_SG_color1  = 0x996633FF;
        e.m_supergroup.m_SG_color2  = 0x336699FF;
        e.m_supergroup.m_SG_rank    = 0;
    }
    else
    {
        e.m_has_supergroup          = true;
        e.m_supergroup.m_SG_id      = sg_id;
        e.m_supergroup.m_SG_name    = sg_name;
        e.m_supergroup.m_SG_color1  = 0xAA3366FF;
        e.m_supergroup.m_SG_color2  = 0x66AA33FF;
        e.m_supergroup.m_SG_rank    = sg_rank;
    }
    qDebug().noquote() << "SG Info:"
             << "\n  Has Team:" << e.m_has_supergroup
             << "\n  ID:" << e.m_supergroup.m_SG_id
             << "\n  Name:" << e.m_supergroup.m_SG_name
             << "\n  Color1:" << e.m_supergroup.m_SG_color1
             << "\n  Color2:" << e.m_supergroup.m_SG_color2
             << "\n  Rank:" << e.m_supergroup.m_SG_rank;
}

void    setAssistTarget(Entity &e) { e.m_target_idx = getAssistTargetIdx(e); }

// For live debugging
void    setu1(Entity &e, int val) { e.u1 = val; }
void    setu2(Entity &e, int val) { e.u2 = val; }
void    setu3(Entity &e, int val) { e.u3 = val; }
void    setu4(Entity &e, int val) { e.u4 = val; }
void    setu5(Entity &e, int val) { e.u5 = val; }
void    setu6(Entity &e, int val) { e.u6 = val; }

// Toggles
void    toggleFly(Entity &e) { e.m_is_flying = !e.m_is_flying; }
void    toggleFalling(Entity &e) { e.m_is_falling = !e.m_is_falling; }
void    toggleJumping(Entity &e) { e.m_is_jumping = !e.m_is_jumping; }
void    toggleSliding(Entity &e) { e.m_is_sliding = !e.m_is_sliding; }

void toggleStunned(Entity &e)
{
    e.m_is_stunned = !e.m_is_stunned;
    // TODO: toggle stunned FX above head
}

void toggleJumppack(Entity &e)
{
    e.m_has_jumppack = !e.m_has_jumppack;
    // TODO: toggle costume part for jetpack back item.
}

void    toggleControlsDisabled(Entity &e) { e.m_controls_disabled = !e.m_controls_disabled; }
void    toggleFullUpdate(Entity &e) { e.m_full_update = !e.m_full_update; }
void    toggleControlId(Entity &e) { e.m_has_control_id = !e.m_has_control_id; }

// Misc Methods
void charUpdateDB(Entity *e)
{
    CharacterDatabase *char_db = AdminServer::instance()->character_db();
    // Update Character In Database
    if(!char_db->update(e))
        qDebug() << "Character failed to update in database!";
}

void charUpdateOptions(Entity *e)
{
    CharacterDatabase *char_db = AdminServer::instance()->character_db();
    // Update Client Options/Keybinds
    if(!char_db->updateClientOptions(e))
        qDebug() << "Client Options failed to update in database!";
}

void charUpdateGUI(Entity *e)
{
    CharacterDatabase *char_db = AdminServer::instance()->character_db();
    // Update Client GUI settings
    if(!char_db->updateGUISettings(e))
        qDebug() << "Client GUISettings failed to update in database!";
}

int getEntityOriginIndex(bool is_player, const QString &origin_name)
{
    const MapServerData &data(g_GlobalMapServer->runtimeData());
    const Parse_AllOrigins &origins_to_search(is_player ? data.m_player_origins : data.m_other_origins);

    int idx=0;
    for(const Parse_Origin &orig : origins_to_search)
    {
        if(orig.Name.compare(origin_name,Qt::CaseInsensitive)==0)
            return idx;
        idx++;
    }
    qWarning() << "Failed to locate origin index for"<<origin_name;
    return 0;
}

int getEntityClassIndex(bool is_player, const QString &class_name)
{
    const MapServerData &data(g_GlobalMapServer->runtimeData());
    const Parse_AllCharClasses &classes_to_search(is_player ? data.m_player_classes : data.m_other_classes);

    int idx=0;
    for(const CharClass_Data &classdata : classes_to_search)
    {
        if(classdata.m_Name.compare(class_name,Qt::CaseInsensitive)==0)
            return idx;
        idx++;
    }
    qWarning() << "Failed to locate class index for"<<class_name;
    return 0;

}

// Poll EntityManager to return Entity by Name or IDX
Entity * getEntity(MapClient *src, const QString &name)
{
    MapInstance *mi = src->current_map();
    EntityManager &em(mi->m_entities);

    // Iterate through all active entities and return entity by name
    for (Entity* pEnt : em.m_live_entlist)
    {
        if (pEnt->name() == name)
            return pEnt;
    }
    qWarning() << "Entity" << name << "does not exist, or is not currently online.";
    return nullptr;
}

Entity * getEntity(MapClient *src, const int32_t &idx)
{
    MapInstance *mi = src->current_map();
    EntityManager &em(mi->m_entities);

    if(idx==0) {
        qWarning() << "Entity" << idx << "does not exist, or is not currently online.";
        return nullptr;
    }
    // Iterate through all active entities and return entity by idx
    for (Entity* pEnt : em.m_live_entlist)
    {
        if (pEnt->m_idx == idx)
            return pEnt;
    }
    qWarning() << "Entity" << idx << "does not exist, or is not currently online.";
    return nullptr;
}

Entity * getEntityByDBID(MapClient *src, const int32_t &idx)
{
    MapInstance *mi = src->current_map();
    EntityManager &em(mi->m_entities);

    if(idx==0) {
        qWarning() << "Entity" << idx << "does not exist in the database.";
        return nullptr;
    }
    // TODO: Iterate through all entities in Database and return entity by db_id
    for (Entity* pEnt : em.m_live_entlist)
    {
        if (pEnt->m_db_id == idx)
            return pEnt;
    }
    qWarning() << "Entity with db_id" << idx << "does not exist, or is not currently online.";
    return nullptr;
}

void sendServerMOTD(Entity *e)
{
    if(!e->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }

    MapClient *src = e->m_client;
    qDebug().noquote() << "Sending Server MOTD to" << e->m_char.getName();

    QString fileName("scripts/motd.smlx");
    QFile file(fileName);
    if(file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString contents(file.readAll());
        StandardDialogCmd *dlg = new StandardDialogCmd(contents);
        src->addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
    }
    else {
        QString errormsg = "Failed to load MOTD file. \'" + file.fileName() + "\' not found.";
        qDebug() << errormsg;
        sendInfoMessage(MessageChannel::DEBUG_INFO, errormsg, src);
    }
}


/*
 * Character Methods
 */
// Getter
uint32_t            getLevel(const Character &c) { return c.m_char_data.m_level; }
uint32_t            getCombatLevel(const Character &c) { return c.m_char_data.m_combat_level; }
uint32_t            getHP(const Character &c) { return c.m_current_attribs.m_HitPoints; }
uint32_t            getEnd(const Character &c) { return c.m_current_attribs.m_Endurance; }
uint64_t            getLastCostumeId(const Character &c) { return c.m_char_data.m_last_costume_id; }
const QString &     getOrigin(const Character &c) { return c.m_char_data.m_origin_name; }
const QString &     getClass(const Character &c) { return c.m_char_data.m_class_name; }
const QString &     getMapName(const Character &c) { return c.m_char_data.m_mapName; }
uint32_t            getXP(const Character &c) { return c.m_char_data.m_experience_points; }
uint32_t            getDebt(const Character &c) { return c.m_char_data.m_experience_debt; }
uint32_t            getPatrolXP(const Character &c) { return c.m_char_data.m_experience_patrol; }
const QString &     getTitles(const Character &c) { return c.m_char_data.m_titles[3]; }
uint32_t            getInf(const Character &c) { return c.m_char_data.m_influence; }
const QString &     getDescription(const Character &c) { return c.m_char_data.m_character_description ; }
const QString &     getBattleCry(const Character &c) { return c.m_char_data.m_battle_cry; }
const QString &     getAlignment(const Character &c) { return c.m_char_data.m_alignment; }

// Setters
void setLevel(Character &c, uint32_t val)
{
    if(val>50)
        val = 50;
    c.m_char_data.m_level = val;
    // TODO: set max attribs based upon level
    //MapServerData map_server_data;
    //c.m_experience_points = map_server_data.expForLevel(val);
}

void setCombatLevel(Character &c, uint32_t val)
{
    if(val>50)
        val = 50;
    c.m_char_data.m_combat_level = val;
}
void    setHP(Character &c, uint32_t val) { c.m_current_attribs.m_HitPoints = val; }
void    setEnd(Character &c, uint32_t val) { c.m_current_attribs.m_Endurance = val; }
void    setLastCostumeId(Character &c, uint64_t val) { c.m_char_data.m_last_costume_id = val; }
void    setMapName(Character &c, const QString &val) { c.m_char_data.m_mapName = val; }

void setXP(Character &c, uint32_t val)
{
    c.m_char_data.m_experience_points = val;
    for(auto const& lvl: c.m_other_attribs.m_ExperienceRequired)
    {
        if(val>=lvl && val<lvl+1)
        {
            setLevel(c, lvl);
            // TODO: set max attribs based upon level.
        }
    }
}

void    setDebt(Character &c, uint32_t val) { c.m_char_data.m_experience_debt = val; }

void setTitles(Character &c, bool prefix, QString generic, QString origin, QString special)
{
    // if "NULL", clear string
    if(generic=="NULL")
        generic.clear();
    if(origin=="NULL")
        origin.clear();
    if(special=="NULL")
        special.clear();

    c.m_char_data.m_has_titles = prefix || !generic.isEmpty() || !origin.isEmpty() || !special.isEmpty();
    if(!c.m_char_data.m_has_titles)
      return;

    c.m_char_data.m_has_the_prefix = prefix;
    c.m_char_data.m_titles[0] = generic;
    c.m_char_data.m_titles[1] = origin;
    c.m_char_data.m_titles[2] = special;
}

void    setInf(Character &c, uint32_t val) { c.m_char_data.m_influence = val; }
void    setDescription(Character &c, QString val) { c.m_char_data.m_character_description = val; }
void    setBattleCry(Character &c, QString val) { c.m_char_data.m_battle_cry = val; }

// Toggles
void toggleAFK(Character &c, const QString &msg)
{
    c.m_char_data.m_afk = !c.m_char_data.m_afk;
    if(c.m_char_data.m_afk)
        c.m_char_data.m_afk_msg = msg;
}

void    toggleLFG(Character &c) { c.m_char_data.m_lfg = !c.m_char_data.m_lfg; }
void    toggleTeamBuffs(Character &c) { c.m_gui.m_team_buffs = !c.m_gui.m_team_buffs; }


/*
 * Sidekick Methods -- Sidekick system requires teaming.
 */
// TODO: expose these to config and fail-test
static const int g_max_sidekick_level_difference = 3;
static const int g_min_sidekick_mentor_level = 10;

bool isSidekickMentor(const Entity &e) {
    return (e.m_char.m_char_data.m_sidekick.sk_type == SidekickType::IsMentor);
}

void addSidekick(Entity &src, Entity &tgt)
{
    QString     msg = "Unable to add sidekick.";
    Sidekick    &src_sk = src.m_char.m_char_data.m_sidekick;
    Sidekick    &tgt_sk = tgt.m_char.m_char_data.m_sidekick;
    uint32_t    src_lvl = getLevel(src.m_char);
    uint32_t    tgt_lvl = getLevel(tgt.m_char);

    // Only a mentor may invite a sidekick
    if(src_lvl < tgt_lvl+g_max_sidekick_level_difference)
        msg = "To Mentor another player, you must be at least 3 levels higher than them.";
    else if(src_lvl < g_min_sidekick_mentor_level)
        msg = "To Mentor another player, you must be at least level 10.";
    else if(src_sk.sk_has_sidekick)
        msg = "You are already Mentoring someone.";
    else if (tgt_sk.sk_has_sidekick)
        msg = tgt.name() + "is already a sidekick.";
    else if(!src.m_has_team && !tgt.m_has_team && src.m_team == nullptr && tgt.m_team == nullptr)
            msg = "To Mentor another player, you must be on the same team.";
    else if(src.m_team->m_team_idx != tgt.m_team->m_team_idx)
        msg = "To Mentor another player, you must be on the same team.";
    else
    {
        src_sk.sk_type = SidekickType::IsMentor;
        tgt_sk.sk_type = SidekickType::IsSidekick;
        setCombatLevel(tgt.m_char, src_lvl - 1);
        // TODO: Implement 225 feet "leash" for sidekicks.

        src_sk.sk_db_id = tgt.m_db_id;
        tgt_sk.sk_db_id = src.m_db_id;
        src_sk.sk_has_sidekick = true;
        tgt_sk.sk_has_sidekick = true;

        msg = QString("%1 is now Mentoring %2.").arg(src.name(),tgt.name());
        qDebug().noquote() << msg;

        // Send message to each player
        msg = QString("You are now Mentoring %1.").arg(tgt.name()); // Customize for src.
        sendInfoMessage(MessageChannel::TEAM, msg, src.m_client);
        msg = QString("%1 is now Mentoring you.").arg(src.name()); // Customize for src.
        sendInfoMessage(MessageChannel::TEAM, msg, tgt.m_client);

        return; // break early
    }

    qDebug().noquote() << msg;
    sendInfoMessage(MessageChannel::USER_ERROR, msg, src.m_client);
}

void removeSidekick(Entity &src)
{
    QString     msg = "Unable to remove sidekick.";
    Sidekick    &src_sk = src.m_char.m_char_data.m_sidekick;

    if(!src_sk.sk_has_sidekick || src_sk.sk_db_id == 0)
    {
        msg = "You are not sidekicked with anyone.";
        qDebug().noquote() << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, src.m_client);
        return; // break early
    }

    Entity      *tgt            = getEntityByDBID(src.m_client, src_sk.sk_db_id);
    Sidekick    &tgt_sk         = tgt->m_char.m_char_data.m_sidekick;

    if(tgt == nullptr)
    {
        msg = "Your sidekick is not currently online.";
        qDebug().noquote() << msg;

        // reset src Sidekick relationship
        src_sk.sk_has_sidekick = false;
        src_sk.sk_type         = SidekickType::NoSidekick;
        src_sk.sk_db_id        = 0;
        setCombatLevel(src.m_char,getLevel(src.m_char)); // reset CombatLevel

        return; // break early
    }

    // Anyone can terminate a Sidekick relationship
    if(!tgt_sk.sk_has_sidekick || (tgt_sk.sk_db_id != src.m_db_id))
    {
        // tgt doesn't know it's sidekicked with src. So clear src sidekick info.
        src_sk.sk_has_sidekick = false;
        src_sk.sk_type         = SidekickType::NoSidekick;
        src_sk.sk_db_id        = 0;
        setCombatLevel(src.m_char,getLevel(src.m_char)); // reset CombatLevel
        msg = QString("You are no longer sidekicked with anyone.");
    }
    else {
        src_sk.sk_has_sidekick = false;
        src_sk.sk_type         = SidekickType::NoSidekick;
        src_sk.sk_db_id        = 0;
        setCombatLevel(src.m_char,getLevel(src.m_char)); // reset CombatLevel

        tgt_sk.sk_has_sidekick = false;
        tgt_sk.sk_type         = SidekickType::NoSidekick;
        tgt_sk.sk_db_id        = 0;
        setCombatLevel(tgt->m_char,getLevel(tgt->m_char)); // reset CombatLevel

        msg = QString("%1 and %2 are no longer sidekicked.").arg(src.name(),tgt->name());
        qDebug().noquote() << msg;

        // Send message to each player
        if(isSidekickMentor(src))
        {
            // src is mentor, tgt is sidekick
            msg = QString("You are no longer mentoring %1.").arg(tgt->name());
            sendInfoMessage(MessageChannel::TEAM, msg, src.m_client);
            msg = QString("%1 is no longer mentoring you.").arg(src.name());
            sendInfoMessage(MessageChannel::TEAM, msg, tgt->m_client);
        }
        else
        {
            // src is sidekick, tgt is mentor
            msg = QString("You are no longer mentoring %1.").arg(src.name());
            sendInfoMessage(MessageChannel::TEAM, msg, tgt->m_client);
            msg = QString("%1 is no longer mentoring you.").arg(tgt->name());
            sendInfoMessage(MessageChannel::TEAM, msg, src.m_client);
        }

        return; // break early
    }

    qDebug().noquote() << msg;
    sendInfoMessage(MessageChannel::USER_ERROR, msg, src.m_client);
}
