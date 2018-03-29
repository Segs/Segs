#include "Entity.h"
#include "Character.h"
#include "DataHelpers.h"
#include "MapClient.h"

class QString;
class InfoMessageCmd; // leverage InfoMessageCmd

struct SlashCommand
{
    QStringList m_valid_prefixes;
    std::function<void(QString &,Entity *)> m_handler;
    int m_required_access_level;
};

extern std::vector<SlashCommand> g_defined_slash_commands;

bool canAccessCommand(SlashCommand &cmd, const Entity &e);
void runCommand(QString &str, Entity &e);

// Access Level 9 Commands (GMs)
void cmdHandler_Script(QString &cmd, Entity *e);
void cmdHandler_Dialog(QString &cmd, Entity *e);
void cmdHandler_InfoMessage(QString &cmd, Entity *e);
void cmdHandler_SmileX(QString &cmd, Entity *e);
void cmdHandler_Fly(QString &cmd, Entity *e);
void cmdHandler_Falling(QString &cmd, Entity *e);
void cmdHandler_Sliding(QString &cmd, Entity *e);
void cmdHandler_Jumping(QString &cmd, Entity *e);
void cmdHandler_Stunned(QString &cmd, Entity *e);
void cmdHandler_Jumppack(QString &cmd, Entity *e);
void cmdHandler_SetSpeed(QString &cmd, Entity *e);
void cmdHandler_SetBackupSpd(QString &cmd, Entity *e);
void cmdHandler_SetJumpHeight(QString &cmd, Entity *e);
void cmdHandler_SetHP(QString &cmd, Entity *e);
void cmdHandler_SetEnd(QString &cmd, Entity *e);
void cmdHandler_SetXP(QString &cmd, Entity *e);
void cmdHandler_SetDebt(QString &cmd, Entity *e);
void cmdHandler_SetInf(QString &cmd, Entity *e);
void cmdHandler_SetLevel(QString &cmd, Entity *e);
void cmdHandler_SetCombatLevel(QString &cmd, Entity *e);
void cmdHandler_UpdateChar(QString &cmd, Entity *e);
void cmdHandler_DebugChar(QString &cmd, Entity *e);
void cmdHandler_ControlsDisabled(QString &cmd, Entity *e);
void cmdHandler_UpdateId(QString &cmd, Entity *e);
void cmdHandler_FullUpdate(QString &cmd, Entity *e);
void cmdHandler_HasControlId(QString &cmd, Entity *e);
void cmdHandler_SetTeam(QString &cmd, Entity *e);
void cmdHandler_SetSuperGroup(QString &cmd, Entity *e);
void cmdHandler_SettingsDump(QString &cmd, Entity *e);
void cmdHandler_TeamDebug(QString &cmd, Entity *e);
void cmdHandler_GUIDebug(QString &cmd, Entity *e);
void cmdHandler_SetWindowVisibility(QString &cmd, Entity *e);
void cmdHandler_KeybindDebug(QString &cmd, Entity *e);
void cmdHandler_ToggleLogging(QString &cmd, Entity *e);
void cmdHandler_FriendsListDebug(QString &cmd, Entity *e);
void cmdHandler_SetU1(QString &cmd, Entity *e);
void cmdHandler_SetU2(QString &cmd, Entity *e);
void cmdHandler_SetU3(QString &cmd, Entity *e);
void cmdHandler_SetU4(QString &cmd, Entity *e);
void cmdHandler_SetU5(QString &cmd, Entity *e);
void cmdHandler_SetU6(QString &cmd, Entity *e);

// Access Level 1 Commands
void cmdHandler_Help(QString &cmd, Entity *e);
void cmdHandler_AFK(QString &cmd, Entity *e);
void cmdHandler_WhoAll(QString &cmd, Entity *e);
void cmdHandler_SetTitles(QString &cmd, Entity *e);
void cmdHandler_Stuck(QString &cmd, Entity *e);
void cmdHandler_LFG(QString &cmd, Entity *e);
void cmdHandler_MOTD(QString &cmd, Entity *e);
void cmdHandler_Invite(QString &cmd, Entity *e);
void cmdHandler_Kick(QString &cmd, Entity *e);
void cmdHandler_LeaveTeam(QString &cmd, Entity *e);
void cmdHandler_FindMember(QString &cmd, Entity *e);
void cmdHandler_MakeLeader(QString &cmd, Entity *e);
void cmdHandler_SetAssistTarget(QString &cmd, Entity *e);
void cmdHandler_Sidekick(QString &cmd, Entity *e);
void cmdHandler_UnSidekick(QString &cmd, Entity *e);
void cmdHandler_TeamBuffs(QString &cmd, Entity *e);
void cmdHandler_Friend(QString &cmd, Entity *e);
void cmdHandler_Unfriend(QString &cmd, Entity *e);
void cmdHandler_FriendList(QString &cmd, Entity *e);
