#include "SlashCommand.h"
#include "DataHelpers.h"
#include "MapInstance.h"
#include "Settings.h"
#include "Logging.h"

#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>

InfoMessageCmd *info; // leverage InfoMessageCmd

std::vector<SlashCommand> g_defined_slash_commands = {
    /* Access Level 9 Commands */
    {{"Script"}, &cmdHandler_Script, 9},
    {{"Dialog", "dlg"}, &cmdHandler_Dialog, 9},
    {{"InfoMessage", "imsg"}, &cmdHandler_InfoMessage, 9},
    {{"SmileX"}, &cmdHandler_SmileX, 9},
    {{"fly", "flying"}, &cmdHandler_Fly, 9},
    {{"falling"}, &cmdHandler_Falling, 9},
    {{"sliding"}, &cmdHandler_Sliding, 9},
    {{"jumping"}, &cmdHandler_Jumping, 9},
    {{"stunned"}, &cmdHandler_Stunned, 9},
    {{"jumppack"}, &cmdHandler_Jumppack, 9},
    {{"setSpeed", "speed"}, &cmdHandler_SetSpeed, 9},
    {{"setBackupSpd", "BackupSpd"}, &cmdHandler_SetBackupSpd, 9},
    {{"setJumpHeight", "JumpHeight"}, &cmdHandler_SetJumpHeight, 9},
    {{"setHP"}, &cmdHandler_SetHP, 9},
    {{"setEnd"}, &cmdHandler_SetEnd, 9},
    {{"setXP"}, &cmdHandler_SetXP, 9},
    {{"setDebt"}, &cmdHandler_SetDebt, 9},
    {{"setInf"}, &cmdHandler_SetInf, 9},
    {{"setLevel"}, &cmdHandler_SetLevel, 9},
    {{"setCombatLevel"}, &cmdHandler_SetCombatLevel, 9},
    {{"UpdateChar", "CharUpdate", "save"}, &cmdHandler_UpdateChar, 9},
    {{"DebugChar", "chardebug"}, &cmdHandler_DebugChar, 9},
    {{"ControlsDisabled"}, &cmdHandler_ControlsDisabled, 9},
    {{"updateid"}, &cmdHandler_UpdateId, 9},
    {{"fullupdate"}, &cmdHandler_FullUpdate, 9},
    {{"hascontrolid"}, &cmdHandler_HasControlId, 9},
    {{"setTeam", "setTeamID"}, &cmdHandler_SetTeam, 9},
    {{"setSuperGroup","setSG"}, &cmdHandler_SetSuperGroup, 9},
    {{"settingsDump", "settingsDebug"}, &cmdHandler_SettingsDump, 9},
    {{"teamDump", "teamDebug"}, &cmdHandler_TeamDebug, 9},
    {{"guiDump", "guiDebug"}, &cmdHandler_GUIDebug, 9},
    {{"setWindowVisibility", "setWinVis"}, &cmdHandler_SetWindowVisibility, 9},
    {{"keybindDump", "keybindDebug"}, &cmdHandler_KeybindDebug, 9},
    {{"toggleLogging", "log"}, &cmdHandler_ToggleLogging, 9},
    {{"setu1"}, &cmdHandler_SetU1, 9},
    {{"setu2"}, &cmdHandler_SetU2, 9},
    {{"setu3"}, &cmdHandler_SetU3, 9},
    {{"setu4"}, &cmdHandler_SetU4, 9},
    {{"setu5"}, &cmdHandler_SetU5, 9},
    {{"setu6"}, &cmdHandler_SetU6, 9},
    /* Access Level 1 Commands */
    {{"help","listcommands"}, &cmdHandler_Help, 1},
    {{"afk"}, &cmdHandler_AFK, 1},
    {{"whoall"}, &cmdHandler_WhoAll, 1},
    {{"setTitles"}, &cmdHandler_SetTitles, 1},
    {{"stuck"}, &cmdHandler_Stuck, 1},
    {{"lfg"}, &cmdHandler_LFG, 1},
    {{"motd"}, &cmdHandler_MOTD, 1},
    {{"i","invite"}, &cmdHandler_Invite, 1},
    {{"k","kick"}, &cmdHandler_Kick, 1},
    {{"leaveteam"}, &cmdHandler_LeaveTeam, 1},
    {{"findmember"}, &cmdHandler_FindMember, 1},
    {{"makeleader","ml"}, &cmdHandler_MakeLeader, 1},
    {{"assist"}, &cmdHandler_SetAssistTarget, 1},
    {{"sidekick","sk"}, &cmdHandler_Sidekick, 1},
    {{"unsidekick","unsk"}, &cmdHandler_UnSidekick, 1},
    {{"buffs"}, &cmdHandler_TeamBuffs, 1}
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

    QStringList args;
    args = cmd.split(" ");
    float v1 = args.value(1).toFloat();
    float v2 = args.value(2).toFloat();
    float v3 = args.value(3).toFloat();
    setSpeed(*e, v1, v2, v3);

    args.removeAt(0); // remove command string
    QString msg = "Set Speed to: " + args.join(" ");
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetBackupSpd(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float val = cmd.split(" ").value(1).toFloat();
    setBackupSpd(*e, val);

    QString msg = "Set BackupSpd to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetJumpHeight(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float val = cmd.split(" ").value(1).toFloat();
    setJumpHeight(*e, val);

    QString msg = "Set JumpHeight to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetHP(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float attrib = cmd.split(" ").value(1).toFloat();
    float maxattrib = e->m_char.m_max_attribs.m_HitPoints;

    if(attrib > maxattrib)
        attrib = maxattrib;

    setHP(e->m_char,attrib);

    QString msg = "Setting HP to: " + QString::number(attrib) + "/" + QString::number(maxattrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetEnd(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float attrib = cmd.split(" ").value(1).toFloat();
    float maxattrib = e->m_char.m_max_attribs.m_Endurance;

    if(attrib > maxattrib)
        attrib = maxattrib;

    setEnd(e->m_char,attrib);

    QString msg = "Setting Endurance to: " + QString::number(attrib) + "/" + QString::number(maxattrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetXP(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    uint32_t attrib = cmd.split(" ").value(1).toUInt();
    uint32_t lvl = getLevel(e->m_char);

    setXP(e->m_char, attrib);
    QString msg = "Setting XP to " + QString::number(attrib);

    uint32_t newlvl = getLevel(e->m_char);
    if(lvl != newlvl)
        msg += " and LVL to " + QString::number(newlvl);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetDebt(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    float attrib = cmd.split(" ").value(1).toFloat();

    setDebt(e->m_char, attrib);
    QString msg = "Setting XP Debt to " + QString::number(attrib);

    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetInf(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint32_t attrib = cmd.split(" ").value(1).toUInt();

    setInf(e->m_char, attrib);

    QString msg = "Setting influence to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetLevel(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint32_t attrib = cmd.split(" ").value(1).toUInt();

    setLevel(e->m_char, attrib); // TODO: Why does this result in -1?

    QString msg = "Setting Level to: " + QString::number(attrib);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetCombatLevel(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint32_t attrib = cmd.split(" ").value(1).toUInt();

    setCombatLevel(e->m_char, attrib); // TODO: Why does this result in -1?

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

    QString msg = "DebugChar: " + e->name()
            + "\n  " + e->m_char.m_char_data.m_origin_name
            + "\n  " + e->m_char.m_char_data.m_class_name
            + "\n  map: " + e->m_char.m_char_data.m_mapName
            + "\n  db_id: " + QString::number(e->m_db_id) + ":" + QString::number(e->m_char.m_db_id)
            + "\n  idx: " + QString::number(e->m_idx)
            + "\n  access: " + QString::number(e->m_entity_data.m_access_level)
            + "\n  acct: " + QString::number(e->m_char.m_account_id)
            + "\n  lvl/clvl: " + QString::number(e->m_char.m_char_data.m_level) + "/" + QString::number(e->m_char.m_char_data.m_combat_level)
            + "\n  inf: " + QString::number(e->m_char.m_char_data.m_influence)
            + "\n  xp/debt: " + QString::number(e->m_char.m_char_data.m_experience_points) + "/" + QString::number(e->m_char.m_char_data.m_experience_debt)
            + "\n  lfg: " + QString::number(e->m_char.m_char_data.m_lfg)
            + "\n  afk: " + QString::number(e->m_char.m_char_data.m_afk)
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
    uint8_t attrib = cmd.split(" ").value(1).toUInt();

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
    int val = cmd.split(" ").value(1).toUInt();

    setTeamID(*e, val);

    QString msg = "Set Team ID to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetSuperGroup(QString &cmd, Entity *e) {
    MapClient *src  = e->m_client;
    QStringList args;
    args = cmd.split(QRegExp("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

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

    e->m_char.m_gui.guiDump(); // Send GUISettings dump
}

void cmdHandler_SetWindowVisibility(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    QStringList args;
    args = cmd.split(QRegExp("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    uint32_t idx = args.value(1).toInt();
    WindowVisibility val = (WindowVisibility)args.value(2).toInt();

    QString msg = "Toggling " + QString::number(idx) +  " GUIWindow visibility: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);

    e->m_char.m_gui.m_wnds.at(idx).setWindowVisibility(val); // Set WindowVisibility
    e->m_char.m_gui.m_wnds.at(idx).guiWindowDump(); // for debugging
}

void cmdHandler_KeybindDebug(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    QString msg = "Sending Keybinds dump to console output.";
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);

    e->m_char.m_keybinds.keybindsDump(); // Send GUISettings dump
}

void cmdHandler_ToggleLogging(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    QStringList args;
    args = cmd.split(QRegExp("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry
    args.removeFirst();

    QString msg = "Toggle logging of categories: " + args.join(" ");
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);

    for (auto category : args)
        toggleLogging(category); // Toggle each category listed
}

// Slash commands for setting bit values
void cmdHandler_SetU1(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.split(" ").value(1).toUInt();

    setu1(*e, val);

    QString msg = "Set u1 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU2(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.split(" ").value(1).toUInt();

    setu2(*e, val);

    QString msg = "Set u2 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU3(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.split(" ").value(1).toUInt();

    setu3(*e, val);

    QString msg = "Set u3 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU4(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.split(" ").value(1).toUInt();

    setu4(*e, val);

    QString msg = "Set u4 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU5(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.split(" ").value(1).toUInt();

    setu5(*e, val);

    QString msg = "Set u5 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU6(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.split(" ").value(1).toUInt();

    setu6(*e, val);

    QString msg = "Set u6 to: " + QString::number(val);
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

// Access Level 1 Commands
void cmdHandler_Help(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    QString msg = "Below is a list of all slash commands. They are not case sensitive.\n";

    for(const auto &sc : g_defined_slash_commands)
    {
        int alvl = getAccessLevel(*e);
        if(alvl >= sc.m_required_access_level)
            msg += "\t" + sc.m_valid_prefixes.join(", ") + "\n";
    }

    qCDebug(logSlashCommand).noquote() << cmd << ":\n" << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, src);
}

void cmdHandler_AFK(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    int space = cmd.indexOf(' ');
    QString val = cmd.mid(space+1);
    toggleAFK(e->m_char, val);

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
        Character *c        = &cl->char_entity()->m_char;
        QString name        = cl->char_entity()->name();
        QString lvl         = QString::number(getLevel(*c));
        QString clvl        = QString::number(getCombatLevel(*c));
        QString origin      = getOrigin(*c);
        QString archetype   = QString(getClass(*c)).remove("Class_");

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
    args = cmd.split(QRegExp("\"?( |$)(?=(([^\"]*\"){2})*[^\"]*$)\"?")); // regex wizardry

    if(cmd.toLower() == "settitles")
    {
        setTitles(e->m_char);
        msg = "Titles reset to nothing";
    }
    else
    {
        prefix  = !args.value(1).isEmpty();
        generic = args.value(2);
        origin  = args.value(3);
        special = args.value(4);
        setTitles(e->m_char, prefix, generic, origin, special);
        msg = "Titles changed to: " + QString::number(prefix) + " " + generic + " " + origin + " " + special;
    }
    qCDebug(logSlashCommand) << msg;
    sendInfoMessage(MessageChannel::USER_ERROR, msg, src);
}

void cmdHandler_Stuck(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    // TODO: Implement true move-to-safe-location-nearby logic
    e->m_entity_data.pos = glm::vec3(128.0f,16.0f,-198.0f); // Atlas Park starting location

    QString msg = "Resetting location to default spawn ("
            + QString::number(e->m_entity_data.pos.x) + ","
            + QString::number(e->m_entity_data.pos.y) + ","
            + QString::number(e->m_entity_data.pos.z) + ")";
    qCDebug(logSlashCommand) << cmd << ":" << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, src);
}

void cmdHandler_LFG(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleLFG(e->m_char);

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

    if(inviteTeam(*e,*tgt))
        msg = "Inviting " + name + " to team.";
    else
        msg = "Failed to invite " + name + ". They are already on a team.";

    qCDebug(logSlashCommand).noquote() << msg;
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

void cmdHandler_Sidekick(QString &cmd, Entity *e) {
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

    if(tgt == nullptr || e->m_char.isEmpty() || tgt->m_char.isEmpty())
        return;

    addSidekick(*e,*tgt);
}

void cmdHandler_UnSidekick(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    if(e->m_char.isEmpty())
        return;

    removeSidekick(*e);
}

void cmdHandler_TeamBuffs(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    toggleTeamBuffs(e->m_char);

    QString msg = "Toggling Team Buffs display mode.";
    qCDebug(logSlashCommand).noquote() << msg;
}
