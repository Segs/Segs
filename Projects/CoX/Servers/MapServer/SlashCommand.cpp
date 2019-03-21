/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "SlashCommand.h"

#include "DataHelpers.h"
#include "MessageHelpers.h"
#include "Messages/Map/ForceLogout.h"
#include "Messages/Map/GameCommandList.h"
#include "Messages/Map/MapXferWait.h"
#include "GameData/ClientStates.h"
#include "GameData/GameDataStore.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/map_definitions.h"
#include "Logging.h"
#include "MapLink.h"
#include "MapInstance.h"
#include "GameData/Character.h"
#include "GameData/CharacterHelpers.h"
#include "GameData/Entity.h"
#include "GameData/EntityHelpers.h"
#include "GameData/VisitLocation.h"
#include "GameData/LFG.h"
#include "GameData/Trade.h"
#include "Settings.h"

#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QRegularExpression>
#include <QtCore/QDebug>
#include <ctime>

using namespace SEGSEvents;

namespace  {
class InfoMessageCmd; // leverage InfoMessageCmd

struct SlashCommand
{
    QStringList m_valid_prefixes;
    QString m_help_text;
    std::function<void(const QString &,MapClientSession &)> m_handler;
    uint32_t m_required_access_level;
};

//bool canAccessCommand(const SlashCommand &cmd, const Entity &e); --> function not defined (yet)
bool canAccessCommand(const SlashCommand &cmd, MapClientSession &src);

// prototypes of all commands
// Access Level 9 Commands (GMs)
void cmdHandler_Script(const QString &cmd, MapClientSession &sess);
void cmdHandler_Dialog(const QString &cmd, MapClientSession &sess);
void cmdHandler_InfoMessage(const QString &cmd, MapClientSession &sess);
void cmdHandler_SmileX(const QString &cmd, MapClientSession &sess);
void cmdHandler_Fly(const QString &cmd, MapClientSession &sess);
void cmdHandler_Falling(const QString &cmd, MapClientSession &sess);
void cmdHandler_Sliding(const QString &cmd, MapClientSession &sess);
void cmdHandler_Jumping(const QString &cmd, MapClientSession &sess);
void cmdHandler_Stunned(const QString &cmd, MapClientSession &sess);
void cmdHandler_Jumppack(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetSpeed(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetBackupSpd(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetJumpHeight(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetHP(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetEnd(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetXP(const QString &cmd, MapClientSession &sess);
void cmdHandler_GiveXP(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetDebt(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetInf(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetLevel(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetCombatLevel(const QString &cmd, MapClientSession &sess);
void cmdHandler_UpdateChar(const QString &cmd, MapClientSession &sess);
void cmdHandler_DebugChar(const QString &cmd, MapClientSession &sess);
void cmdHandler_ControlsDisabled(const QString &cmd, MapClientSession &sess);
void cmdHandler_UpdateId(const QString &cmd, MapClientSession &sess);
void cmdHandler_FullUpdate(const QString &cmd, MapClientSession &sess);
void cmdHandler_HasControlId(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetTeam(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetSuperGroup(const QString &cmd, MapClientSession &sess);
void cmdHandler_SettingsDump(const QString &cmd, MapClientSession &sess);
void cmdHandler_TeamDebug(const QString &cmd, MapClientSession &sess);
void cmdHandler_GUIDebug(const QString &, MapClientSession &sess);
void cmdHandler_SetWindowVisibility(const QString &cmd, MapClientSession &sess);
void cmdHandler_KeybindDebug(const QString &cmd, MapClientSession &sess);
void cmdHandler_ToggleLogging(const QString &cmd, MapClientSession &sess);
void cmdHandler_FriendsListDebug(const QString &cmd, MapClientSession &sess);
void cmdHandler_SendFloatingNumbers(const QString &cmd, MapClientSession &sess);
void cmdHandler_ToggleInterp(const QString &cmd, MapClientSession &sess);
void cmdHandler_ToggleMoveInstantly(const QString &cmd, MapClientSession &sess);
void cmdHandler_ToggleCollision(const QString &cmd, MapClientSession &sess);
void cmdHandler_ToggleMovementAuthority(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetSequence(const QString &cmd, MapClientSession &sess);
void cmdHandler_AddTriggeredMove(const QString &cmd, MapClientSession &sess);
void cmdHandler_AddTimeStateLog(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetClientState(const QString &cmd, MapClientSession &sess);
void cmdHandler_AddEntirePowerSet(const QString &cmd, MapClientSession &sess);
void cmdHandler_AddPower(const QString &cmd, MapClientSession &sess);
void cmdHandler_AddInspiration(const QString &cmd, MapClientSession &sess);
void cmdHandler_AddEnhancement(const QString &cmd, MapClientSession &sess);
void cmdHandler_LevelUpXp(const QString &cmd, MapClientSession &sess);
void cmdHandler_FaceEntity(const QString &cmd, MapClientSession &sess);
void cmdHandler_FaceLocation(const QString &cmd, MapClientSession &sess);
void cmdHandler_MoveZone(const QString &cmd, MapClientSession &sess);
void cmdHandler_TestDeadNoGurney(const QString &cmd, MapClientSession &sess);
void cmdHandler_DoorMessage(const QString &cmd, MapClientSession &sess);
void cmdHandler_Browser(const QString &cmd, MapClientSession &sess);
void cmdHandler_SendTimeUpdate(const QString &cmd, MapClientSession &sess);
void cmdHandler_SendContactDialog(const QString &cmd, MapClientSession &sess);
void cmdHandler_SendContactDialogYesNoOk(const QString &cmd, MapClientSession &sess);
void cmdHandler_SendWaypoint(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetStateMode(const QString &cmd, MapClientSession &sess);
void cmdHandler_Revive(const QString &cmd, MapClientSession &sess);
void cmdHandler_AddCostumeSlot(const QString &cmd, MapClientSession &sess);
void cmdHandler_ContactStatusList(const QString &cmd, MapClientSession &sess);
void cmdHandler_AddTestTask(const QString &/*cmd*/, MapClientSession &sess);
void cmdHandler_ReloadScripts(const QString &/*cmd*/, MapClientSession &sess);
void cmdHandler_OpenStore(const QString &/*cmd*/, MapClientSession &sess);
void cmdHandler_ForceLogout(const QString &cmd, MapClientSession &sess);
void cmdHandler_SendLocations(const QString &cmd, MapClientSession &sess);
void cmdHandler_SendConsoleOutput(const QString &cmd, MapClientSession &sess);
void cmdHandler_SendConsolePrint(const QString &cmd, MapClientSession &sess);
void cmdHandler_ClearTarget(const QString &cmd, MapClientSession &sess);
void cmdHandler_StartTimer(const QString &cmd, MapClientSession &sess);

// For live value-testing
void cmdHandler_SetU1(const QString &cmd, MapClientSession &sess);

// Access Level 2[GM] Commands
void cmdHandler_AddNPC(const QString &cmd, MapClientSession &sess);
void cmdHandler_MoveTo(const QString &cmd, MapClientSession &sess);

// Access Level 1 Commands
void cmdHandler_CmdList(const QString &cmd, MapClientSession &sess);
void cmdHandler_AFK(const QString &cmd, MapClientSession &sess);
void cmdHandler_WhoAll(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetTitles(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetCustomTitles(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetSpecialTitle(const QString &cmd, MapClientSession &sess);
void cmdHandler_Stuck(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetSpawnLocation(const QString &cmd, MapClientSession &sess);
void cmdHandler_LFG(const QString &cmd, MapClientSession &sess);
void cmdHandler_MOTD(const QString &cmd, MapClientSession &sess);
void cmdHandler_Invite(const QString &cmd, MapClientSession &sess);
void cmdHandler_Kick(const QString &cmd, MapClientSession &sess);
void cmdHandler_LeaveTeam(const QString &cmd, MapClientSession &sess);
void cmdHandler_FindMember(const QString &cmd, MapClientSession &sess);
void cmdHandler_MakeLeader(const QString &cmd, MapClientSession &sess);
void cmdHandler_SetAssistTarget(const QString &cmd, MapClientSession &sess);
void cmdHandler_Sidekick(const QString &cmd, MapClientSession &sess);
void cmdHandler_UnSidekick(const QString &cmd, MapClientSession &sess);
void cmdHandler_TeamBuffs(const QString &cmd, MapClientSession &sess);
void cmdHandler_Friend(const QString &cmd, MapClientSession &sess);
void cmdHandler_Unfriend(const QString &cmd, MapClientSession &sess);
void cmdHandler_FriendList(const QString &cmd, MapClientSession &sess);
void cmdHandler_MapXferList(const QString &cmd, MapClientSession &sess);
void cmdHandler_ReSpec(const QString &cmd, MapClientSession &sess);
void cmdHandler_Trade(const QString &cmd, MapClientSession &sess);
void cmdHandler_Tailor(const QString &cmd, MapClientSession &sess);
void cmdHandler_CostumeChange(const QString &cmd, MapClientSession &sess);
void cmdHandler_Train(const QString &cmd, MapClientSession &sess);
void cmdHandler_Kiosk(const QString &cmd, MapClientSession &sess);

// Access Level 0 Commands
void cmdHandler_TeamAccept(const QString &cmd, MapClientSession &sess);
void cmdHandler_TeamDecline(const QString &cmd, MapClientSession &sess);
void cmdHandler_SidekickAccept(const QString &cmd, MapClientSession &sess);
void cmdHandler_SidekickDecline(const QString &cmd, MapClientSession &sess);
void cmdHandler_EmailHeaders(const QString &cmd, MapClientSession &sess);
void cmdHandler_EmailRead(const QString &cmd, MapClientSession &sess);
void cmdHandler_EmailSend(const QString &cmd, MapClientSession &sess);
void cmdHandler_EmailDelete(const QString &cmd, MapClientSession &sess);
void cmdHandler_TradeAccept(const QString &cmd, MapClientSession &sess);
void cmdHandler_TradeDecline(const QString &cmd, MapClientSession &sess);

static const SlashCommand g_defined_slash_commands[] = {
    /* Access Level 9 Commands */
    {{"Script"},"Run a Script", cmdHandler_Script, 9},
    {{"Dialog", "dlg"},"Open a dialog box with any string arg", cmdHandler_Dialog, 9},
    {{"InfoMessage", "imsg"},"Send an info message into chat. Expects <int> <string>, e.g. /imsg 1 test", cmdHandler_InfoMessage, 9},
    {{"SmileX"},"Runs the smlx script SmileX", cmdHandler_SmileX, 9},
    {{"fly", "flying"},"Toggle flying On/Off", cmdHandler_Fly, 9},
    {{"falling"},"Toggle falling On/Off", cmdHandler_Falling, 9},
    {{"sliding"},"Toggle sliding On/Off", cmdHandler_Sliding, 9},
    {{"jumping"},"Toggle jumping On/Off", cmdHandler_Jumping, 9},
    {{"stunned"},"Toggle stunned character On/Off", cmdHandler_Stunned, 9},
    {{"jumppack"},"Toggle jump pack On/Off", cmdHandler_Jumppack, 9},
    {{"setSpeed", "speed"},"Set your player Speed", cmdHandler_SetSpeed, 9},
    {{"setBackupSpd", "BackupSpd"},"Set the Backup Speed", cmdHandler_SetBackupSpd, 9},
    {{"setJumpHeight", "JumpHeight"},"Set the Jump Height", cmdHandler_SetJumpHeight, 9},
    {{"setHP"},"Set the HP value of your character", cmdHandler_SetHP, 9},
    {{"setEnd"},"Set your Endurance", cmdHandler_SetEnd, 9},
    {{"setXP"},"Set your XP", cmdHandler_SetXP, 9},
    {{"giveXP"},"Give yourself XP", cmdHandler_GiveXP, 9},
    {{"setDebt"},"Set your Debt", cmdHandler_SetDebt, 9},
    {{"setInf"},"Set your Influence", cmdHandler_SetInf, 9},
    {{"setLevel"},"Set your Level", cmdHandler_SetLevel, 9},
    {{"setCombatLevel"},"Set your Combat Level", cmdHandler_SetCombatLevel, 9},
    {{"UpdateChar", "CharUpdate", "save"},"Update character information in the database", cmdHandler_UpdateChar, 9},
    {{"DebugChar", "chardebug"},"View your characters debug information", cmdHandler_DebugChar, 9},
    {{"ControlsDisabled"},"Disable controls", cmdHandler_ControlsDisabled, 9},
    {{"updateid"},"Update ID", cmdHandler_UpdateId, 9},
    {{"fullupdate"},"Full Update", cmdHandler_FullUpdate, 9},
    {{"hascontrolid"},"Force the server to acknowledge input ids", cmdHandler_HasControlId, 9},
    {{"setTeam", "setTeamID"},"Set the team idx", cmdHandler_SetTeam, 9},
    {{"setSuperGroup","setSG"},"Set your Super Group", cmdHandler_SetSuperGroup, 9},
    {{"settingsDump","settingsDebug"},"Output settings.cfg to console", cmdHandler_SettingsDump, 9},
    {{"teamDump", "teamDebug"}, "Output team settings to console", cmdHandler_TeamDebug, 9},
    {{"guiDump", "guiDebug"}, "Output gui settings to console", cmdHandler_GUIDebug, 9},
    {{"setWindowVisibility", "setWinVis"}, "", cmdHandler_SetWindowVisibility, 9},
    {{"keybindDump", "keybindDebug"}, "Output keybind settings to console", cmdHandler_KeybindDebug, 9},
    {{"toggleLogging", "log"}, "Modify log categories (e.g. input, teams, ...)", cmdHandler_ToggleLogging, 9},
    {{"friendsDump", "friendsDebug"}, "Output friendlist info to console", cmdHandler_FriendsListDebug, 9},
    {{"damage", "heal"}, "Make current target (or self) take damage/health", cmdHandler_SendFloatingNumbers, 9},
    {{"interp"},"Toggle Interpolation", &cmdHandler_ToggleInterp, 9},
    {{"moveinstantly"},"Toggle move_instantly", &cmdHandler_ToggleMoveInstantly, 9},
    {{"collision"},"Toggle Collision on/off", &cmdHandler_ToggleCollision, 9},
    {{"movement"},"Toggle server authority for Movement on/off", &cmdHandler_ToggleMovementAuthority, 9},
    {{"setSeq"},"Set Sequence values <update> <move_idx> <duration>", &cmdHandler_SetSequence, 9},
    {{"addTriggeredMove"},"Set TriggeredMove values <move_idx> <delay> <fx_idx>", &cmdHandler_AddTriggeredMove, 9},
    {{"timestate", "setTimeStateLog"},"Set TimeStateLog value.", cmdHandler_AddTimeStateLog, 9},
    {{"clientstate"},"Set ClientState mode", &cmdHandler_SetClientState, 9},
    {{"addpowerset"},"Adds entire PowerSet (by 'pcat pset' idxs) to Entity", &cmdHandler_AddEntirePowerSet, 9},
    {{"addpower"},"Adds Power (by 'pcat pset pow' idxs) to Entity", &cmdHandler_AddPower, 9},
    {{"addinsp"},"Adds Inspiration (by name) to Entity", &cmdHandler_AddInspiration, 9},
    {{"addboost", "addEnhancement"},"Adds Enhancement (by name) to Entity", &cmdHandler_AddEnhancement, 9},
    {{"levelupxp"},"Level Up Character to Level Provided", &cmdHandler_LevelUpXp, 9},
    {{"face"}, "Face a target", cmdHandler_FaceEntity, 9},
    {{"faceLocation"}, "Face a location", cmdHandler_FaceLocation, 9},
    {{"mapmove", "movezone", "mz"}, "Move to a map id", cmdHandler_MoveZone, 9},
    {{"deadnogurney"}, "Test Dead No Gurney. Fakes sending the client packet.", cmdHandler_TestDeadNoGurney, 9},
    {{"doormsg"}, "Test Door Message. Fakes sending the client packet.", cmdHandler_DoorMessage, 9},
    {{"browser"}, "Test Browser. Sends content to a browser window", cmdHandler_Browser, 9},
    {{"timeupdate"}, "Test TimeUpdate. Sends time update to server", cmdHandler_SendTimeUpdate, 9},
    {{"contactdlg", "cdlg"}, "Test ContactDialog. Sends contact dialog with responses to server", cmdHandler_SendContactDialog, 9},
    {{"contactdlgyesno", "cdlg2"}, "Test ContactDialogYesNoOk. Sends contact dialog with yes/no response to server", cmdHandler_SendContactDialogYesNoOk, 9},
    {{"setwaypoint"}, "Test SendWaypoint. Send waypoint to client", cmdHandler_SendWaypoint, 9},
    {{"setstatemode"}, "Send StateMode. Send StateMode to client", cmdHandler_SetStateMode, 9},
    {{"revive"}, "Revive Self or Target Player", cmdHandler_Revive, 9},
    {{"addcostumeslot"}, "Add a new costume slot to Character", cmdHandler_AddCostumeSlot, 9},
    {{"contactList"}, "Update Contact List", cmdHandler_ContactStatusList, 9},
    {{"testTask"}, "Test Task", cmdHandler_AddTestTask, 9},
    {{"reloadLua"}, "Reload all Lua scripts", cmdHandler_ReloadScripts, 9},
    {{"openStore"}, "Open store Window", cmdHandler_OpenStore, 9},
    {{"forceLogout"}, "Logout player", cmdHandler_ForceLogout, 9},
    {{"sendLocation"}, "Send Location Test", cmdHandler_SendLocations, 9},
    {{"developerConsoleOutput"}, "Send message to -console window", cmdHandler_SendConsoleOutput, 9},
    {{"clientConsoleOutput"}, "Send message to ingame (~) console", cmdHandler_SendConsolePrint, 9},
    {{"clearTarget"}, "Clear current target", cmdHandler_ClearTarget, 9},
    {{"startTimer"}, "Create a small timer", cmdHandler_StartTimer, 9},

    // For live value-testing
    {{"setu1"},"Set bitvalue u1. Used for live-debugging.", cmdHandler_SetU1, 9},

    /* Access Level 2 Commands */
    {{"addNpc"},"add <npc_name> with costume [variation] in front of gm", cmdHandler_AddNPC, 2},
    {{"moveTo", "setpos", "setpospyr"},"set the gm's position to <x> <y> <z>", cmdHandler_MoveTo, 2},

    /* Access Level 1 Commands */
    {{"cmdlist","commandlist"},"List all accessible commands", cmdHandler_CmdList, 1},
    {{"afk"},"Mark yourself as Away From Keyboard", cmdHandler_AFK, 1},
    {{"whoall"},"Shows who is on the current map ", cmdHandler_WhoAll, 1},
    {{"setTitles","title"},"Open the Title selection window", cmdHandler_SetTitles, 1},
    {{"setCustomTitles"},"Set your titles manually", cmdHandler_SetCustomTitles, 1},
    {{"setSpecialTitle"},"Set your Special title", cmdHandler_SetSpecialTitle, 1},
    {{"stuck"},"Free yourself if your character gets stuck", cmdHandler_Stuck, 1},
    {{"gotospawn"},"Teleport to a SpawnLocation", cmdHandler_SetSpawnLocation, 1},
    {{"lfg"},"Toggle looking for group status ", cmdHandler_LFG, 1},
    {{"motd"},"View the server MOTD", cmdHandler_MOTD, 1},
    {{"i","invite"}, "Invite player to team", cmdHandler_Invite, 1},
    {{"k","kick"}, "Kick player from team", cmdHandler_Kick, 1},
    {{"leaveteam"}, "Leave the team you're currently on", cmdHandler_LeaveTeam, 1},
    {{"findmember"}, "Open Looking For Group Window", cmdHandler_FindMember, 1},
    {{"makeleader","ml"}, "Make another player team leader", cmdHandler_MakeLeader, 1},
    {{"assist"}, "Re-target based on the selected entity's current target", cmdHandler_SetAssistTarget, 1},
    {{"sidekick","sk"}, "Invite player to sidekick", cmdHandler_Sidekick, 1},
    {{"unsidekick","unsk"}, "Remove sidekick/duo relationship", cmdHandler_UnSidekick, 1},
    {{"buffs"}, "Toggle buffs display on team window", cmdHandler_TeamBuffs, 1},
    {{"friend"}, "Add friend to friendlist", cmdHandler_Friend, 1},
    {{"unfriend","estrange"}, "Remove friend from friendlist", cmdHandler_Unfriend, 1},
    {{"friendlist", "fl"}, "Toggle visibility of friendslist", cmdHandler_FriendList, 1},
    {{"MapXferList", "mapmenu"}, "Show MapXferList", cmdHandler_MapXferList, 1},
    {{"respec"}, "Start ReSpec", cmdHandler_ReSpec, 1},
    {{"trade"}, "Trade with player", cmdHandler_Trade, 1},
    {{"tailor"}, "Open Tailor Window", cmdHandler_Tailor, 1},
    {{"cc"}, "Costume Change", cmdHandler_CostumeChange, 1},
    {{"train"}, "Train Up Level", cmdHandler_Train, 1},
    {{"kiosk"}, "Event kiosk", cmdHandler_Kiosk, 1},

    /* Access Level 0 Commands :: These are "behind the scenes" and sent by the client */
    {{"team_accept"}, "Accept Team invite", cmdHandler_TeamAccept, 0},
    {{"team_decline"}, "Decline Team invite", cmdHandler_TeamDecline, 0},
    {{"sidekick_accept"}, "Accept Sidekick invite", cmdHandler_SidekickAccept, 0},
    {{"sidekick_decline"}, "Decline Sidekick invite", cmdHandler_SidekickDecline, 0},
    {{"emailheaders"}, "Request Email Headers", cmdHandler_EmailHeaders, 0},
    {{"emailread"}, "Request Email Message with Given ID", cmdHandler_EmailRead, 0},
    {{"emailsend"}, "Send Email", cmdHandler_EmailSend, 0},
    {{"emaildelete"}, "Delete Email with Given ID",cmdHandler_EmailDelete, 0},
    {{"trade_accept"}, "Accept Trade invite", cmdHandler_TradeAccept, 0},
    {{"trade_decline"}, "Decline Trade invite", cmdHandler_TradeDecline, 0},
};

/************************************************************
 *  Helper Functions
 ***********************************************************/
static QString getCommandParameter(const QString& cmd)
{
    const int space = cmd.indexOf(' ');
    if(space == -1)
    {
        return "";
    }

    return cmd.mid(space + 1);
}

static Entity* getTargetEntity(MapClientSession& sess)
{
    if(sess.m_ent == nullptr)
    {
        return nullptr;
    }

    const uint32_t idx = getTargetIdx(*sess.m_ent);
    if(idx == 0)
    {
        return nullptr;
    }

    return getEntity(&sess, idx);
}

static Entity* getEntityFromCommand(const QString &cmd, MapClientSession& sess)
{
    const QString name = getCommandParameter(cmd);
    if(name.isEmpty())
    {
        return getTargetEntity(sess);
    }

    return getEntity(&sess, name);
}

/************************************************************
 *  Slash Command Handlers
 ***********************************************************/

void cmdHandler_MoveZone(const QString &cmd, MapClientSession &sess)
{
    uint32_t map_idx = cmd.midRef(cmd.indexOf(' ') + 1).toInt();
    if(map_idx == getMapIndex(sess.m_current_map->name()))
        map_idx = (map_idx + 1) % 23;   // To prevent crashing if trying to access the map you're on.
    MapXferData map_data = MapXferData();
    map_data.m_target_map_name = getMapName(map_idx);
    sess.link()->putq(new MapXferWait(getMapPath(map_idx)));

    HandlerLocator::getMap_Handler(sess.is_connected_to_game_server_id)
        ->putq(new ClientMapXferMessage({sess.link()->session_token(), map_data}, 0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 9 Commands (GMs)
void cmdHandler_Script(const QString &cmd, MapClientSession &sess)
{
    QString code = cmd.mid(7, cmd.size() - 7);
    sess.m_current_map->m_scripting_interface->runScript(&sess, code, "user provided script");
}

void cmdHandler_Dialog(const QString &cmd, MapClientSession &sess)
{
    sess.addCommandToSendNextUpdate(std::make_unique<StandardDialogCmd>(cmd.mid(4)));
}

void cmdHandler_InfoMessage(const QString &cmd, MapClientSession &sess)
{
    QString msg;
    int cmdType = int(MessageChannel::USER_ERROR);

    int first_space = cmd.indexOf(' ');
    int second_space = cmd.indexOf(' ',first_space+1);
    if(second_space==-1)
        msg = "The /imsg command takes two arguments, a <b>number</b> and a <b>string</b>";
    else
    {
        bool ok = true;
        cmdType = cmd.midRef(first_space+1,second_space-(first_space+1)).toInt(&ok);
        if(!ok || cmdType<1 || cmdType>21)
        {
            msg = "The first /imsg argument must be a <b>number</b> between 1 and 21";
            cmdType = int(MessageChannel::USER_ERROR);
        }
        else
            msg = cmd.mid(second_space+1);
    }
    sendInfoMessage(static_cast<MessageChannel>(cmdType), msg, sess);
}

void cmdHandler_SmileX(const QString &cmd, MapClientSession &sess)
{
    int space = cmd.indexOf(' ');
    QString fileName("scripts/" + cmd.mid(space+1));
    if(!fileName.endsWith(".smlx"))
            fileName.append(".smlx");
    QFile file(fileName);
    if(file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString contents(file.readAll());
        sess.addCommandToSendNextUpdate(std::make_unique<StandardDialogCmd>(contents));
    }
    else {
        QString errormsg = "Failed to load smilex file. \'" + file.fileName() + "\' not found.";
        qCDebug(logSlashCommand) << errormsg;
        sendInfoMessage(MessageChannel::ADMIN, errormsg, sess);
    }
}

void cmdHandler_Fly(const QString &cmd, MapClientSession &sess)
{
    toggleFlying(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_Falling(const QString &cmd, MapClientSession &sess)
{
    toggleFalling(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_Sliding(const QString &cmd, MapClientSession &sess)
{
    toggleSliding(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_Jumping(const QString &cmd, MapClientSession &sess)
{
    toggleJumping(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_Stunned(const QString &cmd, MapClientSession &sess)
{
    toggleStunned(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_Jumppack(const QString &cmd, MapClientSession &sess)
{
    toggleJumppack(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetSpeed(const QString &cmd, MapClientSession &sess)
{
    QVector<QStringRef> args(cmd.splitRef(' '));
    float v1 = args.value(1).toFloat();
    float v2 = args.value(2).toFloat();
    float v3 = args.value(3).toFloat();
    setSpeed(*sess.m_ent, v1, v2, v3);

    QString msg = QString("Set Speed to: <%1,%2,%3>").arg(v1).arg(v2).arg(v3);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetBackupSpd(const QString &cmd, MapClientSession &sess)
{
    float val = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    setBackupSpd(*sess.m_ent, val);

    QString msg = "Set BackupSpd to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetJumpHeight(const QString &cmd, MapClientSession &sess)
{
    float val = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    setJumpHeight(*sess.m_ent, val);

    QString msg = "Set JumpHeight to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetHP(const QString &cmd, MapClientSession &sess)
{
    float attrib = cmd.midRef(cmd.indexOf(' ')+1).toFloat();

    setHP(*sess.m_ent->m_char, attrib);

    QString msg = QString("Setting HP to: %1 / %2")
            .arg(attrib).arg(getMaxHP(*sess.m_ent->m_char));
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetEnd(const QString &cmd, MapClientSession &sess)
{
    float attrib = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    float maxattrib = sess.m_ent->m_char->m_max_attribs.m_Endurance;

    if(attrib > maxattrib)
        attrib = maxattrib;

    setEnd(*sess.m_ent->m_char,attrib);

    QString msg = QString("Setting Endurance to: %1 / %2").arg(attrib).arg(maxattrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetXP(const QString &cmd, MapClientSession &sess)
{
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();
    uint32_t lvl = getLevel(*sess.m_ent->m_char);

    setXP(*sess.m_ent->m_char, attrib);
    QString msg = "Setting XP to " + QString::number(attrib);

    uint32_t newlvl = getLevel(*sess.m_ent->m_char);
    if(lvl != newlvl)
        msg += " and LVL to " + QString::number(newlvl);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_GiveXP(const QString &cmd, MapClientSession &sess)
{
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();
    uint32_t lvl = getLevel(*sess.m_ent->m_char);

    giveXp(sess, attrib);
    QString msg = "Giving " + QString::number(attrib) + " XP";

    uint32_t newlvl = getLevel(*sess.m_ent->m_char);
    if(lvl != newlvl)
        msg += " and setting LVL to " + QString::number(newlvl);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetDebt(const QString &cmd, MapClientSession &sess)
{
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setDebt(*sess.m_ent->m_char, attrib);
    QString msg = QString("Setting XP Debt to %1").arg(attrib);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetInf(const QString &cmd, MapClientSession &sess)
{
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setInf(*sess.m_ent->m_char, attrib);

    QString msg = "Setting influence to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetLevel(const QString &cmd, MapClientSession &sess)
{
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt()-1; // convert from 1-50 to 0-49

    setLevel(*sess.m_ent->m_char, attrib);

    QString contents = FloatingInfoMsg.find(FloatingMsg_Leveled).value();
    sendFloatingInfo(sess, contents, FloatingInfoStyle::FloatingInfo_Attention, 4.0);

    QString msg = "Setting Level to: " + QString::number(attrib + 1);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetCombatLevel(const QString &cmd, MapClientSession &sess)
{
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt()-1; // convert from 1-50 to 0-49

    setCombatLevel(*sess.m_ent->m_char, attrib);

    QString msg = "Setting Combat Level to: " + QString::number(attrib+1);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_UpdateChar(const QString &cmd, MapClientSession &sess)
{
    markEntityForDbStore(sess.m_ent, DbStoreFlags::Full);

    QString msg = "Updating Character in Database: " + sess.m_ent->name();
    qCDebug(logSlashCommand) << cmd << ":" << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_DebugChar(const QString &/*cmd*/, MapClientSession &sess)
{
    const Character &chardata(*sess.m_ent->m_char);
    QString msg = "DebugChar: " + sess.m_ent->name()
            + "\n  " + chardata.m_char_data.m_origin_name
            + "\n  " + chardata.m_char_data.m_class_name
            + "\n  map: " + getEntityDisplayMapName(sess.m_ent->m_entity_data)
            + "\n  db_id: " + QString::number(sess.m_ent->m_db_id) + ":" + QString::number(chardata.m_db_id)
            + "\n  idx: " + QString::number(sess.m_ent->m_idx)
            + "\n  access: " + QString::number(sess.m_ent->m_entity_data.m_access_level)
            + "\n  acct: " + QString::number(chardata.m_account_id)
            + "\n  lvl/clvl: " + QString::number(chardata.m_char_data.m_level+1) + "/" + QString::number(chardata.m_char_data.m_combat_level+1)
            + "\n  inf: " + QString::number(chardata.m_char_data.m_influence)
            + "\n  xp/debt: " + QString::number(chardata.m_char_data.m_experience_points) + "/" + QString::number(chardata.m_char_data.m_experience_debt)
            + "\n  lfg: " + QString::number(chardata.m_char_data.m_lfg)
            + "\n  afk: " + QString::number(chardata.m_char_data.m_afk)
            + "\n  tgt_idx: " + QString::number(getTargetIdx(*sess.m_ent));
    sess.m_ent->dump();
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_ControlsDisabled(const QString &cmd, MapClientSession &sess)
{
    toggleControlsDisabled(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_UpdateId(const QString &cmd, MapClientSession &sess)
{
    uint8_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUShort();

    setUpdateID(*sess.m_ent, attrib);

    QString msg = "Setting updateID to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_FullUpdate(const QString &cmd, MapClientSession &sess)
{
    toggleFullUpdate(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_HasControlId(const QString &cmd, MapClientSession &sess)
{
    toggleControlId(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetTeam(const QString &cmd, MapClientSession &sess)
{
    uint8_t val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setTeamID(*sess.m_ent, val);

    QString msg = "Set Team ID to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetSuperGroup(const QString &cmd, MapClientSession &sess)
{
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    int sg_id       = args.value(1).toInt();
    QString sg_name = args.value(2);
    int sg_rank     = args.value(3).toInt();

    setSuperGroup(*sess.m_ent, sg_id, sg_name, sg_rank);

    QString msg = QString("Set SuperGroup:  id: %1  name: %2  rank: %3").arg(QString::number(sg_id), sg_name, QString::number(sg_rank));
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SettingsDump(const QString &/*cmd*/, MapClientSession &sess)
{
    QString msg = "Sending settings config dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    settingsDump(); // Send settings dump
}

void cmdHandler_TeamDebug(const QString &/*cmd*/, MapClientSession &sess)
{
    QString msg = "Sending team debug to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    sess.m_ent->m_team->dump(); // Send team debug info
}


void cmdHandler_GUIDebug(const QString &/*cmd*/, MapClientSession &sess)
{
    QString msg = "Sending GUISettings dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    sess.m_ent->m_player->m_gui.guiDump(); // Send GUISettings dump
}

void cmdHandler_SetWindowVisibility(const QString &cmd, MapClientSession &sess)
{
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    uint32_t idx = args.value(1).toUInt();
    WindowVisibility val = (WindowVisibility)args.value(2).toInt();

    QString msg = "Toggling " + QString::number(idx) +  " GUIWindow visibility: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    sess.m_ent->m_player->m_gui.m_wnds.at(idx).setWindowVisibility(val); // Set WindowVisibility
    sess.m_ent->m_player->m_gui.m_wnds.at(idx).guiWindowDump(); // for debugging
}

void cmdHandler_KeybindDebug(const QString &/*cmd*/, MapClientSession &sess)
{
    QString msg = "Sending Keybinds dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    sess.m_ent->m_player->m_keybinds.keybindsDump(); // Send GUISettings dump
}

void cmdHandler_ToggleLogging(const QString &cmd, MapClientSession &sess)
{
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry
    args.removeFirst();

    QString msg = "Toggle logging of categories: " + args.join(" ");
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    for (auto category : args)
        toggleLogging(category); // Toggle each category listed
}

void cmdHandler_FriendsListDebug(const QString &/*cmd*/, MapClientSession &sess)
{
    QString msg = "Sending FriendsList dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    dumpFriends(*sess.m_ent); // Send FriendsList dump
}

void cmdHandler_SendFloatingNumbers(const QString &cmd, MapClientSession &sess)
{
    Entity *tgt = nullptr;

    QString msg; // result messages
    int first_space  = cmd.indexOf(' ');
    int second_space = cmd.indexOf(' ',first_space+1);
    int third_space  = cmd.indexOf(' ',second_space+1);

    bool ok1 = true;
    bool ok2 = true;
    uint32_t runtimes   = cmd.midRef(first_space+1, second_space-(first_space+1)).toInt(&ok1);
    float amount        = cmd.midRef(second_space+1, third_space-(second_space+1)).toFloat(&ok2);
    QString name        = cmd.midRef(third_space+1).toString();

    // reign in the insanity
    if(runtimes<=0)
        runtimes = 1;
    else if(runtimes > 5)
        runtimes = 5;

    if(!ok1 || !ok2 || name.isEmpty())
    {
        msg = "FloatingNumbers takes three arguments: `/damage <number_times_to_run> <damage_amount> <target_name>`";
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
        return;
    }

    tgt = getEntity(&sess,name); // get Entity by name

    if(tgt == nullptr)
    {
        msg = "FloatingNumbers target " + name + " cannot be found.";
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
        return;
    }

    for(uint32_t i = 0; i<runtimes; i++)
    {
        sendFloatingNumbers(sess, tgt->m_idx, int(amount));

        setHP(*tgt->m_char, getHP(*tgt->m_char) - amount); // deal dmg

        if(amount >= 0) // damage
        {
            msg = QString("%1 deals %2 points of damage to %3.").arg(sess.m_ent->name()).arg(amount).arg(name);
            qCDebug(logSlashCommand) << msg;

            msg = QString("You deal %1 points of damage to %2.").arg(QString::number(amount), name);
            sendInfoMessage(MessageChannel::DAMAGE, msg, sess);
            msg = QString("%1 has dealt you %2 points of damage!").arg(sess.m_ent->name()).arg(amount);
            sendInfoMessage(MessageChannel::DAMAGE, msg, *tgt->m_client);
        }
        else
        {
            msg = QString("%1 heals %2 points of damage from %3.").arg(sess.m_ent->name()).arg(-amount).arg(name);
            qCDebug(logSlashCommand) << msg;

            msg = QString("You heal %1 points of damage from %2.").arg(-amount).arg(name);
            sendInfoMessage(MessageChannel::TEAM, msg, sess); // TEAM for green
            msg = QString("%1 has healed %2 points of damage from you!").arg(sess.m_ent->name()).arg(-amount);
            sendInfoMessage(MessageChannel::TEAM, msg, *tgt->m_client); // TEAM for green
        }
    }
}

void cmdHandler_ToggleInterp(const QString &cmd, MapClientSession &sess)
{
    toggleInterp(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_ToggleMoveInstantly(const QString &cmd, MapClientSession &sess)
{
    toggleMoveInstantly(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_ToggleCollision(const QString &cmd, MapClientSession &sess)
{
    toggleCollision(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_ToggleMovementAuthority(const QString &cmd, MapClientSession &sess)
{
    toggleMovementAuthority(*sess.m_ent);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetSequence(const QString &cmd, MapClientSession &sess)
{
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    bool        update  = args.value(1).toUInt();
    uint32_t    idx     = args.value(2).toUInt();
    uint8_t     time    = args.value(3).toUInt();

    QString msg = "Setting Sequence " + QString::number(idx) + " for " + QString::number(time);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);

    sess.m_ent->m_seq_update = update;
    sess.m_ent->m_seq_move_idx = idx;
    sess.m_ent->m_seq_move_change_time = time;
}

void cmdHandler_AddTriggeredMove(const QString &cmd, MapClientSession &sess)
{
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    uint32_t move_idx, delay, fx_idx;
    move_idx    = args.value(1).toUInt();
    delay       = args.value(2).toUInt();
    fx_idx      = args.value(3).toUInt();

    addTriggeredMove(*sess.m_ent, move_idx, delay, fx_idx);

    QString msg = QString("Setting TriggeredMove: idx %1;  ticks: %2;  fx_idx: %3").arg(move_idx).arg(delay).arg(fx_idx);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_AddTimeStateLog(const QString &cmd, MapClientSession &sess)
{
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    if(val == 0)
        val = std::time(nullptr);

    sendTimeStateLog(sess, val);

    QString msg = "Set TimeStateLog to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SetClientState(const QString &cmd, MapClientSession &sess)
{
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    sendClientState(sess, ClientStates(val));

    QString msg = "Setting ClientState to: " + QString::number(val);
    //qCDebug(logSlashCommand) << msg; // we're already sending a debug msg elsewhere
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_AddEntirePowerSet(const QString &cmd, MapClientSession &sess)
{
    CharacterData &cd = sess.m_ent->m_char->m_char_data;
    QString floating_msg = FloatingInfoMsg.find(FloatingMsg_FoundClue).value();

    QVector<QStringRef> args(cmd.splitRef(' '));
    uint32_t v1 = args.value(1).toInt();
    uint32_t v2 = args.value(2).toInt();

    if(args.size() < 4)
    {
        qCDebug(logSlashCommand) << "Bad invocation:" << cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation: " + cmd, sess);
        return;
    }

    QString msg = QString("Granting Entire PowerSet <%1, %2> to %3").arg(v1).arg(v2).arg(sess.m_ent->name());

    PowerPool_Info ppool;
    ppool.m_pcat_idx = v1;
    ppool.m_pset_idx = v2;

    addEntirePowerSet(cd, ppool);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
    sendFloatingInfo(sess, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
}

void cmdHandler_AddPower(const QString &cmd, MapClientSession &sess)
{
    CharacterData &cd = sess.m_ent->m_char->m_char_data;
    QString floating_msg = FloatingInfoMsg.find(FloatingMsg_FoundClue).value();

    bool ok;
    QVector<QStringRef> args(cmd.splitRef(' '));
    uint32_t v1 = args.value(1).toInt(&ok);
    uint32_t v2 = args.value(2).toInt();
    uint32_t v3 = args.value(3).toInt();

    if(args.size() < 4 || !ok)
    {
        qCDebug(logSlashCommand) << "Bad invocation:" << cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation: " + cmd, sess);
        return;
    }

    QString msg = QString("Granting Power <%1, %2, %3> to %4").arg(v1).arg(v2).arg(v3).arg(sess.m_ent->name());

    PowerPool_Info ppool;
    ppool.m_pcat_idx = v1;
    ppool.m_pset_idx = v2;
    ppool.m_pow_idx = v3;

    addPower(cd, ppool);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
    sendFloatingInfo(sess, floating_msg, FloatingInfoStyle::FloatingInfo_Attention, 4.0);
}

void cmdHandler_AddInspiration(const QString &cmd, MapClientSession &sess)
{
    int space = cmd.indexOf(' ');
    QString val = cmd.mid(space+1);
    giveInsp(sess, val);
}

void cmdHandler_AddEnhancement(const QString &cmd, MapClientSession &sess)
{
    QVector<QStringRef> args(cmd.splitRef(' '));
    QString name = args.value(1).toString();
    uint32_t level = args.value(2).toUInt() -1;

    if(args.size() < 3)
    {
        level = getLevel(*sess.m_ent->m_char);
    }

    giveEnhancement(sess, name, level);
}

void cmdHandler_LevelUpXp(const QString &cmd, MapClientSession &sess)
{
    GameDataStore &data(getGameData());

    // must adjust level for 0-index array, capped at 49
    uint32_t level = cmd.midRef(cmd.indexOf(' ')+1).toUInt();
    uint32_t max_level = data.expMaxLevel();
    level = std::max(uint32_t(0), std::min(level, max_level));

    // XP must be high enough for the level you're advancing to
    // since this slash command is forcing a levelup, let's
    // increase xp accordingly
    if(getXP(*sess.m_ent->m_char) < data.expForLevel(level))
        setXP(*sess.m_ent->m_char, data.expForLevel(level));
    else
        return;

    qCDebug(logPowers) << "LEVELUP" << sess.m_ent->name() << "to" << level+1
                       << "NumPowers:" << countAllOwnedPowers(sess.m_ent->m_char->m_char_data, false) // no temps
                       << "NumPowersAtLevel:" << data.countForLevel(level, data.m_pi_schedule.m_Power);

    // send levelup pkt to client
    sess.m_ent->m_char->m_in_training = true; // flag character so we can handle dialog response
    sendLevelUp(sess);
}

void cmdHandler_FaceEntity(const QString &cmd, MapClientSession &sess)
{
    Entity *tgt = nullptr;
    QVector<QStringRef> parts;
    parts = cmd.splitRef(' ');

    if(parts.size() < 2)
    {
        qCDebug(logSlashCommand) << "Bad invocation:"<<cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:"+cmd, sess);
        return;
    }

    QString name = parts[1].toString();
    tgt = getEntity(&sess, name); // get Entity by name
    if(tgt == nullptr)
    {
        QString msg = QString("FaceEntity target %1 cannot be found.").arg(name);
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
        return;
    }
    sendFaceEntity(sess, tgt->m_idx);
}

void cmdHandler_FaceLocation(const QString &cmd, MapClientSession &sess)
{
    QVector<QStringRef> parts;
    parts = cmd.splitRef(' ');

    if(parts.size() < 4)
    {
        qCDebug(logSlashCommand) << "Bad invocation:"<<cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:"+cmd, sess);
        return;
    }

    glm::vec3 loc {
      parts[1].toFloat(),
      parts[2].toFloat(),
      parts[3].toFloat()
    };

    sendFaceLocation(sess, loc);
}

void cmdHandler_TestDeadNoGurney(const QString &cmd, MapClientSession &sess)
{
    qCDebug(logSlashCommand) << "Sending DeadNoGurney:" << cmd;
    sendDeadNoGurney(sess);
}

void cmdHandler_DoorMessage(const QString &cmd, MapClientSession &sess)
{
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry
    args.removeFirst(); // remove cmdstring

    if(args.size() < 2)
    {
        qCDebug(logSlashCommand) << "Bad invocation:" << cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:" + cmd, sess);
        return;
    }

    bool ok = true;
    uint32_t delay_status = args[0].toInt(&ok);
    args.removeFirst(); // remove integer
    QString msg = args.join(" ");

    if(!ok || delay_status > 2)
    {
        qCDebug(logSlashCommand) << "First argument must be 0, 1, or 2;" << cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "First argument must be 0, 1, or 2;" + cmd, sess);
        return;
    }

    sendDoorMessage(sess, DoorMessageStatus(delay_status), msg);
}

void cmdHandler_Browser(const QString &cmd, MapClientSession &sess)
{
    int space = cmd.indexOf(' ');
    QString content = cmd.mid(space+1);
    sendBrowser(sess, content);
}

void cmdHandler_SendTimeUpdate(const QString &/*cmd*/, MapClientSession &sess)
{
    // client expects PostgresEpoch of Jan 1 2000
    QDateTime base_date(QDate(2000,1,1));
    int32_t time_in_sec = static_cast<int32_t>(base_date.secsTo(QDateTime::currentDateTime()));

    sendTimeUpdate(sess, time_in_sec);
}

void cmdHandler_SendContactDialog(const QString &cmd, MapClientSession &sess)
{
    int space = cmd.indexOf(' ');
    QString content = cmd.mid(space+1);
    std::vector<ContactEntry> active_contacts;

    for(int i = 0; i < 4; ++i)
    {
        ContactEntry con;
        con.m_response_text = QString("Response #%1").arg(i);
        con.m_link = i; // a reference to contactLinkHash?
        active_contacts.push_back(con);
    }

    sendContactDialog(sess, content, active_contacts);
}

void cmdHandler_SendContactDialogYesNoOk(const QString &cmd, MapClientSession &sess)
{
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry
    args.removeFirst(); // remove cmdstring

    if(args.size() < 2)
    {
        qCDebug(logSlashCommand) << "Bad invocation:" << cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:" + cmd, sess);
        return;
    }

    bool ok = true;
    bool has_yesno = args[0].toInt(&ok);
    args.removeFirst(); // remove integer
    QString content = args.join(" ");

    if(!ok)
    {
        qCDebug(logSlashCommand) << "First argument must be boolean value;" << cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "First argument must be boolean value;" + cmd, sess);
        return;
    }

    sendContactDialogYesNoOk(sess, content, has_yesno);
}

void cmdHandler_SendWaypoint(const QString &cmd, MapClientSession &sess)
{
    QVector<QStringRef> parts;
    parts = cmd.splitRef(' ');

    if(parts.size() < 4)
    {
        qCDebug(logSlashCommand) << "Bad invocation: " << cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:" + cmd, sess);
        return;
    }

    Destination cur_dest = getCurrentDestination(*sess.m_ent);
    int idx = cur_dest.point_idx; // client will only change waypoint if idx == client_side_idx

    glm::vec3 loc {
      parts[1].toFloat(),
      parts[2].toFloat(),
      parts[3].toFloat()
    };

    QString msg = QString("Sending SendWaypoint: %1 <%2, %3, %4>")
                  .arg(idx)
                  .arg(loc.x, 0, 'f', 1)
                  .arg(loc.y, 0, 'f', 1)
                  .arg(loc.z, 0, 'f', 1);


    sendWaypoint(sess, idx, loc);
    setCurrentDestination(*sess.m_ent, idx, loc);
    sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
}

void cmdHandler_SetStateMode(const QString &cmd, MapClientSession &sess)
{
    uint32_t val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    sess.m_ent->m_rare_update = true; // this must also be true for statemode to send
    sess.m_ent->m_has_state_mode = true;
    sess.m_ent->m_state_mode = static_cast<ClientStates>(val);

    QString msg = "Set StateMode to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_Revive(const QString &cmd, MapClientSession &sess)
{
    Entity *tgt = nullptr;
    QString msg = "Revive format is '/revive {lvl} {optional: target_name}'";
    QVector<QStringRef> parts;
    parts = cmd.splitRef(' ');

    if(parts.size() < 2)
    {
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
        return;
    }

    int revive_lvl = parts[1].toUInt();
    if(parts.size() > 2)
    {
        QString name = parts[2].toString();
        tgt = getEntity(&sess, name); // get Entity by name
        if(tgt == nullptr)
        {
            msg = QString("Revive target %1 cannot be found. Targeting Self.").arg(name);
            qCDebug(logSlashCommand) << msg;
            sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
            tgt = sess.m_ent;
        }
    }
    else
        tgt = sess.m_ent;

    revivePlayer(*tgt, static_cast<ReviveLevel>(revive_lvl));

    msg = "Reviving " + tgt->name();
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_AddCostumeSlot(const QString &/*cmd*/, MapClientSession &sess)
{
    sess.m_ent->m_char->addCostumeSlot();
    markEntityForDbStore(sess.m_ent, DbStoreFlags::Full);

    QString msg = "Adding Costume Slot to " + sess.m_ent->name();
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_ContactStatusList(const QString &/*cmd*/, MapClientSession &sess)
{
    Contact startingContact;
    startingContact.setName("Officer Flint"); // "OfficerFlint
    startingContact.m_current_standing = 0;
    startingContact.m_notify_player = true;
    startingContact.m_task_index = 1;
    startingContact.m_npc_id = 1939; // Npc Id
    startingContact.m_has_location = true;
    startingContact.m_task_index = 0;
    startingContact.m_location_description = "Outbreak";
    startingContact.m_location.location.x = -62.0;
    startingContact.m_location.location.y = 0.0;
    startingContact.m_location.location.z = 182.0;
    startingContact.m_location.m_location_name = "Outbreak Starting";
    startingContact.m_location.m_location_map_name = "City_00_01"; //folder name?
    startingContact.m_confidant_threshold = 3;
    startingContact.m_friend_threshold = 2;
    startingContact.m_complete_threshold = 4;
    startingContact.m_can_use_cell = false;

    updateContactStatusList(sess, startingContact);
    QString msg = "Sending OfficerFlint to contactList";
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_SendLocations(const QString &/*cmd*/, MapClientSession &sess)
{
    VisitLocation visitlocation;
    visitlocation.m_location_name = "Test1";
    visitlocation.m_pos = glm::vec3(-44, 0, 261);

    sendLocation(sess, visitlocation);
}

void cmdHandler_SendConsoleOutput(const QString &cmd, MapClientSession &sess)
{
    QString msg;
    QVector<QStringRef> parts;
    parts = cmd.splitRef(' ');

    if(parts.size() != 2 )
    {
        qCDebug(logSlashCommand) << "SendConsoleOutput. Bad invocation:  " << cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "ConsoleOutput format is '/consoleOutput <Message>'", sess);
        return;
    }

    msg = parts[1].toString();
    sendDeveloperConsoleOutput(sess, msg);
}

void cmdHandler_SendConsolePrint(const QString &cmd, MapClientSession &sess)
{
    QString msg;
    QVector<QStringRef> parts;
    parts = cmd.splitRef(' ');

    if(parts.size() != 2)
    {
        qCDebug(logSlashCommand) << "SendConsolePrintF. Bad invocation:  " << cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "ConsolePrintF format is '/consolePrintF <Message>'", sess);
        return;
    }

    msg = parts[1].toString();
    sendClientConsoleOutput(sess, msg);
}

void cmdHandler_ClearTarget(const QString &cmd, MapClientSession &sess)
{
    QVector<QStringRef> parts;
    parts = cmd.splitRef(' ');

    if(parts.size () != 2)
    {
        qCDebug(logSlashCommand) << "ClearTarget. Bad invocation:  " << cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "ClearTarget '/clearTarget <targetIdx>'", sess);
        return;
    }

    int idx = parts[1].toInt();
    setTarget(*sess.m_ent, idx);
}
void cmdHandler_StartTimer(const QString &cmd, MapClientSession &sess)
{
    QVector<QStringRef> parts;
    parts = cmd.splitRef(' ');

    if(parts.size () != 3)
    {
        qCDebug(logSlashCommand) << "StartTimer. Bad invocation:  " << cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "StartTimer '/StartTimer <timerName> <seconds>'", sess);
        return;
    }
    QString message = parts[1].toString();
    float time = parts[2].toFloat();

    sendMissionObjectiveTimer(sess, message, time);

}

// Slash commands for setting bit values
void cmdHandler_SetU1(const QString &cmd, MapClientSession &sess)
{
    uint32_t val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu1(*sess.m_ent, val);

    QString msg = "Set u1 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 2 Commands
void cmdHandler_AddNPC(const QString &cmd, MapClientSession &sess)
{
    QVector<QStringRef> parts;
    int variation = 0;

    if(cmd.contains('"')) // assume /addNpc "A guy in a hat" 1
    {
        int start_idx = cmd.indexOf('"');
        int end_idx = cmd.indexOf('"',start_idx+1);
        parts.push_back(cmd.midRef(0,start_idx-1));

        if(end_idx!=-1)
            parts.push_back(cmd.midRef(start_idx+1,end_idx-start_idx-2));
        if(cmd.midRef(end_idx+1).size()>0)
            parts.push_back(cmd.midRef(end_idx+1));
    }
    else
        parts = cmd.splitRef(' ');

    if(parts.size()>2) // assume /addNpc Monsterifier 1
        variation = parts[2].toInt();

    if(parts.size()<2)
    {
        qCDebug(logSlashCommand) << "Bad invocation:"<<cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:"+cmd, sess);
        return;
    }

    QString name = parts[1].toString();
    glm::vec3 offset = glm::vec3 {2,0,1};
    glm::vec3 gm_loc = sess.m_ent->m_entity_data.m_pos + offset;
    addNpc(sess, name, gm_loc, variation, name);
}

void cmdHandler_MoveTo(const QString &cmd, MapClientSession &sess)
{
    QVector<QStringRef> parts;
    parts = cmd.splitRef(' ');
    if(parts.size()<4)
    {
        qCDebug(logSlashCommand) << "Bad invocation:"<<cmd;
        sendInfoMessage(MessageChannel::USER_ERROR, "Bad invocation:"+cmd, sess);
        return;
    }

    glm::vec3 new_pos {
      parts[1].toFloat(),
      parts[2].toFloat(),
      parts[3].toFloat()
    };

    forcePosition(*sess.m_ent,new_pos);
    sendInfoMessage(MessageChannel::DEBUG_INFO, QString("New position set"), sess);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 1 Commands
void cmdHandler_CmdList(const QString &cmd, MapClientSession &sess)
{

    QString msg = "Below is a list of all slash commands that your account can access. They are not case sensitive.\n";
    QString content = "<face heading><span align=center><color #ff0000>Command List</color></span></face><br>\n<br>\n";

    for (const auto &sc : g_defined_slash_commands)
    {
        uint32_t alvl = getAccessLevel(*sess.m_ent);
        if(alvl < sc.m_required_access_level)
            continue;

        // We don't want to show acess_level 0 (Under the Hood commands)
        if(sc.m_required_access_level == 0)
            continue;

        // Use msg for std out, msg_dlg for ingame dialog box
        msg += "\t" + sc.m_valid_prefixes.join(", ") + " [" + QString::number(sc.m_required_access_level) +
               "]:\t" + sc.m_help_text + "\n";
        content += QString("<color #ffCC99><i>%1</i></color>[<color #66ffff>%2</color>]: %3<br>")
                       .arg(sc.m_valid_prefixes.join(", "))
                       .arg(sc.m_required_access_level)
                       .arg(sc.m_help_text);
    }

    // Browser output
    sess.addCommand<Browser>(content);
    // CMD line (debug) output
    qCDebug(logSlashCommand).noquote() << cmd << ":\n" << msg;
}

void cmdHandler_AFK(const QString &cmd, MapClientSession &sess)
{
    Entity* e = sess.m_ent;

    int space = cmd.indexOf(' ');
    QString val = cmd.mid(space+1);
    toggleAFK(*e->m_char, val);

    QString msg = "Setting afk message to: " + val;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::EMOTE, msg, sess);

    // the server regards writing on chat (including cmd commands) as an input
    // so specifically for afk, we treat it as a non-input in this way
    e->m_has_input_on_timeframe = false;
}

void cmdHandler_WhoAll(const QString &/*cmd*/, MapClientSession &sess)
{
    MapInstance *     mi  = sess.m_current_map;

    QString msg = "Players on this map:\n";

    for (MapClientSession *cl : mi->m_session_store)
    {
        Character &c(*cl->m_ent->m_char);
        QString    name      = cl->m_ent->name();
        QString    lvl       = QString::number(getLevel(c));
        QString    clvl      = QString::number(getCombatLevel(c));
        QString    origin    = getOrigin(c);
        QString    archetype = QString(getClass(c)).remove("Class_");

        // Format: character_name "lvl" level "clvl" combat_level origin archetype
        msg += QString("%1 lvl %2 clvl %3 %4 %5\n").arg(name,lvl,clvl,origin,archetype);
    }

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, sess);
}

void cmdHandler_SetTitles(const QString &cmd, MapClientSession &sess)
{
    int space = cmd.indexOf(' ');
    QString val = cmd.mid(space+1);

    setTitle(sess, val);
}

void cmdHandler_SetCustomTitles(const QString &cmd, MapClientSession &sess)
{
    bool        prefix;
    QString     msg, generic, origin, special;
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    if(cmd.toLower() == "settitles")
    {
        setTitles(*sess.m_ent->m_char);
        msg = "Titles reset to nothing";
    }
    else
    {
        prefix  = !args.value(1).isEmpty();
        generic = args.value(2);
        origin  = args.value(3);
        special = args.value(4);
        setTitles(*sess.m_ent->m_char, prefix, generic, origin, special);
        msg = "Titles changed to: " + QString::number(prefix) + " " + generic + " " + origin + " " + special;
    }
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
}

void cmdHandler_SetSpecialTitle(const QString &cmd, MapClientSession &sess)
{
    bool        prefix;
    QString     msg, generic, origin, special;
    int space = cmd.indexOf(' ');
    prefix = sess.m_ent->m_char->m_char_data.m_has_the_prefix;
    generic = getGenericTitle(*sess.m_ent->m_char);
    origin = getOriginTitle(*sess.m_ent->m_char);
    special = cmd.mid(space+1);

    setTitles(*sess.m_ent->m_char, prefix, generic, origin, special);
    msg = "Titles changed to: " + QString::number(prefix) + " " + generic + " " + origin + " " + special;

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
}

void cmdHandler_Stuck(const QString &cmd, MapClientSession &sess)
{
    // TODO: Implement true move-to-safe-location-nearby logic
    //forcePosition(*sess.m_ent, sess.m_current_map->closest_safe_location(sess.m_ent->m_entity_data.m_pos));
    sess.m_current_map->setPlayerSpawn(*sess.m_ent);

    QString msg = QString("Resetting location to default spawn <%1, %2, %3>")
            .arg(sess.m_ent->m_entity_data.m_pos.x, 0, 'f', 1)
            .arg(sess.m_ent->m_entity_data.m_pos.y, 0, 'f', 1)
            .arg(sess.m_ent->m_entity_data.m_pos.z, 0, 'f', 1);

    qCDebug(logSlashCommand) << cmd << ":" << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, sess);
}

void cmdHandler_SetSpawnLocation(const QString &cmd, MapClientSession &sess)
{
    const QString spawnLocation = getCommandParameter(cmd);
    if(spawnLocation.isEmpty())
    {
        // No SpawnLocation given, bail.
        return;
    }
    sess.m_current_map->setSpawnLocation(*sess.m_ent, spawnLocation);
}

void cmdHandler_LFG(const QString &cmd, MapClientSession &sess)
{
    toggleLFG(*sess.m_ent);
    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, sess);
}

void cmdHandler_MOTD(const QString &/*cmd*/, MapClientSession &sess)
{
    sendServerMOTD(&sess);
    QString msg = "Opening Server MOTD";
    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, sess);
}

void cmdHandler_Invite(const QString &cmd, MapClientSession &sess)
{
    Entity* const tgt = getEntityFromCommand(cmd, sess);
    if(tgt == nullptr)
    {
        return;
    }

    if(tgt->m_has_team)
    {
        const QString msg = tgt->name() + " is already on a team.";
        qCDebug(logTeams) << msg;
        sendInfoMessage(MessageChannel::SERVER, msg, sess);
        return;
    }

    if(tgt->name() == sess.m_name)
    {
        const QString msg = "You cannot invite yourself to a team.";
        qCDebug(logTeams) << msg;
        sendInfoMessage(MessageChannel::SERVER, msg, sess);
        return;
    }

    if(sess.m_ent->m_has_team && sess.m_ent->m_team != nullptr)
    {
        if(!sess.m_ent->m_team->isTeamLeader(sess.m_ent))
        {
            const QString msg = "Only the team leader can invite players to the team.";
            qCDebug(logTeams) << sess.m_ent->name() << msg;
            sendInfoMessage(MessageChannel::TEAM, msg, sess);
            return;
        }
    }

    sendTeamOffer(sess, *tgt->m_client);
}

void cmdHandler_Kick(const QString &cmd, MapClientSession &sess)
{
    Entity* const tgt = getEntityFromCommand(cmd, sess);
    if(tgt == nullptr)
    {
        return;
    }

    const QString name = tgt->name();
    QString msg;
    if(kickTeam(*tgt))
        msg = "Kicking " + name + " from team.";
    else
        msg = "Failed to kick " + name;

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::TEAM, msg, sess);
}

void cmdHandler_LeaveTeam(const QString &/*cmd*/, MapClientSession &sess)
{
    leaveTeam(*sess.m_ent);
    QString msg = "Leaving Team";
    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::TEAM, msg, sess);
}

void cmdHandler_FindMember(const QString &/*cmd*/, MapClientSession &sess)
{
    sendTeamLooking(sess);
    QString msg = "Finding Team Member";
    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::CHAT_TEXT, msg, sess);
}


void cmdHandler_MakeLeader(const QString &cmd, MapClientSession &sess)
{
    Entity* const tgt = getEntityFromCommand(cmd, sess);
    if(tgt == nullptr)
    {
        return;
    }

    const QString name = tgt->name();
    QString msg;
    if(makeTeamLeader(*sess.m_ent,*tgt))
        msg = "Making " + name + " team leader.";
    else
        msg = "Failed to make " + name + " team leader.";

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::TEAM, msg, sess);
}

void cmdHandler_SetAssistTarget(const QString &/*cmd*/, MapClientSession &sess)
{
    // it appears that `/assist` should target the target of your current target
    // but it also seems that what we call m_assist_target_idx is actually
    // the last friendly target you selected. We need to check target type
    // and set m_target_idx or m_assist_target_idx depending on result.

    Entity *target_ent = getEntity(&sess, getTargetIdx(*sess.m_ent));
    if(target_ent == nullptr)
        return;

    uint32_t new_target = getTargetIdx(*target_ent);
    if(new_target == 0)
        return;

    if(target_ent->m_is_villian)
        setTarget(*sess.m_ent, new_target);
    else
        setAssistTarget(*sess.m_ent, new_target);

    QString msg = "Now targeting " + target_ent->name() + "'s target";
    sendInfoMessage(MessageChannel::TEAM, msg, sess);
    qCDebug(logSlashCommand).noquote() << msg;
}

void cmdHandler_Sidekick(const QString &cmd, MapClientSession &sess)
{
    Entity* const tgt = getEntityFromCommand(cmd, sess);
    if(tgt == nullptr || sess.m_ent->m_char->isEmpty() || tgt->m_char->isEmpty())
    {
        return;
    }

    auto res=inviteSidekick(*sess.m_ent, *tgt);
    static const QLatin1Literal possible_messages[] = {
        QLatin1String("Unable to add sidekick."),
        QLatin1String("To Mentor another player, you must be at least 3 levels higher than them."),
        QLatin1String("To Mentor another player, you must be at least level 10."),
        QLatin1String("You are already Mentoring someone."),
        QLatin1String("Target is already a sidekick."),
        QLatin1String("To Mentor another player, you must be on the same team."),
    };
    if(res==SidekickChangeStatus::SUCCESS)
    {
        // sendSidekickOffer
        sendSidekickOffer(*tgt->m_client, sess.m_ent->m_db_id); // tgt gets dialog, src.db_id is named.
    }
    else
    {
        qCDebug(logTeams).noquote() << possible_messages[int(res)-1];
        sendInfoMessage(MessageChannel::USER_ERROR, possible_messages[int(res)-1], sess);
    }
}

void cmdHandler_UnSidekick(const QString &/*cmd*/, MapClientSession &sess)
{
    if(sess.m_ent->m_char->isEmpty())
        return;
    QString msg;

    uint32_t sidekick_id = getSidekickId(*sess.m_ent->m_char);
    auto res = removeSidekick(*sess.m_ent, sidekick_id);
    if(res==SidekickChangeStatus::GENERIC_FAILURE)
    {
        msg = "You are not sidekicked with anyone.";
        qCDebug(logTeams).noquote() << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
    }
    else if(res==SidekickChangeStatus::SUCCESS)
    {
        Entity *tgt = getEntityByDBID(sess.m_current_map, sidekick_id);
        QString tgt_name = tgt ? tgt->name() : "Unknown Player";
        if(isSidekickMentor(*sess.m_ent))
        {
            // src is mentor, tgt is sidekick
            msg = QString("You are no longer mentoring %1.").arg(tgt_name);
            sendInfoMessage(MessageChannel::TEAM, msg, sess);
            if(tgt)
            {
                msg = QString("%1 is no longer mentoring you.").arg(sess.m_ent->name());
                sendInfoMessage(MessageChannel::TEAM, msg, *tgt->m_client);
            }
        }
        else
        {
            // src is sidekick, tgt is mentor
            if(tgt)
            {
                msg = QString("You are no longer mentoring %1.").arg(sess.m_ent->name());
                sendInfoMessage(MessageChannel::TEAM, msg, *tgt->m_client);
            }
            msg = QString("%1 is no longer mentoring you.").arg(tgt_name);
            sendInfoMessage(MessageChannel::TEAM, msg, sess);
        }
    }
    else if(res==SidekickChangeStatus::NOT_SIDEKICKED_CURRENTLY)
    {
        msg = QString("You are no longer sidekicked with anyone.");
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
    }
}

void cmdHandler_TeamBuffs(const QString & /*cmd*/, MapClientSession &sess)
{
    toggleTeamBuffs(*sess.m_ent->m_player);

    QString msg = "Toggling Team Buffs display mode.";
    qCDebug(logSlashCommand).noquote() << msg;
}

void cmdHandler_Friend(const QString &cmd, MapClientSession &sess)
{
    const Entity* const tgt = getEntityFromCommand(cmd, sess);
    if(tgt == nullptr || sess.m_ent->m_char->isEmpty() || tgt->m_char->isEmpty())
    {
        return;
    }

    FriendListChangeStatus status = addFriend(*sess.m_ent, *tgt,getEntityDisplayMapName(tgt->m_entity_data));
    if(status==FriendListChangeStatus::MAX_FRIENDS_REACHED)
    {
        QString msg = "You cannot have more than " + QString::number(g_max_friends) + " friends.";
        qCDebug(logFriends).noquote() << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
    }
    else
    {
        QString msg = "Adding " + tgt->name() + " to your friendlist.";
        qCDebug(logFriends).noquote() << msg;
        sendInfoMessage(MessageChannel::FRIENDS, msg, sess);
        // Send FriendsListUpdate
        sendFriendsListUpdate(sess, sess.m_ent->m_char->m_char_data.m_friendlist);
    }
}

void cmdHandler_Unfriend(const QString &cmd, MapClientSession &sess)
{
    // Cannot use getEntityFromCommand as we need to be able to unfriend logged out characters.
    QString name = getCommandParameter(cmd);
    if(name.isEmpty())
    {
        const Entity* const tgt = getTargetEntity(sess);
        if(tgt == nullptr)
            return;

        name = tgt->name();
    }

    FriendListChangeStatus status =  removeFriend(*sess.m_ent, name);
    if(status==FriendListChangeStatus::FRIEND_REMOVED)
    {
        QString msg = "Removing " + name + " from your friends list.";
        sendInfoMessage(MessageChannel::FRIENDS, msg, sess);

        // Send FriendsListUpdate
        sendFriendsListUpdate(sess, sess.m_ent->m_char->m_char_data.m_friendlist);
    }
    else
    {
        QString msg = name + " is not on your friends list.";
        sendInfoMessage(MessageChannel::USER_ERROR, msg, sess);
    }
}

void cmdHandler_FriendList(const QString &/*cmd*/, MapClientSession &sess)
{
    if(sess.m_ent->m_char->isEmpty())
        return;

    toggleFriendList(*sess.m_ent);
}

void cmdHandler_MapXferList(const QString &/*cmd*/, MapClientSession &sess)
{
    showMapMenu(sess);
}

void cmdHandler_ReSpec(const QString &/*cmd*/, MapClientSession &sess)
{
    CharacterData &cd = sess.m_ent->m_char->m_char_data;

    if(sess.m_ent->m_char->isEmpty())
        return;

    QString msg = "No powersets found for player " + sess.m_ent->name();

    if(cd.m_powersets.size() > 1)
    {
        msg = "Removing all powers for player " + sess.m_ent->name();
        cd.m_reset_powersets = true;
        cd.m_has_updated_powers = true;
    }

    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
    qCDebug(logSlashCommand).noquote() << msg;
}

void cmdHandler_Trade(const QString &cmd, MapClientSession &sess)
{
    Entity* const tgt = getEntityFromCommand(cmd, sess);
    if(tgt == nullptr || sess.m_ent == nullptr)
        return;

    QString msg, tgt_msg;
    TradeSystemMessages result;
    result = requestTrade(*sess.m_ent, *tgt);

    switch(result)
    {
    case TradeSystemMessages::SRC_ALREADY_IN_TRADE:
        msg = "You are already in a trade.";
        break;
    case TradeSystemMessages::SRC_CONSIDERING_ANOTHER_TRADE:
        msg = "You are already considering a trade offer.";
        break;
    case TradeSystemMessages::TGT_ALREADY_IN_TRADE:
        msg = tgt->name() + " is already in a trade.";
        break;
    case TradeSystemMessages::TGT_CONSIDERING_ANOTHER_TRADE:
        msg = tgt->name() + " is already considering a trade offer.";
        break;
    case TradeSystemMessages::SEND_TRADE_OFFER:
        sendTradeOffer(*tgt->m_client, sess.m_ent->name()); // send tradeOffer
        msg = QString("You sent a trade request to %1.").arg(tgt->name());
        tgt_msg = sess.m_ent->name() + " sent a trade request.";
        sendInfoMessage(MessageChannel::SERVER, tgt_msg, *tgt->m_client);
        break;
    default:
        msg = "Something went wrong with trade request!"; // this should never happen
    }

    sendInfoMessage(MessageChannel::SERVER, msg, *sess.m_ent->m_client);
}

void cmdHandler_Tailor(const QString &/*cmd*/, MapClientSession &sess)
{
    sendTailorOpen(sess);
}

void cmdHandler_CostumeChange(const QString &cmd, MapClientSession &sess)
{
    uint32_t costume_idx = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setCurrentCostumeIdx(*sess.m_ent->m_char, costume_idx);

    QString msg = "Changing costume to: " + QString::number(costume_idx);
    qCDebug(logTailor) << msg;
}

void cmdHandler_Train(const QString &/*cmd*/, MapClientSession &sess)
{
    playerTrain(sess);
}

void cmdHandler_Kiosk(const QString &/*cmd*/, MapClientSession &sess)
{
    sendKiosk(sess);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Access Level 0 Commands
void cmdHandler_TeamAccept(const QString &cmd, MapClientSession &sess)
{
    // game command: "team_accept \"From\" to_db_id to_db_id \"To\""

    QString msgfrom = "Something went wrong with TeamAccept.";
    QString msgtgt = "Something went wrong with TeamAccept.";
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    QString from_name       = args.value(1);
    uint32_t tgt_db_id      = args.value(2).toUInt();
    uint32_t tgt_db_id_2    = args.value(3).toUInt(); // always the same?
    QString tgt_name        = args.value(4);

    if(tgt_db_id != tgt_db_id_2)
        qWarning() << "TeamAccept db_ids do not match!";

    Entity *from_ent = getEntity(&sess,from_name);
    if(from_ent == nullptr)
        return;

    if(inviteTeam(*from_ent,*sess.m_ent))
    {
        msgfrom = "Inviting " + tgt_name + " to team.";
        msgtgt = "Joining " + from_name + "'s team.";

    }
    else
    {
        msgfrom = "Failed to invite " + tgt_name + ". They are already on a team.";
    }

    qCDebug(logSlashCommand).noquote() << msgfrom;
    sendInfoMessage(MessageChannel::TEAM, msgfrom, *from_ent->m_client);
    sendInfoMessage(MessageChannel::TEAM, msgtgt, sess);
}

void cmdHandler_TeamDecline(const QString &cmd, MapClientSession &sess)
{
    // game command: "team_decline \"From\" to_db_id \"To\""
    QString msg;
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    QString from_name   = args.value(1);
    uint32_t tgt_db_id  = args.value(2).toUInt();
    QString tgt_name    = args.value(3);

    Entity *from_ent = getEntity(&sess,from_name);
    if(from_ent == nullptr)
        return;

    msg = tgt_name + " declined a team invite from " + from_name + QString::number(tgt_db_id);
    qCDebug(logSlashCommand).noquote() << msg;

    msg = tgt_name + " declined your team invite."; // to sender
    sendInfoMessage(MessageChannel::TEAM, msg, *from_ent->m_client);
    msg = "You declined the team invite from " + from_name; // to target
    sendInfoMessage(MessageChannel::TEAM, msg, sess);
}

void cmdHandler_SidekickAccept(const QString &/*cmd*/, MapClientSession &sess)
{
    uint32_t db_id  = sess.m_ent->m_char->m_char_data.m_sidekick.m_db_id;
    //TODO: Check that entity is in the same map ?
    Entity *tgt     = getEntityByDBID(sess.m_current_map,db_id);
    if(tgt == nullptr || sess.m_ent->m_char->isEmpty() || tgt->m_char->isEmpty())
        return;

    addSidekick(*sess.m_ent,*tgt);
    // Send message to each player
    QString msg = QString("You are now Mentoring %1.").arg(tgt->name()); // Customize for src.
    sendInfoMessage(MessageChannel::TEAM, msg, sess);
    msg = QString("%1 is now Mentoring you.").arg(sess.m_ent->name()); // Customize for src.
    sendInfoMessage(MessageChannel::TEAM, msg, *tgt->m_client);
}

void cmdHandler_SidekickDecline(const QString &/*cmd*/, MapClientSession &sess)
{
    sess.m_ent->m_char->m_char_data.m_sidekick.m_db_id = 0;
}

void cmdHandler_EmailHeaders(const QString & /*cmd*/, MapClientSession &sess)
{
    getEmailHeaders(sess);
}

void cmdHandler_EmailRead(const QString &cmd, MapClientSession &sess)
{
    uint32_t id = cmd.midRef(cmd.indexOf(' ')+1).toInt();

    readEmailMessage(sess, id);
}

void cmdHandler_EmailSend(const QString &cmd, MapClientSession &sess)
{
    QStringList parts = cmd.split("\"");
    QStringList result;

    for (const auto &part : parts)
    {
        if(part.endsWith('\\') && !result.isEmpty() && !result.back().isEmpty())
            result.back() += part+"\"";
        else
            result.push_back(part);
    }

    if (result.size() != 5)
    {
        sendInfoMessage(MessageChannel::SERVER, "Argument count for sending email is not correct! Please send emails from the email window instead.", sess);
        return;
    }

    result[1].replace("\\q ", ";");
    result[1].replace("\\q", "");

    // result[1] -> recipient name, 3 -> email subject, 4 -> email message
    // attempt to parse names if sending to multiple recipients
    QStringList recipients = result[1].split(";");

    // the last element will be empty in all cases (if sent through email window), so remove it
    recipients.pop_back();

    // first, check if your own character is one of the recipients in the email
    // cannot send email to self as that will trigger /emailRead without the data in db nor EmailHandler
    // and that will segfault the server :)
    if (recipients.contains(sess.m_ent->m_char->getName()))
    {
        sendInfoMessage(MessageChannel::SERVER, "You cannot send an email to yourself!", sess);
        return;
    }

    for (const auto &recipient : recipients)
        sendEmail(sess, recipient, result[3], result[4]);
}

void cmdHandler_EmailDelete(const QString &cmd, MapClientSession &sess)
{
    uint32_t id = cmd.midRef(cmd.indexOf(' ')+1).toInt();

    deleteEmailHeaders(sess, id);

    QString msg = "Email Deleted ID: " + QString::number(id);
    qDebug().noquote() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_TradeAccept(const QString &cmd, MapClientSession &sess)
{
    // Game command: "trade_accept \"From\" to_db_id to_db_id \"To\""
    const QStringList args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry
    if(args.size() < 4)
    {
        qWarning() << "Wrong number of arguments for TradeAccept:" << cmd;
        discardTrade(*sess.m_ent);
        return;
    }

    // We need only the "from" name.
    const QString from_name = args.value(1);
    Entity* const from_ent = getEntity(&sess, from_name);
    if(from_ent == nullptr)
    {
        discardTrade(*sess.m_ent);
        return;
    }

    QString msg, tgt_msg;
    TradeSystemMessages result;
    result = acceptTrade(*sess.m_ent, *from_ent);

    switch(result)
    {
    case TradeSystemMessages::HAS_SENT_NO_TRADE:
        msg = "You have not sent a trade offer.";
        break;
    case TradeSystemMessages::TGT_RECV_NO_TRADE:
        msg = QString("%1 has not received a trade offer.").arg(from_ent->name());
        break;
    case TradeSystemMessages::SRC_RECV_NO_TRADE:
        msg = QString("You are not considering a trade offer from %1.").arg(from_ent->name());
        break;
    case TradeSystemMessages::ACCEPTED_TRADE:
        sendTradeInit(*sess.m_ent->m_client, *from_ent->m_client); // Initiate trade
        msg = QString("You accepted the trade invite from %1.").arg(from_ent->name());
        tgt_msg = sess.m_ent->name() + " accepted your trade invite.";
        sendInfoMessage(MessageChannel::SERVER, tgt_msg, *from_ent->m_client);
        break;
    default:
        msg = "Something went wrong with trade accept!"; // this should never happen
    }

    sendInfoMessage(MessageChannel::SERVER, msg, sess);
}

void cmdHandler_TradeDecline(const QString &cmd, MapClientSession &sess)
{
    // Game command: "trade_decline \"From\" to_db_id \"To\""
    const QStringList args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry
    if(args.size() < 4)
    {
        qWarning() << "Wrong number of arguments for TradeDecline:" << cmd;
        discardTrade(*sess.m_ent);
        return;
    }

    // We need only the "from" name.
    const QString from_name  = args.value(1);
    Entity* const from_ent = getEntity(&sess, from_name);
    if(from_ent == nullptr)
    {
        discardTrade(*sess.m_ent);
        return;
    }

    QString msg, tgt_msg;
    TradeSystemMessages result;
    result = declineTrade(*sess.m_ent, *from_ent);

    switch(result)
    {
    case TradeSystemMessages::HAS_SENT_NO_TRADE:
        msg = "You have not sent a trade offer.";
        break;
    case TradeSystemMessages::TGT_RECV_NO_TRADE:
        msg = QString("%1 has not received a trade offer.").arg(from_ent->name());
        break;
    case TradeSystemMessages::SRC_RECV_NO_TRADE:
        msg = QString("You are not considering a trade offer from %1.").arg(from_ent->name());
        break;
    case TradeSystemMessages::DECLINED_TRADE:
        msg = QString("You declined the trade invite from %1.").arg(from_ent->name());
        tgt_msg = sess.m_ent->name() + " declined your trade invite.";
        sendInfoMessage(MessageChannel::SERVER, tgt_msg, *from_ent->m_client);
        break;
    default:
        msg = "Something went wrong with trade decline!"; // this should never happen
    }

    sendInfoMessage(MessageChannel::SERVER, msg, sess);
}

bool canAccessCommand(const SlashCommand &cmd, MapClientSession &src)
{
    uint32_t alvl = getAccessLevel(*src.m_ent);
    if(alvl >= cmd.m_required_access_level)
        return true;

    QString msg = "You do not have adequate permissions to use the command: " + cmd.m_valid_prefixes.first();
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::USER_ERROR, msg, src);
    return false;
}

void cmdHandler_AddTestTask(const QString &/*cmd*/, MapClientSession &sess)
{
    Task tk;
    tk.m_db_id = 1;
    tk.setDescription("Task Description Goes Here");
    tk.setOwner("OfficerFlint");
    tk.setState("In Progress");
    tk.setDetail("Task detail goes here");
    tk.m_is_complete = false;
    tk.m_in_progress_maybe = true;
    tk.m_is_abandoned = false;
    tk.m_has_location = false;
    tk.m_detail_invalid = true; // aka "needs update"
    tk.m_board_train = false;
    tk.m_location.location = glm::vec3(-83.0, 0.0, 1334.0);
    tk.m_location.setLocationMapName("Outbreak");
    tk.m_location.setLocationName("Outbreak");
    tk.m_finish_time = 0;
    tk.m_unknown_1 = 0;
    tk.m_unknown_2 = 0;
    tk.m_task_idx = 0;

    sendUpdateTaskStatusList(sess, tk);
    QString msg = "Sending Test Task to client";
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, sess);
}

void cmdHandler_ReloadScripts(const QString &/*cmd*/, MapClientSession &sess)
{
    qCDebug(logSlashCommand) << "Reloading all Lua scripts in" << sess.m_current_map->name();

    // Reset script engine
    sess.m_current_map->m_scripting_interface.reset(new ScriptingEngine);
    sess.m_current_map->m_scripting_interface->setIncludeDir(sess.m_current_map->name());
    sess.m_current_map->m_scripting_interface->registerTypes();

    // load all scripts again
    // TODO: this will regenerate any NPCs (luabot) that exist
    sess.m_current_map->load_map_lua();
}


void cmdHandler_OpenStore(const QString &/*cmd*/, MapClientSession &sess)
{
    qCDebug(logSlashCommand) << "OpenStore...";
    openStore(sess, 0); // Default entity_idx as it doesn't change anything currently
}
  
void cmdHandler_ForceLogout(const QString &cmd, MapClientSession &sess)
{
    QVector<QStringRef> parts;
    parts = cmd.splitRef(' ');
    QString message;
    QString name = parts[1].toString();
    int first_space = cmd.indexOf(' ');
    int second_space = cmd.indexOf(' ',first_space+1);

    if(second_space == -1)
    {
        message = "ForceLogout requires a logout message. /forceLogout <HeroName> <Message!>";
        qCDebug(logSlashCommand) << message;
        sendInfoMessage(MessageChannel::USER_ERROR, message, sess);
        return;
    }
    else
    {
        message = cmd.mid(second_space + 1);
    }

    sendForceLogout(sess, name, message);
}


} // end of anonymous namespace

/*
 * runCommand for executing commands on MapClientSession
 */
void runCommand(const QString &str, MapClientSession &e)
{
    for (const auto &cmd : g_defined_slash_commands)
    {
        if(cmd.m_valid_prefixes.contains(str.split(' ').front(), Qt::CaseInsensitive))
        {
            if(!canAccessCommand(cmd, e))
                return; // no access, so return early
            cmd.m_handler(str, e);
            return; // return here to avoid unknown command msg
        }
    }
    qCDebug(logSlashCommand) << "Unknown game command:" << str;
}



//! @}

