| Command                        | Description                                                                    | Access Level |
|--------------------------------|--------------------------------------------------------------------------------|--------------|
| Script                         | Run a Script                                                                   | ADMIN(9)     |
| Dialog, dlg                    | Open a dialog box with any string arg                                          | ADMIN(9)     |
| InfoMessage, imsg              | Send an info message into chat. Expects <int> <string>, e.g. /imsg 1 test      | ADMIN(9)     |
| fly, flying                    | Toggle flying On/Off                                                           | ADMIN(9)     |
| falling                        | Toggle falling On/Off                                                          | ADMIN(9)     |
| sliding                        | Toggle sliding On/Off                                                          | ADMIN(9)     |
| jumping                        | Toggle jumping On/Off                                                          | ADMIN(9)     |
| stunned                        | Toggle stunned character On/Off                                                | ADMIN(9)     |
| jumppack                       | Toggle jump pack On/Off                                                        | ADMIN(9)     |
| setSpeed, speed                | Set your player Speed                                                          | ADMIN(9)     |
| setBackupSpd, BackupSpd        | Set the Backup Speed                                                           | ADMIN(9)     |
| setJumpHeight, JumpHeight      | Set the Jump Height                                                            | ADMIN(9)     |
| setHP                          | Set the HP value of your character                                             | ADMIN(9)     |
| setEnd                         | Set your Endurance                                                             | ADMIN(9)     |
| setXP                          | Set your XP                                                                    | ADMIN(9)     |
| setDebt                        | Set your Debt                                                                  | ADMIN(9)     |
| setInf                         | Set your Influence                                                             | ADMIN(9)     |
| setLevel                       | Set your Level                                                                 | ADMIN(9)     |
| setCombatLevel                 | Set your Combat Level                                                          | ADMIN(9)     |
| UpdateChar, CharUpdate, save   | Update character information in the database                                   | ADMIN(9)     |
| DebugChar, chardebug           | View your characters debug information                                         | ADMIN(9)     |
| ControlsDisabled               | Disable controls                                                               | ADMIN(9)     |
| updateid                       | Update ID                                                                      | ADMIN(9)     |
| fullupdate                     | Full Update                                                                    | ADMIN(9)     |
| hascontrolid                   | Force the server to acknowledge input ids                                      | ADMIN(9)     |
| setTeam, setTeamID             | Set the team idx                                                               | ADMIN(9)     |
| setSuperGroup, setSG           | Set your Super Group                                                           | ADMIN(9)     |
| settingsDump, settingsDebug    | Output settings.cfg to console                                                 | ADMIN(9)     |
| teamDump, teamDebug            | Output team settings to console                                                | ADMIN(9)     |
| guiDump, guiDebug              | Output gui settings to console                                                 | ADMIN(9)     |
| setWindowVisibility, setWinVis |                                                                                | ADMIN(9)     |
| keybindDump, keybindDebug      | Output keybind settings to console                                             | ADMIN(9)     |
| toggleLogging, log             | Modify log categories (e.g. input, teams, ...)                                 | ADMIN(9)     |
| friendsDump, friendsDebug      | Output friendlist info to console                                              | ADMIN(9)     |
| damage, heal                   | Make current target (or self) take damage/health                               | ADMIN(9)     |
| interp                         | Toggle Interpolation                                                           | ADMIN(9)     |
| moveinstantly                  | Toggle move_instantly                                                          | ADMIN(9)     |
| collision                      | Toggle Collision on/off                                                        | ADMIN(9)     |
| movement                       | Toggle server authority for Movement on/off                                    | ADMIN(9)     |
| setSeq                         | Set Sequence values <update> <move_idx> <duration>                             | ADMIN(9)     |
| addTriggeredMove               | Set TriggeredMove values <move_idx> <delay> <fx_idx>                           | ADMIN(9)     |
| timestate, setTimeStateLog     | Set TimeStateLog value.                                                        | ADMIN(9)     |
| clientstate                    | Set ClientState mode                                                           | ADMIN(9)     |
| addpowerset                    | Adds entire PowerSet (by 'pcat pset' idxs) to Entity                           | ADMIN(9)     |
| addpower                       | Adds Power (by 'pcat pset pow' idxs) to Entity                                 | ADMIN(9)     |
| addinsp                        | Adds Inspiration (by name) to Entity                                           | ADMIN(9)     |
| addboost, addEnhancement       | Adds Enhancement (by name) to Entity                                           | ADMIN(9)     |
| levelupxp                      | Level Up Character to Level Provided                                           | ADMIN(9)     |
| face                           | Face a target                                                                  | ADMIN(9)     |
| faceLocation                   | Face a location                                                                | ADMIN(9)     |
| mapmove, movezone, mz          | Move to a map id                                                               | ADMIN(9)     |
| deadnogurney                   | Test Dead No Gurney. Fakes sending the client packet.                          | ADMIN(9)     |
| doormsg                        | Test Door Message. Fakes sending the client packet.                            | ADMIN(9)     |
| browser                        | Test Browser. Sends content to a browser window                                | ADMIN(9)     |
| timeupdate                     | Test TimeUpdate. Sends time update to server                                   | ADMIN(9)     |
| contactdlg, cdlg               | Test ContactDialog. Sends contact dialog with responses to server              | ADMIN(9)     |
| contactdlgyesno, cdlg2         | Test ContactDialogYesNoOk. Sends contact dialog with yes/no response to server | ADMIN(9)     |
| setwaypoint                    | Test SendWaypoint. Send waypoint to client                                     | ADMIN(9)     |
| setstatemode                   | Send StateMode. Send StateMode to client                                       | ADMIN(9)     |
| revive                         | Revive Self or Target Player                                                   | ADMIN(9)     |
| addcostumeslot                 | Add a new costume slot to Character                                            | ADMIN(9)     |
| contactList                    | Update Contact List                                                            | ADMIN(9)     |
| testTask                       | Test Task                                                                      | ADMIN(9)     |
| reloadLua                      | Reload all Lua scripts                                                         | ADMIN(9)     |
| openStore                      | Open store Window                                                              | ADMIN(9)     |
| forceLogout                    | Logout player                                                                  | ADMIN(9)     |
| sendLocation                   | Send Location Test                                                             | ADMIN(9)     |
| developerConsoleOutput         | Send message to -console window                                                | ADMIN(9)     |
| clientConsoleOutput            | Send message to ingame (~) console                                             | ADMIN(9)     |
| setu1                          | Set bitvalue u1. Used for live-debugging.                                      | ADMIN(9)     |
| addNpc                         | add <npc_name> with costume [variation] in front of gm                         | GM(2)        |
| moveTo, setpos, setpospyr      | set the gm's position to <x> <y> <z>                                           | GM(2)        |
| cmdlist, commandlist           | List all accessible commands                                                   | PLAYER(1)    |
| afk                            | Mark yourself as Away From Keyboard                                            | PLAYER(1)    |
| whoall                         | Shows who is on the current map                                                | PLAYER(1)    |
| setTitles, title               | Open the Title selection window                                                | PLAYER(1)    |
| setCustomTitles                | Set your titles manually                                                       | PLAYER(1)    |
| setSpecialTitle                | Set your Special title                                                         | PLAYER(1)    |
| stuck                          | Free yourself if your character gets stuck                                     | PLAYER(1)    |
| gotospawn                      | Teleport to a SpawnLocation                                                    | PLAYER(1)    |
| lfg                            | Toggle looking for group status                                                | PLAYER(1)    |
| motd                           | View the server MOTD                                                           | PLAYER(1)    |
| i, invite                      | Invite player to team                                                          | PLAYER(1)    |
| k, kick                        | Kick player from team                                                          | PLAYER(1)    |
| leaveteam                      | Leave the team you're currently on                                             | PLAYER(1)    |
| findmember                     | Open Looking For Group Window                                                  | PLAYER(1)    |
| makeleader, ml                 | Make another player team leader                                                | PLAYER(1)    |
| assist                         | Re-target based on the selected entity's current target                        | PLAYER(1)    |
| sidekick, sk                   | Invite player to sidekick                                                      | PLAYER(1)    |
| unsidekick, unsk               | Remove sidekick/duo relationship                                               | PLAYER(1)    |
| buffs                          | Toggle buffs display on team window                                            | PLAYER(1)    |
| friend                         | Add friend to friendlist                                                       | PLAYER(1)    |
| unfriend, estrange             | Remove friend from friendlist                                                  | PLAYER(1)    |
| friendlist, fl                 | Toggle visibility of friendslist                                               | PLAYER(1)    |
| MapXferList, mapmenu           | Show MapXferList                                                               | PLAYER(1)    |
| respec                         | Start ReSpec                                                                   | PLAYER(1)    |
| trade                          | Trade with player                                                              | PLAYER(1)    |
| tailor                         | Open Tailor Window                                                             | PLAYER(1)    |
| cc                             | Costume Change                                                                 | PLAYER(1)    |
| train                          | Train Up Level                                                                 | PLAYER(1)    |
