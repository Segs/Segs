#include "SlashCommand.h"
#include "DataHelpers.h"
#include "MapInstance.h"

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
    {{"UpdateChar", "updateDB"}, &cmdHandler_UpdateChar, 9},
    {{"DebugChar", "chardebug"}, &cmdHandler_DebugChar, 9},
    {{"ControlsDisabled"}, &cmdHandler_ControlsDisabled, 9},
    {{"updateid"}, &cmdHandler_UpdateId, 9},
    {{"fullupdate"}, &cmdHandler_FullUpdate, 9},
    {{"hascontrolid"}, &cmdHandler_HasControlId, 9},
    {{"setu1"}, &cmdHandler_SetU1, 9},
    {{"setu2"}, &cmdHandler_SetU2, 9},
    {{"setu3"}, &cmdHandler_SetU3, 9},
    {{"setu4"}, &cmdHandler_SetU4, 9},
    {{"setu5"}, &cmdHandler_SetU5, 9},
    {{"setu6"}, &cmdHandler_SetU6, 9},
    /* Access Level 1 Commands */
    {{"afk"}, &cmdHandler_AFK, 1},
    {{"whoall"}, &cmdHandler_WhoAll, 1},
    {{"setTitles"}, &cmdHandler_SetTitles, 1},
    {{"stuck", "help"}, &cmdHandler_Stuck, 1},
    {{"lfg"}, &cmdHandler_LFG, 1},
};

bool canAccessCommand(const SlashCommand &cmd, const Entity &e)
{
    MapClient *src = e.m_client;
    int alvl = getAccessLevel(e);
    if(alvl >= cmd.m_required_access_level)
        return true;

    QString msg = "You do not have adequate permissions to use the command: " + cmd.m_valid_prefixes.first();
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::USER_ERROR, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
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
    // TODO: This doesn't work anymore because it's out of scope.
    std::unique_ptr<ScriptingEngine> m_scripting_interface;

    QString code = cmd.mid(7,cmd.size()-7);
    m_scripting_interface->runScript(src,code,"user provided script");
}

void cmdHandler_Dialog(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    StandardDialogCmd *dlg = new StandardDialogCmd(cmd.mid(4));
    src->addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
}

