#include "SlashCommand.h"
#include "DataHelpers.h"
#include "MapInstance.h"
#include "Settings.h"
#include "LFG.h"
#include "Logging.h"

#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QRegularExpression>
#include <QtCore/QDebug>

InfoMessageCmd *info; // leverage InfoMessageCmd

std::vector<SlashCommand> g_defined_slash_commands = {
    /* Access Level 9 Commands */
    {{"Script"},"Run a Script", &cmdHandler_Script, 9},
    {{"Dialog", "dlg"},"Open a dialog box with any string arg", &cmdHandler_Dialog, 9},
    {{"InfoMessage", "imsg"},"Send an info message into chat. Expects <int> <string>, e.g. /imsg 1 test", &cmdHandler_InfoMessage, 9},
    {{"SmileX"},"Runs the smlx script SmileX", &cmdHandler_SmileX, 9},
    {{"fly", "flying"},"Toggle flying On/Off", &cmdHandler_Fly, 9},
    {{"falling"},"Toggle falling On/Off", &cmdHandler_Falling, 9},
    {{"sliding"},"Toggle sliding On/Off", &cmdHandler_Sliding, 9},
    {{"jumping"},"Toggle jumping On/Off", &cmdHandler_Jumping, 9},
    {{"stunned"},"Toggle stunned character On/Off", &cmdHandler_Stunned, 9},
    {{"jumppack"},"Toggle jump pack On/Off", &cmdHandler_Jumppack, 9},
    {{"setSpeed", "speed"},"Set your player Speed", &cmdHandler_SetSpeed, 9},
    {{"setBackupSpd", "BackupSpd"},"Set the Backup Speed", &cmdHandler_SetBackupSpd, 9},
    {{"setJumpHeight", "JumpHeight"},"Set the Jump Height", &cmdHandler_SetJumpHeight, 9},
    {{"setHP"},"Set the HP value of your character", &cmdHandler_SetHP, 9},
    {{"setEnd"},"Set your Endurance", &cmdHandler_SetEnd, 9},
    {{"setXP"},"Set your XP", &cmdHandler_SetXP, 9},
    {{"setDebt"},"Set your Debt", &cmdHandler_SetDebt, 9},
    {{"setInf"},"Set your Influence", &cmdHandler_SetInf, 9},
    {{"setLevel"},"Set your Level", &cmdHandler_SetLevel, 9},
    {{"setCombatLevel"},"Set your Combat Level", &cmdHandler_SetCombatLevel, 9},
    {{"UpdateChar", "CharUpdate", "save"},"Update character information in the database", &cmdHandler_UpdateChar, 9},
    {{"DebugChar", "chardebug"},"View your characters debug information", &cmdHandler_DebugChar, 9},
    {{"ControlsDisabled"},"Disable controls", &cmdHandler_ControlsDisabled, 9},
    {{"updateid"},"Update ID", &cmdHandler_UpdateId, 9},
    {{"fullupdate"},"Full Update", &cmdHandler_FullUpdate, 9},
    {{"hascontrolid"},"Force the server to acknowledge input ids", &cmdHandler_HasControlId, 9},
    {{"setTeam", "setTeamID"},"Set the team idx", &cmdHandler_SetTeam, 9},
    {{"setSuperGroup","setSG"},"Set your Super Group", &cmdHandler_SetSuperGroup, 9},
    {{"settingsDump","settingsDebug"},"Output settings.cfg to console", &cmdHandler_SettingsDump, 9},
    {{"teamDump", "teamDebug"}, "Output team settings to console", &cmdHandler_TeamDebug, 9},
    {{"guiDump", "guiDebug"}, "Output gui settings to console", &cmdHandler_GUIDebug, 9},
    {{"setWindowVisibility", "setWinVis"}, "", &cmdHandler_SetWindowVisibility, 9},
    {{"keybindDump", "keybindDebug"}, "Output keybind settings to console", &cmdHandler_KeybindDebug, 9},
    {{"toggleLogging", "log"}, "Modify log categories (e.g. input, teams, ...)", &cmdHandler_ToggleLogging, 9},
    {{"friendsDump", "friendsDebug"}, "Output friendlist info to console", &cmdHandler_FriendsListDebug, 9},
    {{"damage", "heal"}, "Make current target (or self) take damage/health", &cmdHandler_SendFloatingNumbers, 9},
    {{"setu1"},"Set bitvalue u1", &cmdHandler_SetU1, 9},
    {{"setu2"},"Set bitvalue u2", &cmdHandler_SetU2, 9},
    {{"setu3"},"Set bitvalue u3", &cmdHandler_SetU3, 9},
    {{"setu4"},"Set bitvalue u4", &cmdHandler_SetU4, 9},
    {{"setu5"},"Set bitvalue u5", &cmdHandler_SetU5, 9},
    {{"setu6"},"Set bitvalue u6", &cmdHandler_SetU6, 9},
    /* Access Level 1 Commands */
    {{"cmdlist","commandlist"},"List all accessible commands", &cmdHandler_CmdList, 1},
    {{"afk"},"Mark yourself as Away From Keyboard", &cmdHandler_AFK, 1},
    {{"whoall"},"Shows who is on the current map ", &cmdHandler_WhoAll, 1},
    {{"setTitles"},"Set your title", &cmdHandler_SetTitles, 1},
    {{"stuck"},"Free yourself if your character gets stuck", &cmdHandler_Stuck, 1},
    {{"lfg"},"Toggle looking for group status ", &cmdHandler_LFG, 1},
    {{"motd"},"View the server MOTD", &cmdHandler_MOTD, 1},
    {{"i","invite"}, "Invite player to team", &cmdHandler_Invite, 1},
    {{"k","kick"}, "Kick player from team", &cmdHandler_Kick, 1},
    {{"leaveteam"}, "Leave the team you're currently on", &cmdHandler_LeaveTeam, 1},
    {{"findmember"}, "Open Looking For Group Window", &cmdHandler_FindMember, 1},
    {{"makeleader","ml"}, "Make another player team leader", &cmdHandler_MakeLeader, 1},
    {{"assist"}, "Re-target based on the selected entity's current target", &cmdHandler_SetAssistTarget, 1},
    {{"sidekick","sk"}, "Invite player to sidekick", &cmdHandler_Sidekick, 1},
    {{"unsidekick","unsk"}, "Remove sidekick/duo relationship", &cmdHandler_UnSidekick, 1},
    {{"buffs"}, "Toggle buffs display on team window", &cmdHandler_TeamBuffs, 1},
    {{"friend"}, "Add friend to friendlist", &cmdHandler_Friend, 1},
    {{"unfriend","estrange"}, "Remove friend from friendlist", &cmdHandler_Unfriend, 1},
    {{"friendlist", "fl"}, "Toggle visibility of friendslist", &cmdHandler_FriendList, 1},
    /* Access Level 0 Commands :: These are "behind the scenes" and sent by the client */
    {{"team_accept"}, "Accept Team invite", &cmdHandler_TeamAccept, 0},
    {{"team_decline"}, "Decline Team invite", &cmdHandler_TeamDecline, 0},
    {{"sidekick_accept"}, "Accept Sidekick invite", &cmdHandler_SidekickAccept, 0},
    {{"sidekick_decline"}, "Decline Sidekick invite", &cmdHandler_SidekickDecline, 0}
};

