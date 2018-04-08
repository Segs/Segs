#include "SlashCommand.h"
#include "DataHelpers.h"
#include "MapInstance.h"
#include "Settings.h"

#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
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
    {{"setSpeed", "speed"},"Set your player speed", &cmdHandler_SetSpeed, 9},
    {{"setBackupSpd", "BackupSpd"},"Set the Backup Speed", &cmdHandler_SetBackupSpd, 9},
    {{"setJumpHeight", "JumpHeight"},"Set the jump height", &cmdHandler_SetJumpHeight, 9},
    {{"setHP"},"Set the HP value of your character", &cmdHandler_SetHP, 9},
    {{"setEnd"},"Set your Endurance", &cmdHandler_SetEnd, 9},
    {{"setXP"},"Set your XP", &cmdHandler_SetXP, 9},
    {{"setDebt"},"Set your Debt", &cmdHandler_SetDebt, 9},
    {{"setInf"},"Set your Influence", &cmdHandler_SetInf, 9},
    {{"setLevel"},"Set your level", &cmdHandler_SetLevel, 9},
    {{"setCombatLevel"},"Set your combat level", &cmdHandler_SetCombatLevel, 9},
    {{"UpdateChar", "CharUpdate", "save"},"Update character information in the database", &cmdHandler_UpdateChar, 9},
    {{"DebugChar", "chardebug"},"View your characters debug information", &cmdHandler_DebugChar, 9},
    {{"ControlsDisabled"},"Disable controls", &cmdHandler_ControlsDisabled, 9},
    {{"updateid"},"Update ID", &cmdHandler_UpdateId, 9},
    {{"fullupdate"},"Full Update", &cmdHandler_FullUpdate, 9},
    {{"hascontrolid"},"Force the server to acknowledge input ids", &cmdHandler_HasControlId, 9},
    {{"setTeam"},"Set the team", &cmdHandler_SetTeam, 9},
    {{"setSuperGroup"},"Set your Super Group", &cmdHandler_SetSuperGroup, 9},
    {{"settingsDump"},"Output settings.cfg to console", &cmdHandler_SettingsDump, 9},
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
};

bool canAccessCommand(const SlashCommand &cmd, const Entity &e)
{
    MapClient *src = e.m_client;
    int alvl = getAccessLevel(e);
    if(alvl >= cmd.m_required_access_level)
        return true;

    QString msg = "You do not have adequate permissions to use the command: " + cmd.m_valid_prefixes.first();
    qDebug() << msg;
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
    qDebug() << "Unknown game command:" << str;
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
        qDebug() << errormsg;
        sendInfoMessage(MessageChannel::ADMIN, errormsg, src);
    }
}

