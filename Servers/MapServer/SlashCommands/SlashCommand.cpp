/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup SlashCommands Projects/CoX/Servers/MapServer/SlashCommands
 * @{
 */

#include "SlashCommand.h"
#include "SlashCommand_Contacts.h"
#include "SlashCommand_Debug.h"
#include "SlashCommand_Email.h"
#include "SlashCommand_Friends.h"
#include "SlashCommand_Misc.h"
#include "SlashCommand_Movement.h"
#include "SlashCommand_Powers.h"
#include "SlashCommand_Scripts.h"
#include "SlashCommand_Settings.h"
#include "SlashCommand_Stats.h"
#include "SlashCommand_SuperGroup.h"
#include "SlashCommand_Teams.h"
#include "SlashCommand_Trade.h"

#include "GameData/EntityHelpers.h"
#include "Components/Logging.h"
#include "MapInstance.h"
#include "Messages/Map/Browser.h"
#include "MessageHelpers.h"

#include <QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QDebug>

using namespace SEGSEvents;

namespace  {
class InfoMessageCmd; // leverage InfoMessageCmd

struct SlashCommand
{
    QStringList m_valid_prefixes;
    QString m_help_text;
    std::function<void(const QStringList &, MapClientSession &)> m_handler;
    uint32_t m_required_access_level;
};

// bool canAccessCommand(const SlashCommand &cmd, const Entity &e); --> function not defined (yet)
bool canAccessCommand(const SlashCommand &cmd, MapClientSession &src);
// CmdList needs access to g_defined_slash_commands
void cmdHandler_CmdList(const QStringList &params, MapClientSession &sess);

static const SlashCommand g_defined_slash_commands[] = {
    /* SlashCommand - Access Level 1 */
    {{"cmdlist","commandlist"},"List all accessible commands", cmdHandler_CmdList, 1},

    /* SlashCommand_Contacts - Access Level 9 */
    {{"contactdlg", "cdlg"}, "Test ContactDialog. Sends contact dialog with responses to server", cmdHandler_SendContactDialog, 9},
    {{"contactdlgyesno", "cdlg2"}, "Test ContactDialogYesNoOk. Sends contact dialog with yes/no response to server", cmdHandler_SendContactDialogYesNoOk, 9},
    {{"contactList"}, "Update Contact List", cmdHandler_ContactStatusList, 9},

    /* SlashCommand_Debug - Access Level 9 */
    {{"Script"},"Run a Script", cmdHandler_Script, 9},
    {{"Dialog", "dlg"},"Open a dialog box with any string arg", cmdHandler_Dialog, 9},
    {{"InfoMessage", "imsg"},"Send an info message into chat. Expects <int> <string>, e.g. /imsg 1 test", cmdHandler_InfoMessage, 9},
    {{"SmileX"},"Runs the smlx script SmileX", cmdHandler_SmileX, 9},
    {{"DebugChar", "chardebug"},"View your characters debug information", cmdHandler_DebugChar, 9},
    {{"UpdateChar", "CharUpdate", "save"},"Update character information in the database", cmdHandler_UpdateChar, 9},
    {{"settingsDump","settingsDebug"},"Output settings.cfg to console", cmdHandler_SettingsDump, 9},
    {{"guiDump", "guiDebug"}, "Output gui settings to console", cmdHandler_GUIDebug, 9},
    {{"setWindowVisibility", "setWinVis"}, "", cmdHandler_SetWindowVisibility, 9},
    {{"keybindDump", "keybindDebug"}, "Output keybind settings to console", cmdHandler_KeybindDebug, 9},
    {{"toggleLogging", "log"}, "Modify log categories (e.g. input, teams, ...)", cmdHandler_ToggleLogging, 9},
    {{"damage", "heal"}, "Make current target (or self) take damage/health", cmdHandler_SendFloatingNumbers, 9},
    {{"setSeq"},"Set Sequence values <update> <move_idx> <duration>", &cmdHandler_SetSequence, 9},
    {{"addTriggeredMove"},"Set TriggeredMove values <move_idx> <delay> <fx_idx>", &cmdHandler_AddTriggeredMove, 9},
    {{"timestate", "setTimeStateLog"},"Set TimeStateLog value.", cmdHandler_AddTimeStateLog, 9},
    {{"clientstate"},"Set ClientState mode", &cmdHandler_SetClientState, 9},
    {{"levelupxp"},"Level Up Character to Level Provided", &cmdHandler_LevelUpXp, 9},
    {{"deadnogurney"}, "Test Dead No Gurney. Fakes sending the client packet.", cmdHandler_TestDeadNoGurney, 9},
    {{"doormsg"}, "Test Door Message. Fakes sending the client packet.", cmdHandler_DoorMessage, 9},
    {{"browser"}, "Test Browser. Sends content to a browser window", cmdHandler_Browser, 9},
    {{"timeupdate"}, "Test TimeUpdate. Sends time update to server", cmdHandler_SendTimeUpdate, 9},
    {{"setwaypoint"}, "Test SendWaypoint. Send waypoint to client", cmdHandler_SendWaypoint, 9},
    {{"setstatemode"}, "Send StateMode. Send StateMode to client", cmdHandler_SetStateMode, 9},
    {{"revive"}, "Revive Self or Target Player", cmdHandler_Revive, 9},
    {{"addcostumeslot"}, "Add a new costume slot to Character", cmdHandler_AddCostumeSlot, 9},
    {{"testTask"}, "Test Task", cmdHandler_AddTestTask, 9},
    {{"reloadLua"}, "Reload all Lua scripts", cmdHandler_ReloadScripts, 9},
    {{"openStore"}, "Open store Window", cmdHandler_OpenStore, 9},
    {{"forceLogout"}, "Logout player", cmdHandler_ForceLogout, 9},
    {{"sendLocation"}, "Send Location Test", cmdHandler_SendLocations, 9},
    {{"developerConsoleOutput"}, "Send message to -console window", cmdHandler_SendConsoleOutput, 9},
    {{"clientConsoleOutput"}, "Send message to ingame (~) console", cmdHandler_SendConsolePrint, 9},
    {{"clearTarget"}, "Clear current target", cmdHandler_ClearTarget, 9},
    {{"startTimer"}, "Create a small timer", cmdHandler_StartTimer, 9},
    /* SlashCommand_Debug - For live value-testing */
    {{"setu1"},"Set bitvalue u1. Used for live-debugging.", cmdHandler_SetU1, 9},

    /* SlashCommand_Email - Access Level 0 */
    /* These are "behind the scenes" and sent by the client */
    {{"emailheaders"}, "Request Email Headers", cmdHandler_EmailHeaders, 0},
    {{"emailread"}, "Request Email Message with Given ID", cmdHandler_EmailRead, 0},
    {{"emailsend"}, "Send Email", cmdHandler_EmailSend, 0},
    {{"emaildelete"}, "Delete Email with Given ID",cmdHandler_EmailDelete, 0},

    /* SlashCommand_Friends - Access Level 9 */
    {{"friendsDump", "friendsDebug"}, "Output friendlist info to console", cmdHandler_FriendsListDebug, 9},
    /* SlashCommand_Friends - Access Level 1 */
    {{"friend"}, "Add friend to friendlist", cmdHandler_Friend, 1},
    {{"unfriend","estrange"}, "Remove friend from friendlist", cmdHandler_Unfriend, 1},
    {{"friendlist", "fl"}, "Toggle visibility of friendslist", cmdHandler_FriendList, 1},

    /* SlashCommand_Misc - Access Level 2 [GM] */
    {{"addNpc"},"add <npc_name> with costume [variation] in front of gm", cmdHandler_AddNPC, 2},
    /* SlashCommand_Misc - Access Level 1 */
    {{"whoall"},"Shows who is on the current map ", cmdHandler_WhoAll, 1},
    {{"motd"},"View the server MOTD", cmdHandler_MOTD, 1},
    {{"tailor"}, "Open Tailor Window", cmdHandler_Tailor, 1},
    {{"cc"}, "Costume Change", cmdHandler_CostumeChange, 1},
    {{"train"}, "Train Up Level", cmdHandler_Train, 1},
    {{"kiosk"}, "Event kiosk", cmdHandler_Kiosk, 1},

    /* SlashCommand_Movement - Access Level 9 */
    {{"ControlsDisabled"},"Disable controls", cmdHandler_ControlsDisabled, 9},
    {{"updateid"},"Update ID", cmdHandler_UpdateId, 9},
    {{"fullupdate"},"Full Update", cmdHandler_FullUpdate, 9},
    {{"hascontrolid"},"Force the server to acknowledge input ids", cmdHandler_HasControlId, 9},
    {{"interp"},"Toggle Interpolation", &cmdHandler_ToggleInterp, 9},
    {{"moveinstantly"},"Toggle move_instantly", &cmdHandler_ToggleMoveInstantly, 9},
    {{"collision"},"Toggle Collision on/off", &cmdHandler_ToggleCollision, 9},
    {{"movement"},"Toggle server authority for Movement on/off", &cmdHandler_ToggleMovementAuthority, 9},
    {{"face"}, "Face a target", cmdHandler_FaceEntity, 9},
    {{"faceLocation"}, "Face a location", cmdHandler_FaceLocation, 9},
    {{"mapmove", "movezone", "mz"}, "Move to a map id", cmdHandler_MoveZone, 9},
    {{"toggleInputLog"}, "Enable input logging for a player (e.g. /toggleInputLog playerName), if no player name is specified it uses the sender", cmdHandler_ToggleInputLog, 9},
    {{"toggleMovementLog"}, "Enable movement logging for a player (e.g. /toggleMovementLog playerName), if no player name is specified it uses the sender", cmdHandler_ToggleMovementLog, 9},
    /* SlashCommand_Movement - Access Level 2 */
    {{"moveTo", "setpos", "setpospyr"},"set the gm's position to <x> <y> <z>", cmdHandler_MoveTo, 2},
    {{"fly", "flying"},"Toggle flying On/Off", cmdHandler_Fly, 2},
    {{"jumppack"},"Toggle jump pack On/Off", cmdHandler_Jumppack, 2},
    {{"teleport", "tp"}, "Teleport", cmdHandler_Teleport, 2},
    /* SlashCommand_Movement - Access Level 1 */
    {{"stuck"},"Free yourself if your character gets stuck", cmdHandler_Stuck, 1},
    {{"gotospawn"},"Teleport to a SpawnLocation", cmdHandler_SetSpawnLocation, 1},
    {{"MapXferList", "mapmenu"}, "Show MapXferList", cmdHandler_MapXferList, 1},

    /* SlashCommand_Powers - Access Level 9 */
    {{"addpowerset"},"Adds entire PowerSet (by 'pcat pset' idxs) to Entity", &cmdHandler_AddEntirePowerSet, 9},
    {{"addpower"},"Adds Power (by 'pcat pset pow' idxs) to Entity", &cmdHandler_AddPower, 9},
    {{"addinsp"},"Adds Inspiration (by name) to Entity", &cmdHandler_AddInspiration, 9},
    {{"addboost", "addEnhancement"},"Adds Enhancement (by name) to Entity", &cmdHandler_AddEnhancement, 9},
    /* SlashCommand_Powers - Access Level 1 */
    {{"respec"}, "Start ReSpec", cmdHandler_ReSpec, 1},

    /* SlashCommand_Stats - Access Level 9 */
    {{"falling"},"Toggle falling On/Off", cmdHandler_Falling, 9},
    {{"sliding"},"Toggle sliding On/Off", cmdHandler_Sliding, 9},
    {{"jumping"},"Toggle jumping On/Off", cmdHandler_Jumping, 9},
    {{"stunned"},"Toggle stunned character On/Off", cmdHandler_Stunned, 9},
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
    /* SlashCommand_Stats - Access Level 2 [GM] */
    {{"align", "alignment", "herostatus"},"set the gm's alignment to hero, villain, both, none/neither", cmdHandler_Alignment, 2},
    /* SlashCommand_Stats - Access Level 1 */
    {{"afk"},"Mark yourself as Away From Keyboard", cmdHandler_AFK, 1},
    {{"setTitles","title"},"Open the Title selection window", cmdHandler_SetTitles, 1},
    {{"setCustomTitles"},"Set your titles manually", cmdHandler_SetCustomTitles, 1},
    {{"setSpecialTitle"},"Set your Special title", cmdHandler_SetSpecialTitle, 1},
    {{"assist"}, "Re-target based on the selected entity's current target", cmdHandler_SetAssistTarget, 1},

    /* SlashCommand_SuperGroup - Access Level 9 */
    {{"setSuperGroup","setSG"},"Set your Super Group", cmdHandler_SetSuperGroup, 9},

    /* SlashCommand_Teams - Access Level 9 */
    {{"setTeam", "setTeamID"},"Set the team idx", cmdHandler_SetTeam, 9},
    {{"teamDump", "teamDebug"}, "Output team settings to console", cmdHandler_TeamDebug, 9},
    /* SlashCommand_Teams - Access Level 1 */
    {{"i","invite"}, "Invite player to team", cmdHandler_Invite, 1},
    {{"k","kick"}, "Kick player from team", cmdHandler_Kick, 1},
    {{"leaveteam"}, "Leave the team you're currently on", cmdHandler_LeaveTeam, 1},
    {{"findmember"}, "Open Looking For Group Window", cmdHandler_FindMember, 1},
    {{"makeleader","ml"}, "Make another player team leader", cmdHandler_MakeLeader, 1},
    {{"sidekick","sk"}, "Invite player to sidekick", cmdHandler_Sidekick, 1},
    {{"unsidekick","unsk"}, "Remove sidekick/duo relationship", cmdHandler_UnSidekick, 1},
    {{"buffs"}, "Toggle buffs display on team window", cmdHandler_TeamBuffs, 1},
    {{"lfg"},"Toggle looking for group status ", cmdHandler_LFG, 1},
    /* SlashCommand_Teams - Access Level 0 */
    {{"team_accept"}, "Accept Team invite", cmdHandler_TeamAccept, 0},
    {{"team_decline"}, "Decline Team invite", cmdHandler_TeamDecline, 0},
    {{"sidekick_accept"}, "Accept Sidekick invite", cmdHandler_SidekickAccept, 0},
    {{"sidekick_decline"}, "Decline Sidekick invite", cmdHandler_SidekickDecline, 0},

    /* SlashCommand_Trade - Access Level 1 */
    {{"trade"}, "Trade with player", cmdHandler_Trade, 1},
    /* SlashCommand_Trade - Access Level 0 */
    {{"trade_accept"}, "Accept Trade invite", cmdHandler_TradeAccept, 0},
    {{"trade_decline"}, "Decline Trade invite", cmdHandler_TradeDecline, 0},
};


/************************************************************
 *  Helper Functions
 ***********************************************************/
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


/************************************************************
 *  Slash Command Handlers
 ***********************************************************/
void cmdHandler_CmdList(const QStringList &params, MapClientSession &sess)
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
    qCDebug(logSlashCommand).noquote() << params.join(" ") << ":\n" << msg;
}

} // end of anonymous namespace


/************************************************************
 *  runCommand for executing commands on MapClientSession
 ***********************************************************/
void runCommand(const QString &str, MapClientSession &sess)
{
    // Split args on spaces (but leave quote-enclosed spaces)
    QStringList args = str.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?"));
    // Regex always produces an empty match for $
    args.pop_back();
    // May also produce an extra empty match if input ends in quote
    if (args.back().isEmpty())
        args.pop_back();

    QString command_name = args.takeFirst();

    for (const auto &cmd : g_defined_slash_commands)
    {
        if(cmd.m_valid_prefixes.contains(command_name, Qt::CaseInsensitive))
        {
            if(!canAccessCommand(cmd, sess))
                return; // no access, so return early
            cmd.m_handler(args, sess);
            return; // return here to avoid unknown command msg
        }
    }
    qCDebug(logSlashCommand) << "Unknown game command:" << str;
}

//! @}