void cmdHandler_InfoMessage(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    int first_space = cmd.indexOf(' ');
    int second_space = cmd.indexOf(' ',first_space+1);
    if(second_space==-1) {
        info = new InfoMessageCmd(InfoType::USER_ERROR,
                                       "The /imsg command takes two arguments, a <b>number</b> and a <b>string</b>"
                                       );
    }
    else {
        bool ok = true;
        int cmdType = cmd.midRef(first_space+1,second_space-(first_space+1)).toInt(&ok);
        if(!ok || cmdType<1 || cmdType>21) {
            info = new InfoMessageCmd(InfoType::USER_ERROR,
                                           "The first /imsg argument must be a <b>number</b> between 1 and 21"
                                           );
        }
        else {
            info = new InfoMessageCmd(InfoType(cmdType),cmd.mid(second_space+1));
        }

    }
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SmileX(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    int space = cmd.indexOf(' ');
    QString fileName("scripts/" + cmd.mid(space+1));
    if(!fileName.endsWith(".smlx"))
            fileName.append(".smlx");
    QFile file(fileName);
    if(QFileInfo::exists(fileName) && file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString contents(file.readAll());
        StandardDialogCmd *dlg = new StandardDialogCmd(contents);
        src->addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
    }
    else {
        QString errormsg = "Failed to load smilex file. \'" + file.fileName() + "\' not found.";
        qDebug() << errormsg;
        src->addCommandToSendNextUpdate(std::unique_ptr<ChatMessage>(ChatMessage::adminMessage(errormsg)));
    }
}

void cmdHandler_Fly(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleFly(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_Falling(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleFalling(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_Sliding(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleSliding(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_Jumping(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleJumping(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_Stunned(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleStunned(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_Jumppack(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleJumppack(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
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
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SetBackupSpd(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float val = cmd.split(" ").value(1).toFloat();
    setBackupSpd(*e, val);

    QString msg = "Set BackupSpd to: " + QString::number(val);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SetJumpHeight(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float val = cmd.split(" ").value(1).toFloat();
    setJumpHeight(*e, val);

    QString msg = "Set JumpHeight to: " + QString::number(val);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SetHP(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float attrib = cmd.split(" ").value(1).toFloat();
    float maxattrib = e->m_char.m_max_attribs.m_HitPoints;

    if(attrib > maxattrib)
        attrib = maxattrib;

    setHP(e->m_char,attrib);

    QString msg = "Setting HP to: " + QString::number(attrib) + "/" + QString::number(maxattrib);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SetEnd(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    float attrib = cmd.split(" ").value(1).toFloat();
    float maxattrib = e->m_char.m_max_attribs.m_Endurance;

    if(attrib > maxattrib)
        attrib = maxattrib;

    setEnd(e->m_char,attrib);

    QString msg = "Setting Endurance to: " + QString::number(attrib) + "/" + QString::number(maxattrib);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
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

    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SetDebt(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    float attrib = cmd.split(" ").value(1).toFloat();

    setDebt(e->m_char, attrib);
    QString msg = "Setting XP Debt to " + QString::number(attrib);

    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SetInf(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint32_t attrib = cmd.split(" ").value(1).toUInt();

    setInf(e->m_char, attrib);

    QString msg = "Setting influence to: " + QString::number(attrib);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SetLevel(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint32_t attrib = cmd.split(" ").value(1).toUInt();

    setLevel(e->m_char, attrib); // TODO: Why does this result in -1?

    QString msg = "Setting Level to: " + QString::number(attrib);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SetCombatLevel(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint32_t attrib = cmd.split(" ").value(1).toUInt();

    setCombatLevel(e->m_char, attrib); // TODO: Why does this result in -1?

    QString msg = "Setting Combat Level to: " + QString::number(attrib);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::REGULAR, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_UpdateChar(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    charUpdateDB(e);

    QString msg = "Updating Character in Database: " + e->name();
    qDebug() << cmd << ":" << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
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
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_ControlsDisabled(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleControlsDisabled(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_UpdateId(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    uint8_t attrib = cmd.split(" ").value(1).toUInt();

    setUpdateID(*e, attrib);

    QString msg = "Setting updateID to: " + QString::number(attrib);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_FullUpdate(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleFullUpdate(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_HasControlId(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleControlId(*e);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

// Slash commands for setting bit values
void cmdHandler_SetU1(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.split(" ").value(1).toUInt();

    setu1(*e, val);

    QString msg = "Set u1 to: " + QString::number(val);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SetU2(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.split(" ").value(1).toUInt();

    setu2(*e, val);

    QString msg = "Set u2 to: " + QString::number(val);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SetU3(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.split(" ").value(1).toUInt();

    setu3(*e, val);

    QString msg = "Set u3 to: " + QString::number(val);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SetU4(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.split(" ").value(1).toUInt();

    setu4(*e, val);

    QString msg = "Set u4 to: " + QString::number(val);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SetU5(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.split(" ").value(1).toUInt();

    setu5(*e, val);

    QString msg = "Set u5 to: " + QString::number(val);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_SetU6(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    int val = cmd.split(" ").value(1).toUInt();

    setu6(*e, val);

    QString msg = "Set u6 to: " + QString::number(val);
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::DEBUG_INFO, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

// Access Level 1 Commands
void cmdHandler_AFK(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    int space = cmd.indexOf(' ');
    QString val = cmd.mid(space+1);
    toggleAFK(e->m_char, val);

    QString msg = "Setting afk message to: " + val;
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::EMOTE, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_WhoAll(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;
    // TODO: This doesn't work anymore because it's out of scope.
    ClientStore<MapClient> m_clients;

    QString msg = "Players on this map:\n";

    for(MapClient *cl : m_clients)
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

    qDebug().noquote() << msg;
    info = new InfoMessageCmd(InfoType::SVR_COM, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
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
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::USER_ERROR, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
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
    info = new InfoMessageCmd(InfoType::SVR_COM, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}

void cmdHandler_LFG(QString &cmd, Entity *e) {
    MapClient *src = e->m_client;

    toggleLFG(e->m_char);

    QString msg = "Toggling " + cmd;
    qDebug() << msg;
    info = new InfoMessageCmd(InfoType::SVR_COM, msg);
    src->addCommandToSendNextUpdate(std::unique_ptr<InfoMessageCmd>(info));
}