void cmdHandler_Fly(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleFly(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_Falling(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleFalling(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_Sliding(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleSliding(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_Jumping(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleJumping(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_Stunned(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleStunned(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_Jumppack(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleJumppack(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetSpeed(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    QVector<QStringRef> args(cmd.splitRef(' '));
    float v1 = args.value(1).toFloat();
    float v2 = args.value(2).toFloat();
    float v3 = args.value(3).toFloat();
    setSpeed(*e, v1, v2, v3);

    QString msg = "Set Speed to: <"
            + QString::number(v1) + ","
            + QString::number(v2) + ","
            + QString::number(v3) + ">";
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetBackupSpd(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float val = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    setBackupSpd(*e, val);

    QString msg = "Set BackupSpd to: " + QString::number(val);
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetJumpHeight(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float val = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    setJumpHeight(*e, val);

    QString msg = "Set JumpHeight to: " + QString::number(val);
    qDebug() << msg;
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
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetEnd(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float attrib = cmd.midRef(cmd.indexOf(' ')+1).toFloat();
    float maxattrib = e->m_char->m_max_attribs.m_Endurance;

    if(attrib > maxattrib)
        attrib = maxattrib;

    setEnd(*e->m_char,attrib);

    QString msg = "Setting Endurance to: " + QString::number(attrib) + "/" + QString::number(maxattrib);
    qDebug() << msg;
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

    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetDebt(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    float attrib = cmd.midRef(cmd.indexOf(' ')+1).toFloat();

    setDebt(*e->m_char, attrib);
    QString msg = QString("Setting XP Debt to %1").arg(attrib);

    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetInf(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setInf(*e->m_char, attrib);

    QString msg = "Setting influence to: " + QString::number(attrib);
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetLevel(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setLevel(*e->m_char, attrib); // TODO: Why does this result in -1?

    QString msg = "Setting Level to: " + QString::number(attrib);
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetCombatLevel(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint32_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setCombatLevel(*e->m_char, attrib); // TODO: Why does this result in -1?

    QString msg = "Setting Combat Level to: " + QString::number(attrib);
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_UpdateChar(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    charUpdateDB(e);

    QString msg = "Updating Character in Database: " + e->name();
    qDebug() << cmd << ":" << msg;
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
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_UpdateId(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint8_t attrib = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setUpdateID(*e, attrib);

    QString msg = "Setting updateID to: " + QString::number(attrib);
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_FullUpdate(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleFullUpdate(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_HasControlId(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleControlId(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetTeam(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setTeamID(*e, val);

    QString msg = "Set Team ID to: " + QString::number(val);
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetSuperGroup(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setSuperGroupID(*e, val);

    QString msg = "Set SuperGroup ID to: " + QString::number(val);
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SettingsDump(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    QString msg = "Sending settings config dump to console output.";
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);

    settingsDump(); // Send settings dump
}

// Slash commands for setting bit values
void cmdHandler_SetU1(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu1(*e, val);

    QString msg = "Set u1 to: " + QString::number(val);
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU2(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu2(*e, val);

    QString msg = "Set u2 to: " + QString::number(val);
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU3(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu3(*e, val);

    QString msg = "Set u3 to: " + QString::number(val);
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU4(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu4(*e, val);

    QString msg = "Set u4 to: " + QString::number(val);
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU5(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu5(*e, val);

    QString msg = "Set u5 to: " + QString::number(val);
    qDebug() << msg;
    sendInfoMessage(MessageChannel::DEBUG_INFO, msg, src);
}

void cmdHandler_SetU6(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.midRef(cmd.indexOf(' ')+1).toUInt();

    setu6(*e, val);

    QString msg = "Set u6 to: " + QString::number(val);
    qDebug() << msg;
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
                msg += "\t" + sc.m_valid_prefixes.join(", ") + "[" + QString::number(sc.m_required_access_level) + "]:\t" + sc.m_help_text + "\n";
                msg_dlg += "<color #ffCC99><i>" + sc.m_valid_prefixes.join(", ") + "</i></color>[<color #66ffff>" + QString::number(sc.m_required_access_level) + "</color>]: " + sc.m_help_text + "<br>";
            }
        }
    }
    // Dialog output
    StandardDialogCmd *dlg = new StandardDialogCmd(msg_dlg);
    src->addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
    // CMD line (debug) output
    qDebug().noquote() << cmd << ":\n" << msg;
}

void cmdHandler_AFK(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    int space = cmd.indexOf(' ');
    QString val = cmd.mid(space+1);
    toggleAFK(*e->m_char, val);

    QString msg = "Setting afk message to: " + val;
    qDebug() << msg;
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

    qDebug().noquote() << msg;
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
    qDebug() << msg;
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
    qDebug() << cmd << ":" << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, src);
}

void cmdHandler_LFG(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleLFG(*e->m_char);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, src);
}

void cmdHandler_MOTD(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    sendServerMOTD(e);

    QString msg = "Opening Server MOTD";
    qDebug().noquote() << msg;
    sendInfoMessage(MessageChannel::SERVER, msg, src);
}