bool canAccessCommand(const SlashCommand &cmd, const Entity &e)
{
    MapClient *src = e.m_client;
    int alvl = getAccessLevel(e);
    if(alvl >= cmd.m_required_access_level)
        return true;

    QString msg = "You do not have adequate permissions to use the command: " + cmd.m_valid_prefixes.first();
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::USER_ERROR, msg, src);
    return false;
}

void runCommand(QString &str, Entity &e) {
    for(const auto &cmd : g_defined_slash_commands)
    {
      if(cmd.m_valid_prefixes.contains(str.split(' ').front(),Qt::CaseInsensitive))
      {
          if(!canAccessCommand(cmd, e))
              return;   // no access, so return early
          cmd.m_handler(str,&e);
          return;       // return here to avoid unknown command msg
      }
    }
    qCDebug(logSlashCommand) << "Unknown game command:" << str;
}

/************************************************************
 *  Slash Command Handlers
 ***********************************************************/

// Access Level 9 Commands (GMs)
void cmdHandler_Script(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    MapInstance *mi = src->current_map();

    QString code = cmd.mid(7,cmd.size()-7);
    mi->m_scripting_interface->runScript(src,code,"user provided script");
}

void cmdHandler_Dialog(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    StandardDialogCmd *dlg = new StandardDialogCmd(cmd.mid(4));
    src->addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
}

