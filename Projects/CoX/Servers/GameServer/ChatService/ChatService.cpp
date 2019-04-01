/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#include "ChatService.h"
#include "GameData/Character.h"
#include "GameData/CharacterHelpers.h"
#include "GameData/playerdata_definitions.h"
#include "GameData/Entity.h"
#include "GameData/EntityHelpers.h"
#include "Messages/Map/MapEvents.h"
#include "Servers/MapServer/DataHelpers.h"

#include <QtCore/QDebug>
#include <QRegularExpression>

using namespace SEGSEvents;

std::unique_ptr<ChatServiceToClientData> ChatService::on_emote_command(Entity *ent, const QString &command)
{
    QString msg;                                                                // Initialize the variable to hold the debug message.
    MapClientSession *src = ent->m_client;
    std::vector<MapClientSession *> recipients;

    QString cmd_str = command.section(QRegularExpression("\\s+"), 0, 0);
    QString original_emote = command.section(QRegularExpression("\\s+"), 1, -1);
    QString lowerContents = original_emote.toLower();
                                                                             // Normal Emotes
    static const QStringList afraidCommands = {"afraid", "cower", "fear", "scared"};
    static const QStringList akimboCommands = {"akimbo", "wings"};
    static const QStringList bigWaveCommands = {"bigwave", "overhere"};
    static const QStringList boomBoxCommands = {"boombox", "bb", "dropboombox"};
    static const QStringList bowCommands = {"bow", "sorry"};
    static const QStringList bowDownCommands = {"bowdown", "down"};
    static const QStringList coinCommands = {"coin", "cointoss", "flipcoin"};
    static const QStringList diceCommands = {"dice", "rolldice"};
    static const QStringList evilLaughCommands = {"evillaugh", "elaugh", "muahahaha", "villainlaugh", "villainouslaugh"};
    static const QStringList fancyBowCommands = {"fancybow", "elegantbow"};
    static const QStringList flex1Commands = {"flex1", "flexa"};
    static const QStringList flex2Commands = {"flex2", "flex", "flexb"};
    static const QStringList flex3Commands = {"flex3", "flexc"};
    static const QStringList hiCommands = {"hi", "wave"};
    static const QStringList hmmCommands = {"hmmm", "plotting"};
    static const QStringList laugh2Commands = {"laugh2", "biglaugh", "laughtoo"};
    static const QStringList martialArtsCommands = {"martialarts", "kata"};
    static const QStringList newspaperCommands = {"newspaper"};
    static const QStringList noCommands = {"no", "dontattack"};
    static const QStringList plotCommands = {"plot", "scheme"};
    static const QStringList stopCommands = {"stop", "raisehand"};
    static const QStringList tarzanCommands = {"tarzan", "beatchest"};
    static const QStringList taunt1Commands = {"taunt1", "taunta"};
    static const QStringList taunt2Commands = {"taunt2", "taunt", "tauntb"};
    static const QStringList thanksCommands = {"thanks", "thankyou"};
    static const QStringList waveFistCommands = {"wavefist", "rooting"};
    static const QStringList winnerCommands = {"winner", "champion"};
    static const QStringList yesCommands = {"yes", "thumbsup"};
    static const QStringList yogaCommands = {"yoga", "lotus"};
    static const QStringList snowflakesCommands = {"snowflakes", "throwsnowflakes"};

    if(afraidCommands.contains(lowerContents))                                  // Afraid: Cower in fear, hold stance.
    {
        if(ent->m_motion_state.m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Afraid emote";
        else
            msg = "Unhandled ground Afraid emote";
    }
    else if(akimboCommands.contains(lowerContents) && !ent->m_motion_state.m_is_flying)        // Akimbo: Stands with fists on hips looking forward, hold stance.
        msg = "Unhandled Akimbo emote";                                         // Not allowed when flying.
    else if(lowerContents == "angry")                                           // Angry: Fists on hips and slouches forward, as if glaring or grumbling, hold stance.
        msg = "Unhandled Angry emote";
    else if(lowerContents == "atease")                                          // AtEase: Stands in the 'at ease' military position (legs spread out slightly, hands behind back) stance, hold stance.
        msg = "Unhandled AtEase emote";
    else if(lowerContents == "attack")                                          // Attack: Gives a charge! type point, fists on hips stance.
        msg = "Unhandled Attack emote";
    else if(lowerContents == "batsmash")                                        // BatSmash: Hit someone or something with a bat, repeat.
        msg = "Unhandled BatSmash emote";
    else if(lowerContents == "batsmashreact")                                   // BatSmashReact: React as if getting hit with a bat, often used in duo with BatSmash.
        msg = "Unhandled BatSmashReact emote";
    else if(bigWaveCommands.contains(lowerContents))                            // BigWave: Waves over the head, fists on hips stance.
        msg = "Unhandled BigWave emote";
    else if(boomBoxCommands.contains(lowerContents) && !ent->m_motion_state.m_is_flying)       // BoomBox (has sound): Summons forth a boombox (it just appears) and leans over to turn it on, stands up and does a sort of dance. A random track will play.
        msg = process_random_boombox_emote();
    else if(bowCommands.contains(lowerContents) && !ent->m_motion_state.m_is_flying)           // Bow: Chinese/Japanese style bow with palms together, returns to normal stance.
        msg = "Unhandled Bow emote";                                            // Not allowed when flying.
    else if(bowDownCommands.contains(lowerContents))                            // BowDown: Thrusts hands forward, then points down, as if ordering someone else to bow before you.
        msg = "Unhandled BowDown emote";
    else if(lowerContents == "burp" && !ent->m_motion_state.m_is_flying)                       // Burp (has sound): A raunchy belch, wipes mouth with arm afterward, ape-like stance.
        msg = "Unhandled Burp emote";                                           // Not allowed when flying.
    else if(lowerContents == "cheer")                                           // Cheer: Randomly does one of 3 cheers, 1 fist raised, 2 fists raised or 2 fists lowered, repeats.
    {
        int rNum = rand() % 3 + 1;                                              // Randomly pick the cheer.
        switch(rNum)
        {
            case 1:                                                             // 1: 1 fist raised
            {
                msg = "1: Unhandled \"1 fist raised\" Cheer emote";
                break;
            }
            case 2:                                                             // 2: 2 fists raised
            {
                msg = "2: Unhandled \"2 fists raised\" Cheer emote";
                break;
            }
            case 3:                                                             // 3: 2 fists lowered
            {
                msg = "3: Unhandled \"2 fists lowered\" Cheer emote";
            }
        }
    }
    else if(lowerContents == "clap")                                            // Clap (has sound): Claps hands several times, crossed arms stance.
        msg = "Unhandled Clap emote";
    else if(coinCommands.contains(lowerContents))                               // Coin: Flips a coin, randomly displays heads or tails, and hold stance. Coin image remains until stance broken.
    {
        int rFlip = rand() % 2 + 1;                                             // Randomly pick heads or tails.
        switch(rFlip)
        {
            case 1:                                                             // 1: Heads
            {
                msg = "1: Unhandled heads Coin emote";
                break;
            }
            case 2:                                                             // 2: Tails
            {
                msg = "2: Unhandled tails Coin emote";
            }
        }
    }
    else if(lowerContents == "crossarms" && !ent->m_motion_state.m_is_flying)                  // CrossArms: Crosses arms, stance (slightly different from most other crossed arm stances).
        msg = "Unhandled CrossArms emote";                                      // Not allowed when flying.
    else if(lowerContents == "dance")                                           // Dance: Randomly performs one of six dances.
        msg = process_random_dance_emote();
    else if(diceCommands.contains(lowerContents))                               // Dice: Picks up, shakes and rolls a die, randomly displays the results (1-6), default stance. Die image quickly fades.
        msg = process_random_dice_emote();
    else if(lowerContents == "dice1")                                           // Dice1: Picks up, shakes and rolls a die, displays a 1, default stance.
        msg = "Unhandled Dice1 emote";
    else if(lowerContents == "dice2")                                           // Dice2: Picks up, shakes and rolls a die, displays a 2, default stance.
        msg = "Unhandled Dice2 emote";
    else if(lowerContents == "dice3")                                           // Dice3: Picks up, shakes and rolls a die, displays a 3, default stance.
        msg = "Unhandled Dice3 emote";
    else if(lowerContents == "dice4")                                           // Dice4: Picks up, shakes and rolls a die, displays a 4, default stance.
        msg = "Unhandled Dice4 emote";
    else if(lowerContents == "dice5")                                           // Dice5: Picks up, shakes and rolls a die, displays a 5, default stance.
        msg = "Unhandled Dice5 emote";
    else if(lowerContents == "dice6")                                           // Dice6: Picks up, shakes and rolls a die, displays a 6, default stance.
        msg = "Unhandled Dice6 emote";
    else if(lowerContents == "disagree")                                        // Disagree: Shakes head, crosses hand in front, then offers an alternative, crossed arms stance.
        msg = "Unhandled Disagree emote";
    else if(lowerContents == "drat")                                            // Drat: Raises fists up, then down, stomping at the same time, same ending stance as Frustrated.
        msg = "Unhandled Drat emote";
    else if(lowerContents == "explain")                                         // Explain: Hold arms out in a "wait a minute" gesture, motion alternatives, then shrug.
        msg = "Unhandled Explain emote";
    else if(evilLaughCommands.contains(lowerContents))                          // EvilLaugh: Extremely melodramatic, overacted evil laugh.
        msg = "Unhandled EvilLaugh emote";
    else if(fancyBowCommands.contains(lowerContents))                           // FancyBow: A much more elegant, ball-room style bow, falls into neutral forward facing stance.
        msg = "Unhandled FancyBow emote";
    else if(flex1Commands.contains(lowerContents))                              // Flex1: Fists raised, flexing arms stance, hold stance. This is called a "double biceps" pose.
        msg = "Unhandled Flex1 emote";
    else if(flex2Commands.contains(lowerContents))                              // Flex2: A side-stance flexing arms, hold stance. This is a sideways variation on the "most muscular" pose.
        msg = "Unhandled Flex2 emote";
    else if(flex3Commands.contains(lowerContents))                              // Flex3: Another side-stance, flexing arms, hold stance. This is an open variation on the "side chest" pose.
        msg = "Unhandled Flex3 emote";
    else if(lowerContents == "frustrated")                                      // Frustrated: Raises both fists and leans backwards, shaking fists and head, leads into a quick-breathing angry-looking stance.
        msg = "Unhandled Frustrated emote";
    else if(lowerContents == "grief")                                           // Grief: Falls to knees, hands on forehead, looks up and gestures a sort of "why me?" look with hands, goes into a sort of depressed slump while on knees, holds stance.
        msg = "Unhandled Grief emote";
    else if(hiCommands.contains(lowerContents))                                 // Hi: Simple greeting wave, fists on hips stance.
        msg = "Unhandled Hi emote";
    else if(hmmCommands.contains(lowerContents))                                // Hmmm: Stare into the sky, rubbing chin, thinking.
        msg = "Unhandled Hmmm emote";
    else if(lowerContents == "jumpingjacks")                                    // JumpingJacks (has sound): Does jumping jacks, repeats.
        msg = "Unhandled JumpingJacks emote";
    else if(lowerContents == "kneel")                                           // Kneel: Quickly kneels on both knees with hands on thighs (looks insanely uncomfortable), holds stance.
        msg = "Unhandled Kneel emote";
    else if(lowerContents == "laugh")                                           // Laugh: Fists on hips, tosses head back and laughs.
        msg = "Unhandled Laugh emote";
    else if(laugh2Commands.contains(lowerContents))                             // Laugh2: Another style of laugh.
        msg = "Unhandled Laugh2 emote";
    else if(lowerContents == "lecture")                                         // Lecture: Waves/shakes hands in different motions in a lengthy lecture, fists on hips stance.
        msg = "Unhandled Lecture emote";
    else if(martialArtsCommands.contains(lowerContents))                        // MartialArts (has sound): Warm up/practice punches and blocks.
        msg = "Unhandled MartialArts emote";
    else if(lowerContents == "militarysalute")                                  // MilitarySalute: Stands in the military-style heads-high hand on forehead salute stance.
        msg = "Unhandled MilitarySalute emote";
    else if(newspaperCommands.contains(lowerContents))                          // Newspaper: Materializes a newspaper and reads it.
        msg = "Unhandled Newspaper emote";
    else if(noCommands.contains(lowerContents))                                 // No: Shakes head and waves hands in front of character, crossed arms stance.
        msg = "Unhandled No emote";
    else if(lowerContents == "nod")                                             // Nod: Fists on hips, nod yes, hold stance.
        msg = "Unhandled Nod emote";
    else if(lowerContents == "none")                                            // None: Cancels the current emote, if any, and resumes default standing animation cycle.
        msg = "Unhandled None emote";
    else if(lowerContents == "paper")                                           // Paper: Plays rock/paper/scissors, picking paper (displays all three symbols for about 6 seconds, then displays and holds your choice until stance is broken).
        msg = "Unhandled Paper emote";
    else if(plotCommands.contains(lowerContents))                               // Plot: Rubs hands together while hunched over.
        msg = "Unhandled Plot emote";
    else if(lowerContents == "point")                                           // Point: Extends left arm and points in direction character is facing, hold stance.
        msg = "Unhandled Point emote";
    else if(lowerContents == "praise")                                          // Praise: Kneel prostrate and repeatedly bow in adoration.
        msg = "Unhandled Praise emote";
    else if(lowerContents == "protest")                                         // Protest: Hold hold up one of several randomly selected mostly unreadable protest signs.
        msg = "Unhandled Protest emote";
    else if(lowerContents == "roar" && !ent->m_motion_state.m_is_flying)                       // Roar: Claws air, roaring, ape-like stance.
        msg = "Unhandled Roar emote";                                           // Not allowed when flying.
    else if(lowerContents == "rock")                                            // Rock: Plays rock/paper/scissors, picking rock (displays all three symbols for about 6 seconds, then displays and holds your choice until stance is broken).
        msg = "Unhandled Rock emote";
    else if(lowerContents == "salute")                                          // Salute: A hand-on-forehead salute, fists on hips stance.
        msg = "Unhandled Salute emote";
    else if(lowerContents == "scissors")                                        // Scissors: Plays rock/paper/scissors, picking scissors (displays all three symbols for about 6 seconds, then displays and holds your choice until stance is broken).
        msg = "Unhandled Scissors emote";
    else if(lowerContents == "score1")                                          // Score1: Holds a black on white scorecard up, displaying a 1, holds stance.
        msg = "Unhandled Score1 emote";
    else if(lowerContents == "score2")                                          // Score2: Holds a black on white scorecard up, displaying a 2, holds stance.
        msg = "Unhandled Score2 emote";
    else if(lowerContents == "score3")                                          // Score3: Holds a black on white scorecard up, displaying a 3, holds stance.
        msg = "Unhandled Score3 emote";
    else if(lowerContents == "score4")                                          // Score4: Holds a black on white scorecard up, displaying a 4, holds stance.
        msg = "Unhandled Score4 emote";
    else if(lowerContents == "score5")                                          // Score5: Holds a black on white scorecard up, displaying a 5, holds stance.
        msg = "Unhandled Score5 emote";
    else if(lowerContents == "score6")                                          // Score6: Holds a black on white scorecard up, displaying a 6, holds stance.
        msg = "Unhandled Score6 emote";
    else if(lowerContents == "score7")                                          // Score7: Holds a black on white scorecard up, displaying a 7, holds stance.
        msg = "Unhandled Score7 emote";
    else if(lowerContents == "score8")                                          // Score8: Holds a black on white scorecard up, displaying a 8, holds stance.
        msg = "Unhandled Score8 emote";
    else if(lowerContents == "score9")                                          // Score9: Holds a black on white scorecard up, displaying a 9, holds stance.
        msg = "Unhandled Score9 emote";
    else if(lowerContents == "score10")                                         // Score10: Holds a black on white scorecard up, displaying a 10, holds stance.
        msg = "Unhandled Score10 emote";
    else if(lowerContents == "shucks")                                          // Shucks: Swings fist and head dejectedly, neutral forward facing stance (not the default stance, same as huh/shrug).
        msg = "Unhandled Shucks emote";
    else if(lowerContents == "sit")                                             // Sit: Sits down, legs forward, with knees bent, elbows on knees, and slightly slumped over, stance.
        msg = "Unhandled Sit emote";
    else if(lowerContents == "smack")                                           // Smack: Backhand slap.
        msg = "Unhandled Smack emote";
    else if(stopCommands.contains(lowerContents))                               // Stop: Raises your right hand above your head, hold stance.
        msg = "Unhandled Stop emote";
    else if(tarzanCommands.contains(lowerContents))                             // Tarzan: Beats chest and howls, angry-looking stance.
    {
        if(ent->m_motion_state.m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Tarzan emote";
        else
            msg = "Unhandled ground Tarzan emote";
    }
    else if(taunt1Commands.contains(lowerContents))                             // Taunt1: Taunts, beckoning with one hand, then slaps fist into palm, repeating stance.

    {
        if(ent->m_motion_state.m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Taunt1 emote";
        else
            msg = "Unhandled ground Taunt1 emote";
    }
    else if(taunt2Commands.contains(lowerContents))                             // Taunt2: Taunts, beckoning with both hands, combat stance.
    {
        if(ent->m_motion_state.m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Taunt2 emote";
        else
            msg = "Unhandled ground Taunt2 emote";
    }
    else if(thanksCommands.contains(lowerContents))                             // Thanks: Gestures with hand, neutral forward facing stance.
        msg = "Unhandled Thanks emote";
    else if(lowerContents == "thewave")                                         // Thewave: Does the wave (as seen in stadiums at sporting events), neutral facing forward stance.
        msg = "Unhandled Thewave emote";
    else if(lowerContents == "victory")                                         // Victory: Raises hands excitedly, and then again less excitedly, and then a third time almost non-chalantly, falls into neutral forward facing stance.
        msg = "Unhandled Victory emote";
    else if(waveFistCommands.contains(lowerContents))                           // WaveFist (has sound): Waves fist, hoots and then claps (its a cheer), crossed arms stance.
        msg = "Unhandled WaveFist emote";
    else if(lowerContents == "welcome")                                         // Welcome: Open arms welcoming, fists on hips stance.
        msg = "Unhandled Welcome emote";
    else if(lowerContents == "whistle")                                         // Whistle (has sound): Whistles (sounds like a police whistle), ready-stance.
        msg = "Unhandled Whistle emote";
    else if(winnerCommands.contains(lowerContents))                             // Winner: Fist in fist cheer, right, and then left, neutral forward facing stance.
        msg = "Unhandled Winner emote";
    else if(lowerContents == "yourewelcome")                                    // YoureWelcome: Bows head and gestures with hand, neutral forward facing stance.
        msg = "Unhandled YoureWelcome emote";
    else if(yesCommands.contains(lowerContents))                                // Yes: Big (literally) thumbs up and an affirmative nod, fists on hips stance.
        msg = "Unhandled Yes emote";
    else if(yogaCommands.contains(lowerContents))                               // Yoga: Sits down cross legged with hands on knees/legs, holds stance.
    {
        if(ent->m_motion_state.m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Yoga emote";
        else
            msg = "Unhandled ground Yoga emote";
    }
                                                                                // Boombox Emotes
    else if(lowerContents.startsWith("bb") && !ent->m_motion_state.m_is_flying)                // Check if Boombox Emote.
    {                                                                           // Not allowed when flying.
        lowerContents.replace(0, 2, "");                                        // Remove the "BB" prefix for conciseness.
        if(lowerContents == "altitude")                                         // BBAltitude
            msg = "Unhandled BBAltitude emote";
        else if(lowerContents == "beat")                                        // BBBeat
            msg = "Unhandled BBBeat emote";
        else if(lowerContents == "catchme")                                     // BBCatchMe
            msg = "Unhandled BBCatchMe emote";
        else if(lowerContents == "dance")                                       // BBDance
            msg = "Unhandled BBDance emote";
        else if(lowerContents == "discofreak")                                  // BBDiscoFreak
            msg = "Unhandled BBDiscoFreak emote";
        else if(lowerContents == "dogwalk")                                     // BBDogWalk
            msg = "Unhandled BBDogWalk emote";
        else if(lowerContents == "electrovibe")                                 // BBElectroVibe
            msg = "Unhandled BBElectroVibe emote";
        else if(lowerContents == "heavydude")                                   // BBHeavyDude
            msg = "Unhandled BBHeavyDude emote";
        else if(lowerContents == "infooverload")                                // BBInfoOverload
            msg = "Unhandled BBInfoOverload emote";
        else if(lowerContents == "jumpy")                                       // BBJumpy
            msg = "Unhandled BBJumpy emote";
        else if(lowerContents == "kickit")                                      // BBKickIt
            msg = "Unhandled BBKickIt emote";
        else if(lowerContents == "looker")                                      // BBLooker
            msg = "Unhandled BBLooker emote";
        else if(lowerContents == "meaty")                                       // BBMeaty
            msg = "Unhandled BBMeaty emote";
        else if(lowerContents == "moveon")                                      // BBMoveOn
            msg = "Unhandled BBMoveOn emote";
        else if(lowerContents == "notorious")                                   // BBNotorious
            msg = "Unhandled BBNotorious emote";
        else if(lowerContents == "peace")                                       // BBPeace
            msg = "Unhandled BBPeace emote";
        else if(lowerContents == "quickie")                                     // BBQuickie
            msg = "Unhandled BBQuickie emote";
        else if(lowerContents == "raver")                                       // BBRaver
            msg = "Unhandled BBRaver emote";
        else if(lowerContents == "shuffle")                                     // BBShuffle
            msg = "Unhandled BBShuffle emote";
        else if(lowerContents == "spaz")                                        // BBSpaz
            msg = "Unhandled BBSpaz emote";
        else if(lowerContents == "technoid")                                    // BBTechnoid
            msg = "Unhandled BBTechnoid emote";
        else if(lowerContents == "venus")                                       // BBVenus
            msg = "Unhandled BBVenus emote";
        else if(lowerContents == "winditup")                                    // BBWindItUp
            msg = "Unhandled BBWindItUp emote";
        else if(lowerContents == "wahwah")                                      // BBWahWah
            msg = "Unhandled BBWahWah emote";
        else if(lowerContents == "yellow")                                      // BBYellow
            msg = "Unhandled BBYellow emote";
    }
                                                                                // Unlockable Emotes
                                                                                // TODO: Implement logic and variables for unlocking these emotes.
    else if(lowerContents == "dice7")                                           // Dice7: Picks up, shakes and rolls a die, displays a 7, default stance.
        msg = "Unhandled Dice7 emote";                                          // Unlocked by earning the Burkholder's Bane Badge (from the Ernesto Hess Task Force).
    else if(lowerContents == "listenpoliceband")                                // ListenPoliceBand: Listens in on the heroes' PPD police band radio.
        msg = "Unhandled ListenPoliceBand emote";                               // Heroes can use this without any unlock requirement. For villains, ListenStolenPoliceBand unlocks by earning the Outlaw Badge.
    else if(snowflakesCommands.contains(lowerContents))                         // Snowflakes: Throws snowflakes.
    {
        if(ent->m_motion_state.m_is_flying)                                                    // Different versions when flying and on the ground.
            msg = "Unhandled flying Snowflakes emote";                          // Unlocked by purchasing from the Candy Keeper during the Winter Event.
        else
            msg = "Unhandled ground Snowflakes emote";
    }
    else                                                                        // If not specific command, output EMOTE message.
    {
        // "CharacterName {emote message}"
        msg = QString("%1 %2").arg(ent->name(),original_emote);
    }

    // send only to clients within range
    glm::vec3 senderpos = ent->m_entity_data.m_pos;
    std::vector<Entity*> entities;
    for (Entity *e : ref_entity_mgr.m_live_entlist)
    {
        // is NPC, no need to chat to them right?
        if (e->m_client == nullptr)
            continue;

        glm::vec3 recpos = e->m_entity_data.m_pos;
        float range = 50.0f; // range of "hearing". I assume this is in yards
        float dist = glm::distance(senderpos,recpos);

        qCDebug(logEmotes, "senderpos: %f %f %f", senderpos.x, senderpos.y, senderpos.z);
        qCDebug(logEmotes, "recpos: %f %f %f", recpos.x, recpos.y, recpos.z);
        qCDebug(logEmotes, "sphere: %f", range);
        qCDebug(logEmotes, "dist: %f", dist);

        if(dist <= range)
            entities.emplace_back(e);
    }

    qCDebug(logEmotes) << msg;
    return std::make_unique<ChatServiceToClientData>(ent, entities, msg, MessageChannel::EMOTE);
}

QString ChatService::process_random_dice_emote()
{
    int rDice = rand() % 6 + 1;                                             // Randomly pick a die result.
    switch(rDice)
    {
        case 1: return "1: Unhandled \"1\" Dice emote";
        case 2: return "2: Unhandled \"2\" Dice emote";
        case 3: return "3: Unhandled \"3\" Dice emote";
        case 4: return "4: Unhandled \"4\" Dice emote";
        case 5: return "5: Unhandled \"5\" Dice emote";
        case 6: return "6: Unhandled \"6\" Dice emote";
    }

    return QString();
}

QString ChatService::process_random_dance_emote()
{
    int rDance = rand() % 6 + 1;                                            // Randomly pick the dance.
    switch(rDance)
    {
        case 1: return "1: Unhandled \"Dances with elbows by hips\" Dance emote";
        case 2: return "2: Unhandled \"Dances with fists raised\" Dance emote";
        case 3: return "3: Unhandled \"Swaying hands by hips, aka 'Really Bad' dancing\" Dance emote";
        case 4: return "4: Unhandled \"Swaying hands up in the air, like in a breeze\" Dance emote";
        case 5: return "5: Unhandled \"As Dance4, but jumping as well\" Dance emote";
        case 6: return "6: Unhandled \"The monkey\" Dance emote";
    }

    return QString();
}

QString ChatService::process_random_boombox_emote()
{
    int rSong = rand() % 25 + 1;                                            // Randomly pick a song.
    switch(rSong)
    {
        case 1: return "1: Unhandled \"BBAltitude\" BoomBox emote";
        case 2: return "2: Unhandled \"BBBeat\" BoomBox emote";
        case 3: return "3: Unhandled \"BBCatchMe\" BoomBox emote";
        case 4: return "4: Unhandled \"BBDance\" BoomBox emote";
        case 5: return "5: Unhandled \"BBDiscoFreak\" BoomBox emote";
        case 6: return "6: Unhandled \"BBDogWalk\" BoomBox emote";
        case 7: return "7: Unhandled \"BBElectroVibe\" BoomBox emote";
        case 8: return "8: Unhandled \"BBHeavyDude\" BoomBox emote";
        case 9: return "9: Unhandled \"BBInfoOverload\" BoomBox emote";
        case 10: return "10: Unhandled \"BBJumpy\" BoomBox emote";
        case 11: return "11: Unhandled \"BBKickIt\" BoomBox emote";
        case 12: return "12: Unhandled \"BBLooker\" BoomBox emote";
        case 13: return "13: Unhandled \"BBMeaty\" BoomBox emote";
        case 14: return "14: Unhandled \"BBMoveOn\" BoomBox emote";
        case 15: return "15: Unhandled \"BBNotorious\" BoomBox emote";
        case 16: return "16: Unhandled \"BBPeace\" BoomBox emote";
        case 17: return "17: Unhandled \"BBQuickie\" BoomBox emote";
        case 18: return "18: Unhandled \"BBRaver\" BoomBox emote";
        case 19: return "19: Unhandled \"BBShuffle\" BoomBox emote";
        case 20: return "20: Unhandled \"BBSpaz\" BoomBox emote";
        case 21: return "21: Unhandled \"BBTechnoid\" BoomBox emote";
        case 22: return "22: Unhandled \"BBVenus\" BoomBox emote";
        case 23: return "23: Unhandled \"BBWindItUp\" BoomBox emote";
        case 24: return "24: Unhandled \"BBWahWah\" BoomBox emote";
        case 25: return "25: Unhandled \"BBYellow\" BoomBox emote";
    }

    return QString();
}

QString ChatService::process_replacement_strings(Entity *ent, const QString &msg_text)
{
    /*
    // $$           - newline
    // $archetype   - the archetype of your character
    // $battlecry   - your character's battlecry, as entered on your character ID screen
    // $level       - your character's current level
    // $name        - your character's name
    // $origin      - your character's origin
    // $target      - your currently selected target's name

    msg_text = msg_text.replace("$target",sender->m_ent->target->name());
    */

    QString new_msg = msg_text;
    static const QStringList replacements = {
        "\\$\\$",
        "\\$archetype",
        "\\$battlecry",
        "\\$level",
        "\\$name",
        "\\$origin",
        "\\$target"
    };

    const Character &c(*ent->m_char);

    QString  sender_class       = QString(getClass(c)).remove("Class_");
    QString  sender_battlecry   = getBattleCry(c);
    uint32_t sender_level       = getLevel(c);
    QString  sender_char_name   = c.getName();
    QString  sender_origin      = getOrigin(c);
    uint32_t target_idx         = getTargetIdx(*ent);
    QString  target_char_name   = c.getName();

    qCDebug(logChat) << "src -> tgt: " << ent->m_idx  << "->" << target_idx;

    if(target_idx > 0)
    {
        Entity   *tgt    = getEntity(ent->m_client, target_idx);
        target_char_name = tgt->name(); // change name
    }

    foreach (const QString &str, replacements)
    {
        if(str == "\\$archetype")
            new_msg.replace(QRegularExpression(str), sender_class);
        else if(str == "\\$battlecry")
            new_msg.replace(QRegularExpression(str), sender_battlecry);
        else if(str == "\\$level")
            new_msg.replace(QRegularExpression(str), QString::number(sender_level));
        else if(str == "\\$name")
            new_msg.replace(QRegularExpression(str), sender_char_name);
        else if(str == "\\$origin")
            new_msg.replace(QRegularExpression(str), sender_origin);
        else if(str == "\\$target")
            new_msg.replace(QRegularExpression(str), target_char_name);
        else if(str == "\\$\\$")
        {
            if(new_msg.contains(str))
                qCDebug(logChat) << "need to send newline for" << str; // This apparently works client-side.
        }
    }
    return new_msg;
}

bool ChatService::isChatMessage(const QString &msg)
{
    static const QStringList chat_prefixes = {
            "l", "local",
            "b", "broadcast", "y", "yell",
            "g", "group", "sg", "supergroup",
            "req", "request",
            "f",
            "t", "tell", "w", "whisper", "p", "private"
    };
    QString space(msg.mid(0,msg.indexOf(' ')));
    return chat_prefixes.contains(space);
}

static MessageChannel getKindOfChatMessage(const QStringRef &msg)
{
    if(msg=="l" || msg=="local")                                                            // Aliases: local, l
        return MessageChannel::LOCAL;
    if(msg=="b" || msg=="broadcast" || msg=="y" || msg=="yell")                             // Aliases: broadcast, yell, b, y
        return MessageChannel::BROADCAST;
    if(msg=="g" || msg=="group" || msg=="team")                                             // Aliases: team, g, group
        return MessageChannel::TEAM;
    if(msg=="sg" || msg=="supergroup")                                                      // Aliases: sg, supergroup
        return MessageChannel::SUPERGROUP;
    if(msg=="req" || msg=="request" || msg=="auction" || msg=="sell")                       // Aliases: request, req, auction, sell
        return MessageChannel::REQUEST;
    if(msg=="f")                                                                            // Aliases: f
        return MessageChannel::FRIENDS;
    if(msg=="t" || msg=="tell" || msg=="w" || msg=="whisper" || msg=="p" || msg=="private") // Aliases: t, tell, whisper, w, private, p
        return MessageChannel::PRIVATE;
    // unknown chat types are processed as local chat
    return MessageChannel::LOCAL;
}

bool ChatService::has_emote_prefix(const QString &cmd) // ERICEDIT: This encompasses all emotes.
{
    return cmd.startsWith("em ",Qt::CaseInsensitive) || cmd.startsWith("e ",Qt::CaseInsensitive)
                || cmd.startsWith("me ",Qt::CaseInsensitive) || cmd.startsWith("emote ",Qt::CaseInsensitive);
}

std::unique_ptr<ChatServiceToClientData> ChatService::process_chat(Entity *sender, QString &msg_text)
{
    int first_space = msg_text.indexOf(QRegularExpression("\\s"), 0); // first whitespace, as the client sometimes sends tabs
    QStringRef cmd_str(msg_text.midRef(0,first_space));
    MessageChannel kind = getKindOfChatMessage(cmd_str);

    if(sender == nullptr || sender->m_client == nullptr)
        return nullptr;

    switch(kind)
    {
        case MessageChannel::LOCAL:
            return process_local_chat(sender, msg_text);
        case MessageChannel::BROADCAST:
            return process_broadcast_chat(sender, msg_text);
        case MessageChannel::REQUEST:
            return process_request_chat(sender, msg_text);
        case MessageChannel::PRIVATE:
            return process_private_chat(sender, msg_text);
        case MessageChannel::TEAM:
            return process_team_chat(sender, msg_text);
        case MessageChannel::SUPERGROUP:
            return process_supergroup_chat(sender, msg_text);
        case MessageChannel::FRIENDS:
            return process_friends_chat(sender, msg_text);
        default:
        {
            qCDebug(logChat) << "Unhandled MessageChannel type" << int(kind);
            break;
        }
    }

    return nullptr;
}

std::unique_ptr<ChatServiceToClientData> ChatService::process_local_chat(Entity *sender, QString &msg_text)
{
    // send only to clients within range
    glm::vec3 senderpos = sender->m_entity_data.m_pos;
    std::vector<Entity*> entities;
    for (Entity *e : ref_entity_mgr.m_live_entlist)
    {
        // is NPC, no need to chat to them right?
        if (e->m_client == nullptr)
            continue;

        glm::vec3 recpos = e->m_entity_data.m_pos;
        float range = 50.0f; // range of "hearing". I assume this is in yards
        float dist = glm::distance(senderpos,recpos);

        qCDebug(logEmotes, "senderpos: %f %f %f", senderpos.x, senderpos.y, senderpos.z);
        qCDebug(logEmotes, "recpos: %f %f %f", recpos.x, recpos.y, recpos.z);
        qCDebug(logEmotes, "sphere: %f", range);
        qCDebug(logEmotes, "dist: %f", dist);

        if(dist <= range)
            entities.emplace_back(e);
    }

    QString prepared_chat_message = get_prepared_chat_message(sender->name(), msg_text);
    return std::make_unique<ChatServiceToClientData>(sender, entities, prepared_chat_message, MessageChannel::LOCAL);
}

std::unique_ptr<ChatServiceToClientData> ChatService::process_broadcast_chat(Entity *sender, QString &msg_text)
{
    // send the message to everyone on this map
    std::vector<Entity*> entities;
    for (Entity *e : ref_entity_mgr.m_live_entlist)
    {
        // is NPC, no need to chat to them right?
        if (e->m_client == nullptr)
            continue;

        entities.emplace_back(e);
    }

    QString prepared_chat_message = get_prepared_chat_message(sender->name(), msg_text);
    return std::make_unique<ChatServiceToClientData>(sender, entities, prepared_chat_message, MessageChannel::BROADCAST);
}

std::unique_ptr<ChatServiceToClientData> ChatService::process_request_chat(Entity *sender, QString &msg_text)
{
    // Essentially a carbon copy of process_broadcast_chat, only returning a different message channel

    // send the message to everyone on this map
    std::vector<Entity*> entities;
    for (Entity *e : ref_entity_mgr.m_live_entlist)
    {
        // is NPC, no need to chat to them right?
        if (e->m_client == nullptr)
            continue;

        entities.emplace_back(e);
    }

    QString prepared_chat_message = get_prepared_chat_message(sender->name(), msg_text);
    return std::make_unique<ChatServiceToClientData>(sender, entities, prepared_chat_message, MessageChannel::REQUEST);
}

std::unique_ptr<ChatServiceToClientData> ChatService::process_private_chat(Entity *sender, QString &msg_text)
{
    int first_comma = msg_text.indexOf(',');
    int first_space = msg_text.indexOf(QRegularExpression("\\s"), 0);
    QStringRef target_name_ref(msg_text.midRef(first_space+1,(first_comma - first_space-1)));
    QStringRef msg_content = msg_text.midRef(first_comma+1,msg_text.lastIndexOf("\n"));
    QString prepared_chat_message;

    QString target_name = target_name_ref.toString();
    qCDebug(logChat) << "Private Chat:"
                     << "\n\t" << "target_name:" << target_name
                     << "\n\t" << "msg_text:" << msg_text;

    Entity *tgt;

    // what happens if we give an NPC's name...?
    for (Entity *e : ref_entity_mgr.m_live_entlist)
    {
        // is NPC, no need to chat to them right?
        if (e->m_client == nullptr)
            continue;

        if (e->name() == target_name)
        {
            tgt = e;
            break;
        }
    }

    qWarning() << "Private chat: this only work for players on local server. We should introduce a message router, and send messages to EntityIDs instead of directly using sessions.";

    QString message_to_self;
    if(tgt == nullptr)
    {
        message_to_self = QString("No player named \"%1\" currently online.").arg(target_name);
        return std::make_unique<ChatServiceToClientData>(sender, message_to_self, MessageChannel::USER_ERROR);
    }
    else
    {
        // sends to both self (using sendInfoMessage) and target (using sendChatMessage)
        message_to_self = QString(" --> %1: %2").arg(target_name,msg_content.toString());
        prepared_chat_message = QString(" %1: %2").arg(sender->name(), msg_content.toString());
    }

    std::vector<Entity*> entities;
    entities.emplace_back(tgt);

    return std::make_unique<ChatServiceToClientData>(sender, entities, prepared_chat_message, MessageChannel::PRIVATE, message_to_self, MessageChannel::PRIVATE);
}

std::unique_ptr<ChatServiceToClientData> ChatService::process_team_chat(Entity *sender, QString &msg_text)
{
    if(!sender->m_has_team)
    {
        QString message_to_self = "You are not a member of a Team.";
        return std::make_unique<ChatServiceToClientData>(sender, message_to_self, MessageChannel::USER_ERROR);
    }

    qWarning() << "Team chat: this only work for members on local server. We should introduce a message router, and send messages to EntityIDs instead of directly using sessions.";

    // Only send the message to characters on sender's team
    std::vector<Entity*> entities;
    for (Entity *e : ref_entity_mgr.m_live_entlist)
    {
        // is NPC, no need to chat to them right?
        if (e->m_client == nullptr)
            continue;

        if (sender->m_team->m_team_idx == e->m_team->m_team_idx)
            entities.emplace_back(e);
    }

    QString prepared_chat_message = get_prepared_chat_message(sender->name(), msg_text);
    return std::make_unique<ChatServiceToClientData>(sender, entities, prepared_chat_message, MessageChannel::TEAM);
}

std::unique_ptr<ChatServiceToClientData> ChatService::process_supergroup_chat(Entity *sender, QString &msg_text)
{
    if(!sender->m_has_supergroup)
    {
        QString message_to_self = "You are not a member of a SuperGroup.";
        return std::make_unique<ChatServiceToClientData>(sender, message_to_self, MessageChannel::USER_ERROR);
    }

    qWarning() << "SuperGroup chat: this only work for members on local server. We should introduce a message router, and send messages to EntityIDs instead of directly using sessions.";

    // Only send the message to characters in sender's supergroup
    std::vector<Entity*> entities;
    for (Entity *e : ref_entity_mgr.m_live_entlist)
    {
        // is NPC, no need to chat to them right?
        if (e->m_client == nullptr)
            continue;

        if (sender->m_supergroup.m_SG_id == e->m_supergroup.m_SG_id)
            entities.emplace_back(e);
    }

    QString prepared_chat_message = get_prepared_chat_message(sender->name(), msg_text);
    return std::make_unique<ChatServiceToClientData>(sender, entities, prepared_chat_message, MessageChannel::SUPERGROUP);
}

std::unique_ptr<ChatServiceToClientData> ChatService::process_friends_chat(Entity *sender, QString &msg_text)
{
    FriendsList * fl = &sender->m_char->m_char_data.m_friendlist;

    if(!fl->m_has_friends || fl->m_friends_count == 0)
    {
        QString message_to_self = "You don't have any friends to message.";
        return std::make_unique<ChatServiceToClientData>(sender, message_to_self, MessageChannel::USER_ERROR);
    }

    qWarning() << "Friend chat: this only work for friends on local server. We should introduce a message router, and send messages to EntityIDs instead of directly using sessions.";

    // Only send the message to characters in sender's friendslist
    std::vector<uint32_t> db_ids;
    for(Friend &f : fl->m_friends)
    {
        if(f.m_online_status != true)
            continue;

        db_ids.push_back(f.m_db_id);
    }

    //TODO: this only work for friends on local server
    // introduce a message router, and send messages to EntityIDs instead of directly using sessions.
    std::vector<Entity*> entities;
    for (Entity *e : ref_entity_mgr.m_live_entlist)
    {
        // is NPC, no need to chat to them right?
        if (e->m_client == nullptr)
            continue;

        for (uint32_t id : db_ids)
        {
            if (e->m_db_id == id)
                entities.emplace_back(e);
        }
    }

    // sending to self too...?
    //sendChatMessage(MessageChannel::FRIENDS, prepared_chat_message, sender, *sender_sess);
    entities.emplace_back(sender);

    QString prepared_chat_message = get_prepared_chat_message(sender->name(), msg_text);
    return std::make_unique<ChatServiceToClientData>(sender, entities, prepared_chat_message, MessageChannel::FRIENDS);
}

QString ChatService::get_prepared_chat_message(const QString& name, const QString& msg_text)
{
    int first_space = msg_text.indexOf(QRegularExpression("\\s"), 0);
    QStringRef msg_content(msg_text.midRef(first_space+1,msg_text.lastIndexOf("\n")));
    return QString("[Local] %1: %2").arg(name, msg_content.toString());
}


