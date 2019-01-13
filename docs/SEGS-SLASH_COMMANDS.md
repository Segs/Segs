| Command                        | Description                                                                    | Access Level |
|--------------------------------|--------------------------------------------------------------------------------|--------------|
| Script                         | Run a Script                                                                   | ADMIN        |
| Dialog, dlg                    | Open a dialog box with any string arg                                          | ADMIN        |
| InfoMessage, imsg              | Send an info message into chat. Expects <int> <string>, e.g. /imsg 1 test      | ADMIN        |
| fly, flying                    | Toggle flying On/Off                                                           | ADMIN        |
| falling                        | Toggle falling On/Off                                                          | ADMIN        |
| sliding                        | Toggle sliding On/Off                                                          | ADMIN        |
| jumping                        | Toggle jumping On/Off                                                          | ADMIN        |
| stunned                        | Toggle stunned character On/Off                                                | ADMIN        |
| jumppack                       | Toggle jump pack On/Off                                                        | ADMIN        |
| setSpeed, speed                | Set your player Speed                                                          | ADMIN        |
| setBackupSpd, BackupSpd        | Set the Backup Speed                                                           | ADMIN        |
| setJumpHeight, JumpHeight      | Set the Jump Height                                                            | ADMIN        |
| setHP                          | Set the HP value of your character                                             | ADMIN        |
| setEnd                         | Set your Endurance                                                             | ADMIN        |
| setXP                          | Set your XP                                                                    | ADMIN        |
| setDebt                        | Set your Debt                                                                  | ADMIN        |
| setInf                         | Set your Influence                                                             | ADMIN        |
| setLevel                       | Set your Level                                                                 | ADMIN        |
| setCombatLevel                 | Set your Combat Level                                                          | ADMIN        |
| UpdateChar, CharUpdate, save   | Update character information in the database                                   | ADMIN        |
| DebugChar, chardebug           | View your characters debug information                                         | ADMIN        |
| ControlsDisabled               | Disable controls                                                               | ADMIN        |
| updateid                       | Update ID                                                                      | ADMIN        |
| fullupdate                     | Full Update                                                                    | ADMIN        |
| hascontrolid                   | Force the server to acknowledge input ids                                      | ADMIN        |
| setTeam, setTeamID             | Set the team idx                                                               | ADMIN        |
| setSuperGroup, setSG           | Set your Super Group                                                           | ADMIN        |
| settingsDump, settingsDebug    | Output settings.cfg to console                                                 | ADMIN        |
| teamDump, teamDebug            | Output team settings to console                                                | ADMIN        |
| guiDump, guiDebug              | Output gui settings to console                                                 | ADMIN        |
| setWindowVisibility, setWinVis |                                                                                | ADMIN        |
| keybindDump, keybindDebug      | Output keybind settings to console                                             | ADMIN        |
| toggleLogging, log             | Modify log categories (e.g. input, teams, ...)                                 | ADMIN        |
| friendsDump, friendsDebug      | Output friendlist info to console                                              | ADMIN        |
| damage, heal                   | Make current target (or self) take damage/health                               | ADMIN        |
| interp                         | Toggle Interpolation                                                           | ADMIN        |
| moveinstantly                  | Toggle move_instantly                                                          | ADMIN        |
| collision                      | Toggle Collision on/off                                                        | ADMIN        |
| movement                       | Toggle server authority for Movement on/off                                    | ADMIN        |
| setSeq                         | Set Sequence values <update> <move_idx> <duration>                             | ADMIN        |
| addTriggeredMove               | Set TriggeredMove values <move_idx> <delay> <fx_idx>                           | ADMIN        |
| timestate, setTimeStateLog     | Set TimeStateLog value.                                                        | ADMIN        |
| clientstate                    | Set ClientState mode                                                           | ADMIN        |
| addpowerset                    | Adds entire PowerSet (by 'pcat pset' idxs) to Entity                           | ADMIN        |
| addpower                       | Adds Power (by 'pcat pset pow' idxs) to Entity                                 | ADMIN        |
| addinsp                        | Adds Inspiration (by name) to Entity                                           | ADMIN        |
| addboost, addEnhancement       | Adds Enhancement (by name) to Entity                                           | ADMIN        |
| levelupxp                      | Level Up Character to Level Provided                                           | ADMIN        |
| face                           | Face a target                                                                  | ADMIN        |
| faceLocation                   | Face a location                                                                | ADMIN        |
| mapmove, movezone, mz          | Move to a map id                                                               | ADMIN        |
| deadnogurney                   | Test Dead No Gurney. Fakes sending the client packet.                          | ADMIN        |
| doormsg                        | Test Door Message. Fakes sending the client packet.                            | ADMIN        |
| browser                        | Test Browser. Sends content to a browser window                                | ADMIN        |
| timeupdate                     | Test TimeUpdate. Sends time update to server                                   | ADMIN        |
| contactdlg, cdlg               | Test ContactDialog. Sends contact dialog with responses to server              | ADMIN        |
| contactdlgyesno, cdlg2         | Test ContactDialogYesNoOk. Sends contact dialog with yes/no response to server | ADMIN        |
| setwaypoint                    | Test SendWaypoint. Send waypoint to client                                     | ADMIN        |
| setstatemode                   | Send StateMode. Send StateMode to client                                       | ADMIN        |
| revive                         | Revive Self or Target Player                                                   | ADMIN        |
| addcostumeslot                 | Add a new costume slot to Character                                            | ADMIN        |
| contactList                    | Update Contact List                                                            | ADMIN        |
| testTask                       | Test Task                                                                      | ADMIN        |
| reloadLua                      | Reload all Lua scripts                                                         | ADMIN        |
| openStore                      | Open store Window                                                              | ADMIN        |
| forceLogout                    | Logout player                                                                  | ADMIN        |
| sendLocation                   | Send Location Test                                                             | ADMIN        |
| developerConsoleOutput         | Send message to -console window                                                | ADMIN        |
| clientConsoleOutput            | Send message to ingame (~) console                                             | ADMIN        |
| setu1                          | Set bitvalue u1. Used for live-debugging.                                      | ADMIN        |
| addNpc                         | add <npc_name> with costume [variation] in front of gm                         | GM           |
| moveTo, setpos, setpospyr      | set the gm's position to <x> <y> <z>                                           | GM           |
| cmdlist, commandlist           | List all accessible commands                                                   | PLAYER       |
| afk                            | Mark yourself as Away From Keyboard                                            | PLAYER       |
| whoall                         | Shows who is on the current map                                                | PLAYER       |
| setTitles, title               | Open the Title selection window                                                | PLAYER       |
| setCustomTitles                | Set your titles manually                                                       | PLAYER       |
| setSpecialTitle                | Set your Special title                                                         | PLAYER       |
| stuck                          | Free yourself if your character gets stuck                                     | PLAYER       |
| gotospawn                      | Teleport to a SpawnLocation                                                    | PLAYER       |
| lfg                            | Toggle looking for group status                                                | PLAYER       |
| motd                           | View the server MOTD                                                           | PLAYER       |
| i, invite                      | Invite player to team                                                          | PLAYER       |
| k, kick                        | Kick player from team                                                          | PLAYER       |
| leaveteam                      | Leave the team you're currently on                                             | PLAYER       |
| findmember                     | Open Looking For Group Window                                                  | PLAYER       |
| makeleader, ml                 | Make another player team leader                                                | PLAYER       |
| assist                         | Re-target based on the selected entity's current target                        | PLAYER       |
| sidekick, sk                   | Invite player to sidekick                                                      | PLAYER       |
| unsidekick, unsk               | Remove sidekick/duo relationship                                               | PLAYER       |
| buffs                          | Toggle buffs display on team window                                            | PLAYER       |
| friend                         | Add friend to friendlist                                                       | PLAYER       |
| unfriend, estrange             | Remove friend from friendlist                                                  | PLAYER       |
| friendlist, fl                 | Toggle visibility of friendslist                                               | PLAYER       |
| MapXferList, mapmenu           | Show MapXferList                                                               | PLAYER       |
| respec                         | Start ReSpec                                                                   | PLAYER       |
| trade                          | Trade with player                                                              | PLAYER       |
| tailor                         | Open Tailor Window                                                             | PLAYER       |
| cc                             | Costume Change                                                                 | PLAYER       |
| train                          | Train Up Level                                                                 | PLAYER       |