void cmdHandler_InfoMessage(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
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
    sendInfoMessage(static_cast<MessageChannel>(cmdType), msg, src);
}

void cmdHandler_SmileX(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    int space = cmd.indexOf(' ');
    QString fileName("scripts/" + cmd.mid(space+1));
    if(!fileName.endsWith(".smlx"))
            fileName.append(".smlx");
    QFile file(fileName);
    if(file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString contents(file.readAll());
        StandardDialogCmd *dlg = new StandardDialogCmd(contents);
        src->addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
    }
    else {
        QString errormsg = "Failed to load smilex file. \'" + file.fileName() + "\' not found.";
        qCDebug(logSlashCommand) << errormsg;
        sendInfoMessage(MessageChannel::ADMIN, errormsg, src);
    }
}

void cmdHandler_Fly(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleFly(*e);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_Falling(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleFalling(*e);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_Sliding(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleSliding(*e);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_Jumping(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleJumping(*e);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_Stunned(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleStunned(*e);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_Jumppack(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleJumppack(*e);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetSpeed(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    QVector<QStringRef> args(cmd.splitRef(' '));
    float v1 = args.value(1).toFloat();
    float v2 = args.value(2).toFloat();
    float v3 = args.value(3).toFloat();
    setSpeed(*e, v1, v2, v3);

    QString msg = QString("Set Speed to: <%1,%2,%3>").arg(v1).arg(v2).arg(v3);
  
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetBackupSpd(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float val = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    setBackupSpd(*e, val);

    QString msg = "Set BackupSpd to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetJumpHeight(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float val = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    setJumpHeight(*e, val);

    QString msg = "Set JumpHeight to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetHP(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float attrib = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    float maxattrib = e->m_char->m_max_attribs.m_HitPoints;

    if(attrib > maxattrib)
        attrib = maxattrib;

    setHP(*e->m_char,attrib);

    QString msg = QString("Setting HP to: %1 / %2").arg(attrib).arg(maxattrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetEnd(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float attrib = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    float maxattrib = e->m_char->m_max_attribs.m_Endurance;

    if(attrib > maxattrib)
        attrib = maxattrib;

    setEnd(*e->m_char,attrib);

    QString msg = QString("Setting Endurance to: %1 / %2").arg(attrib).arg(maxattrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetXP(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();
    uint32_t lvl = getLevel(*e->m_char);

    setXP(*e->m_char, attrib);
    QString msg = "Setting XP to " + QString::number(attrib);

    uint32_t newlvl = getLevel(*e->m_char);
    if(lvl != newlvl)
        msg += " and LVL to " + QString::number(newlvl);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetDebt(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    float attrib = cmd.midRef(cmd.indexOf(' ')+1).toFloat();

    setDebt(*e->m_char, attrib);
    QString msg = QString("Setting XP Debt to %1").arg(attrib);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetInf(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setInf(*e->m_char, attrib);

    QString msg = "Setting influence to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetLevel(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setLevel(*e->m_char, attrib); // TODO: Why does this result in -1?

    QString msg = "Setting Level to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetCombatLevel(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setCombatLevel(*e->m_char, attrib); // TODO: Why does this result in -1?

    QString msg = "Setting Combat Level to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_UpdateChar(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    charUpdateDB(e);

    QString msg = "Updating Character in Database: " + e->name();
    qCDebug(logSlashCommand) << cmd << ":" << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_DebugChar(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    const Character &chardata(*e->m_char);
    QString msg = "DebugChar: " + e->name()
            + "\n  " + chardata.m_char_data.m_origin_name
            + "\n  " + chardata.m_char_data.m_class_name
            + "\n  map: " + chardata.m_char_data.m_mapName
            + "\n  db_id: " + QString::number(e->m_db_id) + ":" + QString::number(chardata.m_db_id)
            + "\n  idx: " + QString::number(e->m_idx)
            + "\n  access: " + QString::number(e->m_entity_data.m_access_level)
            + "\n  acct: " + QString::number(chardata.m_account_id)
            + "\n  lvl/clvl: " + QString::number(chardata.m_char_data.m_level) + "/" + QString::number(chardata.m_char_data.m_combat_level)
            + "\n  inf: " + QString::number(chardata.m_char_data.m_influence)
            + "\n  xp/debt: " + QString::number(chardata.m_char_data.m_experience_points) + "/" + QString::number(chardata.m_char_data.m_experience_debt)
            + "\n  lfg: " + QString::number(chardata.m_char_data.m_lfg)
            + "\n  afk: " + QString::number(chardata.m_char_data.m_afk)
            + "\n  tgt_idx: " + QString::number(getTargetIdx(*e));
    e->dump();
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_ControlsDisabled(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleControlsDisabled(*e);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_UpdateId(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint8_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setUpdateID(*e, attrib);

    QString msg = "Setting updateID to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_FullUpdate(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleFullUpdate(*e);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_HasControlId(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleControlId(*e);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetTeam(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setTeamID(*e, val);

    QString msg = "Set Team ID to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetSuperGroup(QString &cmd, Entity *e) {
    MapClient *src  = e->m_client;
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    int sg_id       = args.value(1).toInt();
    QString sg_name = args.value(2);
    int sg_rank     = args.value(3).toInt();

    setSuperGroup(*e, sg_id, sg_name, sg_rank);

    QString msg = QString("Set SuperGroup:  id: %1  name: %2  rank: %3").arg(QString::number(sg_id), sg_name, QString::number(sg_rank));
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SettingsDump(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    QString msg = "Sending settings config dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);

    settingsDump(); // Send settings dump
}

void cmdHandler_TeamDebug(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    QString msg = "Sending team debug to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);

    e->m_team->dump(); // Send team debug info
}

void cmdHandler_GUIDebug(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    QString msg = "Sending GUISettings dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);

    e->m_char->m_gui.guiDump(); // Send GUISettings dump
}

void cmdHandler_SetWindowVisibility(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    uint32_t idx = args.value(1).toInt();
    WindowVisibility val = (WindowVisibility)args.value(2).toInt();

    QString msg = "Toggling " + QString::number(idx) +  " GUIWindow visibility: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);

    e->m_char->m_gui.m_wnds.at(idx).setWindowVisibility(val); // Set WindowVisibility
    e->m_char->m_gui.m_wnds.at(idx).guiWindowDump(); // for debugging
}

void cmdHandler_KeybindDebug(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    QString msg = "Sending Keybinds dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);

    e->m_char->m_keybinds.keybindsDump(); // Send GUISettings dump
}

void cmdHandler_ToggleLogging(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry
    args.removeFirst();

    QString msg = "Toggle logging of categories: " + args.join(" ");
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);

    for (auto category : args)
        toggleLogging(category); // Toggle each category listed
}

void cmdHandler_FriendsListDebug(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    QString msg = "Sending FriendsList dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);

    dumpFriends(*e); // Send FriendsList dump
}

void cmdHandler_SendFloatingNumbers(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
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
        sendInfoMessage(MessageChannel::USER_ERROR, msg, src);
        return;
    }       

    tgt = getEntity(src,name); // get Entity by name

    if(tgt == nullptr)
    {
        msg = "FloatingNumbers target " + name + " cannot be found.";
        qCDebug(logSlashCommand) << msg;
        sendInfoMessage(MessageChannel::USER_ERROR, msg, src);
        return;
    }

    for(int i = 0; i<runtimes; i++)
    {
        sendFloatingNumbers(e, tgt->m_idx, amount);

        setHP(*tgt->m_char, getHP(*tgt->m_char)-amount); // deal dmg

        if(amount >= 0) // damage
        {
            msg = QString("%1 deals %2 points of damage to %3.").arg(e->name(), QString::number(amount), name);
            qCDebug(logSlashCommand) << msg;

            msg = QString("You deal %1 points of damage to %2.").arg(QString::number(amount), name);
            sendInfoMessage(MessageChannel::DAMAGE, msg, src);
            msg = QString("%1 has dealt you %2 points of damage!").arg(e->name(), QString::number(amount));
            sendInfoMessage(MessageChannel::DAMAGE, msg, tgt->m_client);
        }
        else
        {
            msg = QString("%1 heals %2 points of damage from %3.").arg(e->name(), QString::number(-amount), name);
            qCDebug(logSlashCommand) << msg;

            msg = QString("You heal %1 points of damage from %2.").arg(QString::number(-amount), name);
            sendInfoMessage(MessageChannel::TEAM, msg, src); // TEAM for green
            msg = QString("%1 has healed %2 points of damage from you!").arg(e->name(), QString::number(-amount));
            sendInfoMessage(MessageChannel::TEAM, msg, tgt->m_client); // TEAM for green
        }
    }
}

// Slash commands for setting bit values
void cmdHandler_SetU1(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu1(*e, val);

    QString msg = "Set u1 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU2(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu2(*e, val);

    QString msg = "Set u2 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU3(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu3(*e, val);

    QString msg = "Set u3 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU4(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu4(*e, val);

    QString msg = "Set u4 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU5(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu5(*e, val);

    QString msg = "Set u5 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU6(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu6(*e, val);

    QString msg = "Set u6 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

// Access Level 1 Commands
void cmdHandler_CmdList(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    QString msg = "Below is a list of all slash commands that your account can access. They are not case sensitive.\n";
    QString msg_dlg = "<face heading><span align=center><color #ff0000>Command List</color></span></face><br>\n<br>\n";

    for(const auto &sc : g_defined_slash_commands)
    {
        int alvl = getAccessLevel(*e);
        if(alvl >= sc.m_required_access_level)
        {
            // We don't want to show acess_level 0 (Under the Hood commands)
            if(sc.m_required_access_level != 0 )
            {
                // Use msg for std out, msg_dlg for ingame dialog box
                msg += "\t" + sc.m_valid_prefixes.join(", ") + " [" + QString::number(sc.m_required_access_level) + "]:\t" + sc.m_help_text + "\n";
                msg_dlg += QString("<color #ffCC99><i>%1</i></color>[<color #66ffff>%2</color>]: %3<br>").arg(sc.m_valid_prefixes.join(", ")).arg(sc.m_required_access_level).arg(sc.m_help_text);
            }
        }
    }

    // Dialog output
    StandardDialogCmd *dlg = new StandardDialogCmd(msg_dlg);
    src->addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
    // CMD line (debug) output
    qCDebug(logSlashCommand).noquote() << cmd << ":\n" << msg;
}

void cmdHandler_AFK(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    int space = cmd.indexOf(' ');
    QString val = cmd.mid(space+1);
    toggleAFK(*e->m_char, val);

    QString msg = "Setting afk message to: " + val;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::EMOTE, msg, src);
}

void cmdHandler_WhoAll(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    MapInstance *mi = src->current_map();

    QString msg = "Players on this map:\n";

    for(MapClient *cl : mi->m_clients)
    {
        Character &c(*cl->char_entity()->m_char);
        QString name        = cl->char_entity()->name();
        QString lvl         = QString::number(getLevel(c));
        QString clvl        = QString::number(getCombatLevel(c));
        QString origin      = getOrigin(c);
        QString archetype   = QString(getClass(c)).remove("Class_");

        // Format: character_name "lvl" level "clvl" combat_level origin archetype
        msg += name + " lvl " + lvl + " clvl " + clvl + " " + origin + " " + archetype + "\n";
    }

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, src);
}

void cmdHandler_SetTitles(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    bool prefix;
    QString msg, generic, origin, special;
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    if(cmd.toLower() == "settitles")
    {
        setTitles(*e->m_char);
        msg = "Titles reset to nothing";
    }
    else
    {
        prefix  = !args.value(1).isEmpty();
        generic = args.value(2);
        origin  = args.value(3);
        special = args.value(4);
        setTitles(*e->m_char, prefix, generic, origin, special);
        msg = "Titles changed to: " + QString::number(prefix) + " " + generic + " " + origin + " " + special;
    }
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::USER_ERROR, msg, src);
}

void cmdHandler_Stuck(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    // TODO: Implement true move-to-safe-location-nearby logic
    e->m_entity_data.m_pos = glm::vec3(128.0f,16.0f,-198.0f); // Atlas Park starting location

    QString msg = QString("Resetting location to default spawn (%1,%2,%3)").arg(e->m_entity_data.m_pos.x, e->m_entity_data.m_pos.y, e->m_entity_data.m_pos.z);
    qCDebug(logSlashCommand) << cmd << ":" << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, src);
}

void cmdHandler_LFG(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleLFG(*e);

    QString msg = "Toggling " + cmd;
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, src);
}

void cmdHandler_MOTD(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    sendServerMOTD(e);

    QString msg = "Opening Server MOTD";
    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, src);
}

void cmdHandler_Invite(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    Entity *tgt = nullptr;
    QString msg;

    int space = cmd.indexOf(' ');
    QString name = cmd.mid(space+1);

    if(space == -1 || name.isEmpty())
    {
        tgt = getEntity(src,getTargetIdx(*e));
        name = tgt->name();
    }
    else
        tgt = getEntity(src,name);

    if(tgt == nullptr)
        return;

    if(tgt->m_has_team)
    {
        msg = tgt->name() + " is already on a team.";
        qCDebug(logTeams) << msg;
        sendInfoMessage(MessageChannel::SERVER, msg, src);
        return;
    }

    if(tgt->name() == src->name())
    {
        msg = "You cannot invite yourself to a team.";
        qCDebug(logTeams) << msg;
        sendInfoMessage(MessageChannel::SERVER, msg, src);
        return;
    }

    if(e->m_has_team && e->m_team != nullptr)
    {
        if(!e->m_team->isTeamLeader(e))
        {
            msg = "Only the team leader can invite players to the team.";
            qCDebug(logTeams) << e->name() << msg;
            sendInfoMessage(MessageChannel::TEAM, msg, src);
            return;
        }
    }

    sendTeamOffer(e,tgt);
}

void cmdHandler_TeamAccept(QString &cmd, Entity *e) {
    // game command: "team_accept \"From\" to_db_id to_db_id \"To\""
    MapClient *src = e->m_client;

    QString msgfrom = "Something went wrong with TeamAccept.";
    QString msgtgt = "Something went wrong with TeamAccept.";
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    QString from_name       = args.value(1);
    uint32_t tgt_db_id      = args.value(2).toInt();
    uint32_t tgt_db_id_2    = args.value(3).toInt(); // always the same?
    QString tgt_name        = args.value(4);

    if(tgt_db_id != tgt_db_id_2)
        qWarning() << "TeamAccept db_ids do not match!";

    Entity *from_ent = getEntity(src,from_name);
    if(from_ent == nullptr)
        return;

    if(inviteTeam(*from_ent,*e))
    {
        msgfrom = "Inviting " + tgt_name + " to team.";
        msgtgt = "Joining " + from_name + "'s team.";

    }
    else
    {
        msgfrom = "Failed to invite " + tgt_name + ". They are already on a team.";
    }

    qCDebug(logSlashCommand).noquote() << msgfrom;
    sendInfoMessage(MessageChannel::TEAM, msgfrom, from_ent->m_client);
    sendInfoMessage(MessageChannel::TEAM, msgtgt, src);
}

void cmdHandler_TeamDecline(QString &cmd, Entity *e) {
    // game command: "team_decline \"From\" to_db_id \"To\""
    MapClient *src = e->m_client;

    QString msg;
    QStringList args;
    args = cmd.split(QRegularExpression("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    QString from_name   = args.value(1);
    uint32_t tgt_db_id  = args.value(2).toInt();
    QString tgt_name    = args.value(3);

    Entity *from_ent = getEntity(src,from_name);
    if(from_ent == nullptr)
        return;

    msg = tgt_name + " declined a team invite from " + from_name + QString::number(tgt_db_id);
    qCDebug(logSlashCommand).noquote() << msg;

    msg = tgt_name + " declined your team invite."; // to sender
    sendInfoMessage(MessageChannel::TEAM, msg, from_ent->m_client);
    msg = "You declined the team invite from " + from_name; // to target
    sendInfoMessage(MessageChannel::TEAM, msg, src);
}

void cmdHandler_Kick(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    Entity *tgt = nullptr;
    QString msg;

    int space = cmd.indexOf(' ');
    QString name = cmd.mid(space+1);

    if(space == -1 || name.isEmpty())
    {
        tgt = getEntity(src,getTargetIdx(*e));
        name = tgt->name();
    }
    else
        tgt = getEntity(src,name);

    if(tgt == nullptr)
        return;

    if(kickTeam(*tgt))
        msg = "Kicking " + name + " from team.";
    else
        msg = "Failed to kick " + name;

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::TEAM, msg, src);
}

void cmdHandler_LeaveTeam(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    leaveTeam(*e);

    QString msg = "Leaving Team";
    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::TEAM, msg, src);
}

void cmdHandler_FindMember(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    findTeamMember(*e);

    QString msg = "Finding Team Member";
    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::CHAT_TEXT, msg, src);
}

void cmdHandler_MakeLeader(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    Entity *tgt = nullptr;
    QString msg;

    int space = cmd.indexOf(' ');
    QString name = cmd.mid(space+1);

    if(space == -1 || name.isEmpty())
    {
        tgt = getEntity(src,getTargetIdx(*e));
        name = tgt->name();
    }
    else
        tgt = getEntity(src,name);

    if(tgt == nullptr)
        return;

    if(makeTeamLeader(*e,*tgt))
        msg = "Making " + name + " team leader.";
    else
        msg = "Failed to make " + name + " team leader.";

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::TEAM, msg, src);
}

void cmdHandler_SetAssistTarget(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    QString msg = "Setting assist target.";

    setAssistTarget(*e);

    qCDebug(logSlashCommand).noquote() << msg;
    sendInfoMessage(MessageChannel::USER_ERROR, msg, src);
}

void cmdHandler_Sidekick(QString &cmd, Entity *e)
{
    MapClient *src = e->m_client;
    Entity *tgt = nullptr;

    int space = cmd.indexOf(' ');
    QString name = cmd.mid(space+1);

    if(space == -1 || name.isEmpty())
    {
        tgt = getEntity(src,getTargetIdx(*e));
        name = tgt->name();
    }
    else
        tgt = getEntity(src,name);

    if(tgt == nullptr || e->m_char->isEmpty() || tgt->m_char->isEmpty())
        return;

    inviteSidekick(*e,*tgt);
}

void cmdHandler_SidekickAccept(QString &cmd, Entity *e)
{
    MapClient *src  = e->m_client;
    uint32_t db_id  = e->m_char->m_char_data.m_sidekick.m_db_id;
    Entity *tgt     = getEntityByDBID(src,db_id);

    if(tgt == nullptr || e->m_char->isEmpty() || tgt->m_char->isEmpty())
        return;

    addSidekick(*e,*tgt);
}

void cmdHandler_SidekickDecline(QString &cmd, Entity *e)
{
    e->m_char->m_char_data.m_sidekick.m_db_id = 0;
}

void cmdHandler_UnSidekick(QString &cmd, Entity *e)
{
    MapClient *src = e->m_client;

    if(e->m_char->isEmpty())
        return;

    removeSidekick(*e);
}

void cmdHandler_TeamBuffs(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    toggleTeamBuffs(*e->m_char);

    QString msg = "Toggling Team Buffs display mode.";
    qCDebug(logSlashCommand).noquote() << msg;
}

void cmdHandler_Friend(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    Entity *tgt = nullptr;

    int space = cmd.indexOf(' ');
    QString name = cmd.mid(space+1);

    if(space == -1 || name.isEmpty())
    {
        tgt = getEntity(src,getTargetIdx(*e));
        name = tgt->name();
    }
    else
        tgt = getEntity(src,name);

    if(tgt == nullptr || e->m_char->isEmpty() || tgt->m_char->isEmpty())
        return;

    addFriend(*e,*tgt);
}

void cmdHandler_Unfriend(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    Entity *tgt = nullptr;

    int space = cmd.indexOf(' ');
    QString name = cmd.mid(space+1);

    if(space == -1 || name.isEmpty())
    {
        tgt = getEntity(src,getTargetIdx(*e));
        name = tgt->name();
    }
    else
        tgt = getEntity(src,name);

    if(tgt == nullptr || e->m_char->isEmpty() || tgt->m_char->isEmpty())
        return;

    // TODO: Implement getCharacterFromDB(name) if target is not online.

    removeFriend(*e,*tgt);
}

void cmdHandler_FriendList(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    if(e->m_char->isEmpty())
        return;

    toggleFriendList(*e);
}
