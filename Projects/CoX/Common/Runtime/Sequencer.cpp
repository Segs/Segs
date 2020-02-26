#include "Sequencer.h"

#include "Animation.h"
#include "AnimationEngine.h"
#include "Model.h"
#include "Prefab.h"
#include "SceneGraph.h"

#include "GameData/GameDataStore.h"
#include "GameData/seq_definitions.h"
#include "GameData/seq_serializers.h"
#include <QDebug>
#include <QDir>
#include <QMetaEnum>
#include <glm/common.hpp>
#include <glm/ext.hpp>
#include <set>

using namespace SEGS;
namespace
{
QHash<QString, EntitySequencerData *> s_seq_types;
struct TranslatedMove
{
    std::set<uint32_t> irqs; // hash of a Interrupt string representation
    std::set<uint32_t> members;
};
enum ePredictable : bool // used to 'name' bool values
{
    NOT_PREDICTABLE = false,
    PREDICTABLE     = true,
};
struct SeqMetaData
{
    const char * name;
    SeqBitNames  bitname;
    ePredictable predictable;
};
const SeqMetaData seq_data[353] = {
    {"", SeqBitNames::INVALID_BIT, NOT_PREDICTABLE},
    {"FORWARD", SeqBitNames::FORWARD, PREDICTABLE},
    {"BACKWARD", SeqBitNames::BACKWARD, PREDICTABLE},
    {"STEPLEFT", SeqBitNames::STEPLEFT, PREDICTABLE},
    {"STEPRIGHT", SeqBitNames::STEPRIGHT, PREDICTABLE},
    {"JUMP", SeqBitNames::JUMP, PREDICTABLE},
    {"AIR", SeqBitNames::AIR, PREDICTABLE},
    {"APEX", SeqBitNames::APEX, PREDICTABLE},
    {"FLY", SeqBitNames::FLY, PREDICTABLE},
    {"BIGFALL", SeqBitNames::BIGFALL, PREDICTABLE},
    {"SWIM", SeqBitNames::SWIM, PREDICTABLE},
    {"PROFILE", SeqBitNames::PROFILE, PREDICTABLE},
    {"IDLE", SeqBitNames::IDLE, PREDICTABLE},
    {"TURNRIGHT", SeqBitNames::TURNRIGHT, PREDICTABLE},
    {"TURNLEFT", SeqBitNames::TURNLEFT, PREDICTABLE},
    {"COMBAT", SeqBitNames::COMBAT, PREDICTABLE},
    {"WEAPON", SeqBitNames::WEAPON, PREDICTABLE},
    {"GUN", SeqBitNames::GUN, PREDICTABLE},
    {"KUNGFU", SeqBitNames::KUNGFU, PREDICTABLE},
    {"CLUB", SeqBitNames::CLUB, PREDICTABLE},
    {"ICESLIDE", SeqBitNames::ICESLIDE, PREDICTABLE},
    {"CARRY", SeqBitNames::CARRY, PREDICTABLE},
    {"SNEAK", SeqBitNames::SNEAK, PREDICTABLE},
    {"ARMCHAIR", SeqBitNames::ARMCHAIR, NOT_PREDICTABLE},
    {"BARSTOOL", SeqBitNames::BARSTOOL, NOT_PREDICTABLE},
    {"SIT", SeqBitNames::SIT, NOT_PREDICTABLE},
    {"CLAW", SeqBitNames::CLAW, NOT_PREDICTABLE},
    {"STUN", SeqBitNames::STUN, PREDICTABLE},
    {"SLEEP", SeqBitNames::SLEEP, NOT_PREDICTABLE},
    {"SWEEP", SeqBitNames::SWEEP, NOT_PREDICTABLE},
    {"LAUNCH", SeqBitNames::LAUNCH, NOT_PREDICTABLE},
    {"PREMODE", SeqBitNames::PREMODE, NOT_PREDICTABLE},
    {"GRAVEL", SeqBitNames::GRAVEL, NOT_PREDICTABLE},
    {"ROCK", SeqBitNames::ROCK, NOT_PREDICTABLE},
    {"TILE", SeqBitNames::TILE, NOT_PREDICTABLE},
    {"LAVA", SeqBitNames::LAVA, NOT_PREDICTABLE},
    {"GRASS", SeqBitNames::GRASS, NOT_PREDICTABLE},
    {"CONCRETE", SeqBitNames::CONCRETE, NOT_PREDICTABLE},
    {"MUD", SeqBitNames::MUD, NOT_PREDICTABLE},
    {"DIRT", SeqBitNames::DIRT, NOT_PREDICTABLE},
    {"PUDDLE", SeqBitNames::PUDDLE, NOT_PREDICTABLE},
    {"METAL", SeqBitNames::METAL, NOT_PREDICTABLE},
    {"WOOD", SeqBitNames::WOOD, NOT_PREDICTABLE},
    {"INDEEP", SeqBitNames::INDEEP, NOT_PREDICTABLE},
    {"INWATER", SeqBitNames::INWATER, NOT_PREDICTABLE},
    {"INLAVA", SeqBitNames::INLAVA, NOT_PREDICTABLE},
    {"PREDICTABLE", SeqBitNames::PREDICTABLE, NOT_PREDICTABLE},
    {"NOCOLLISION", SeqBitNames::NOCOLLISION, NOT_PREDICTABLE},
    {"WALKTHROUGH", SeqBitNames::WALKTHROUGH, NOT_PREDICTABLE},
    {"CANTMOVE", SeqBitNames::CANTMOVE, NOT_PREDICTABLE},
    {"HIDE", SeqBitNames::HIDE, NOT_PREDICTABLE},
    {"DIENOW", SeqBitNames::DIENOW, NOT_PREDICTABLE},
    {"TARGET", SeqBitNames::TARGET, NOT_PREDICTABLE},
    {"JUSTFIDDLESTICKINDOIT", SeqBitNames::JUSTFIDDLESTICKINDOIT,
     NOT_PREDICTABLE}, // thankfully this is not used, so we don't have to use cuss-words :P
    {"VARIATIONA", SeqBitNames::VARIATIONA, NOT_PREDICTABLE},
    {"VARIATIONB", SeqBitNames::VARIATIONB, NOT_PREDICTABLE},
    {"VARIATIONC", SeqBitNames::VARIATIONC, NOT_PREDICTABLE},
    {"FAR", SeqBitNames::FAR, NOT_PREDICTABLE},
    {"CLOSE", SeqBitNames::CLOSE, NOT_PREDICTABLE},
    {"VERYCLOSE", SeqBitNames::VERYCLOSE, NOT_PREDICTABLE},
    {"WALK", SeqBitNames::WALK, NOT_PREDICTABLE},
    {"RUN", SeqBitNames::RUN, NOT_PREDICTABLE},
    {"FRONT", SeqBitNames::FRONT, NOT_PREDICTABLE},
    {"BACK", SeqBitNames::BACK, NOT_PREDICTABLE},
    {"RIGHT", SeqBitNames::RIGHT, NOT_PREDICTABLE},
    {"LEFT", SeqBitNames::LEFT, NOT_PREDICTABLE},
    {"HIGH", SeqBitNames::HIGH, NOT_PREDICTABLE},
    {"MID", SeqBitNames::MID, NOT_PREDICTABLE},
    {"LOW", SeqBitNames::LOW, NOT_PREDICTABLE},
    {"QUICK", SeqBitNames::QUICK, NOT_PREDICTABLE},
    {"STRONG", SeqBitNames::STRONG, NOT_PREDICTABLE},
    {"FIERCE", SeqBitNames::FIERCE, NOT_PREDICTABLE},
    {"HIT", SeqBitNames::HIT, NOT_PREDICTABLE},
    {"DEATH", SeqBitNames::DEATH, NOT_PREDICTABLE},
    {"PUNCH", SeqBitNames::PUNCH, NOT_PREDICTABLE},
    {"KICK", SeqBitNames::KICK, NOT_PREDICTABLE},
    {"HAND", SeqBitNames::HAND, NOT_PREDICTABLE},
    {"SWORD", SeqBitNames::SWORD, NOT_PREDICTABLE},
    {"SHIELD", SeqBitNames::SHIELD, NOT_PREDICTABLE},
    {"HEAD", SeqBitNames::HEAD, NOT_PREDICTABLE},
    {"FEET", SeqBitNames::FEET, NOT_PREDICTABLE},
    {"FLAME", SeqBitNames::FLAME, NOT_PREDICTABLE},
    {"FIRE", SeqBitNames::FIRE, NOT_PREDICTABLE},
    {"ENERGY", SeqBitNames::ENERGY, NOT_PREDICTABLE},
    {"ICE", SeqBitNames::ICE, NOT_PREDICTABLE},
    {"WINGS", SeqBitNames::WINGS, NOT_PREDICTABLE},
    {"JETPACK", SeqBitNames::JETPACK, PREDICTABLE},
    {"HOVER", SeqBitNames::HOVER, NOT_PREDICTABLE},
    {"EYE", SeqBitNames::EYE, NOT_PREDICTABLE},
    {"CHEST", SeqBitNames::CHEST, NOT_PREDICTABLE},
    {"BACKPACK", SeqBitNames::BACKPACK, NOT_PREDICTABLE},
    {"ALTREADY", SeqBitNames::ALTREADY, NOT_PREDICTABLE},
    {"2HAND", SeqBitNames::TWO_HAND, NOT_PREDICTABLE},
    {"ATTACK", SeqBitNames::ATTACK, NOT_PREDICTABLE},
    {"RADIUS", SeqBitNames::RADIUS, NOT_PREDICTABLE},
    {"ELEMENT", SeqBitNames::ELEMENT, NOT_PREDICTABLE},
    {"MAGIC", SeqBitNames::MAGIC, NOT_PREDICTABLE},
    {"KNOCKBACK", SeqBitNames::KNOCKBACK, NOT_PREDICTABLE},
    {"BLOCK", SeqBitNames::BLOCK, NOT_PREDICTABLE},
    {"LIMP", SeqBitNames::LIMP, NOT_PREDICTABLE},
    {"BLADE", SeqBitNames::BLADE, NOT_PREDICTABLE},
    {"BLUNT", SeqBitNames::BLUNT, NOT_PREDICTABLE},
    {"HANDGUN", SeqBitNames::HANDGUN, NOT_PREDICTABLE},
    {"KNOCKDOWN", SeqBitNames::KNOCKDOWN, NOT_PREDICTABLE},
    {"STARTOFFENSESTANCE", SeqBitNames::STARTOFFENSESTANCE, NOT_PREDICTABLE},
    {"STARTDEFENSESTANCE", SeqBitNames::STARTDEFENSESTANCE, NOT_PREDICTABLE},
    {"TEST0", SeqBitNames::TEST0, NOT_PREDICTABLE},
    {"TEST1", SeqBitNames::TEST1, NOT_PREDICTABLE},
    {"TEST2", SeqBitNames::TEST2, NOT_PREDICTABLE},
    {"TEST3", SeqBitNames::TEST3, NOT_PREDICTABLE},
    {"TEST4", SeqBitNames::TEST4, NOT_PREDICTABLE},
    {"TEST5", SeqBitNames::TEST5, NOT_PREDICTABLE},
    {"TEST6", SeqBitNames::TEST6, NOT_PREDICTABLE},
    {"TEST7", SeqBitNames::TEST7, NOT_PREDICTABLE},
    {"TEST8", SeqBitNames::TEST8, NOT_PREDICTABLE},
    {"TEST9", SeqBitNames::TEST9, NOT_PREDICTABLE},
    {"COMBO2", SeqBitNames::COMBO2, NOT_PREDICTABLE},
    {"COMBO3", SeqBitNames::COMBO3, NOT_PREDICTABLE},
    {"SPEED", SeqBitNames::SPEED, NOT_PREDICTABLE},
    {"SPELLCAST", SeqBitNames::SPELLCAST, NOT_PREDICTABLE},
    {"RUMBLEA", SeqBitNames::RUMBLEA, NOT_PREDICTABLE},
    {"RUMBLEB", SeqBitNames::RUMBLEB, NOT_PREDICTABLE},
    {"PROP", SeqBitNames::PROP, NOT_PREDICTABLE},
    {"UNYIELD", SeqBitNames::UNYIELD, NOT_PREDICTABLE},
    {"CHOKESTUN", SeqBitNames::CHOKESTUN, NOT_PREDICTABLE},
    {"SONICSTUN", SeqBitNames::SONICSTUN, NOT_PREDICTABLE},
    {"BLINDSTUN", SeqBitNames::BLINDSTUN, NOT_PREDICTABLE},
    {"LASSOSTUN", SeqBitNames::LASSOSTUN, NOT_PREDICTABLE},
    {"ELECTROSTUN", SeqBitNames::ELECTROSTUN, NOT_PREDICTABLE},
    {"WEPHIP", SeqBitNames::WEPHIP, NOT_PREDICTABLE},
    {"WEPBACK", SeqBitNames::WEPBACK, NOT_PREDICTABLE},
    {"WEPUNDERARM", SeqBitNames::WEPUNDERARM, NOT_PREDICTABLE},
    {"WEPLEG", SeqBitNames::WEPLEG, NOT_PREDICTABLE},
    {"SPAWNHIP", SeqBitNames::SPAWNHIP, NOT_PREDICTABLE},
    {"SPAWNBACK", SeqBitNames::SPAWNBACK, NOT_PREDICTABLE},
    {"SPAWNSIDE", SeqBitNames::SPAWNSIDE, NOT_PREDICTABLE},
    {"SPAWNSHOULDER", SeqBitNames::SPAWNSHOULDER, NOT_PREDICTABLE},
    {"SPAWNELEMENT", SeqBitNames::SPAWNELEMENT, NOT_PREDICTABLE},
    {"SNIPERBLAST", SeqBitNames::SNIPERBLAST, NOT_PREDICTABLE},
    {"DUAL", SeqBitNames::DUAL, NOT_PREDICTABLE},
    {"REV", SeqBitNames::REV, NOT_PREDICTABLE},
    {"SPIN", SeqBitNames::SPIN, NOT_PREDICTABLE},
    {"SWIPE", SeqBitNames::SWIPE, NOT_PREDICTABLE},
    {"SLASH", SeqBitNames::SLASH, NOT_PREDICTABLE},
    {"CHOP", SeqBitNames::CHOP, NOT_PREDICTABLE},
    {"STAB", SeqBitNames::STAB, NOT_PREDICTABLE},
    {"UPPERCUT", SeqBitNames::UPPERCUT, NOT_PREDICTABLE},
    {"PARRY", SeqBitNames::PARRY, NOT_PREDICTABLE},
    {"EMERGE", SeqBitNames::EMERGE, NOT_PREDICTABLE},
    {"LEADERSHIP", SeqBitNames::LEADERSHIP, NOT_PREDICTABLE},
    {"SPEECH", SeqBitNames::SPEECH, NOT_PREDICTABLE},
    {"MEDSELF", SeqBitNames::MEDSELF, NOT_PREDICTABLE},
    {"MEDOTHER", SeqBitNames::MEDOTHER, NOT_PREDICTABLE},
    {"MEDDEAD", SeqBitNames::MEDDEAD, NOT_PREDICTABLE},
    {"GETUP", SeqBitNames::GETUP, NOT_PREDICTABLE},
    {"TRAIN", SeqBitNames::TRAIN, NOT_PREDICTABLE},
    {"REPAIR", SeqBitNames::REPAIR, NOT_PREDICTABLE},
    {"RAMPAGE", SeqBitNames::RAMPAGE, NOT_PREDICTABLE},
    {"VANDAL", SeqBitNames::VANDAL, NOT_PREDICTABLE},
    {"COWER", SeqBitNames::COWER, NOT_PREDICTABLE},
    {"MACE", SeqBitNames::MACE, NOT_PREDICTABLE},
    {"AXE", SeqBitNames::AXE, NOT_PREDICTABLE},
    {"SIA", SeqBitNames::SIA, NOT_PREDICTABLE},
    {"SHURIKEN", SeqBitNames::SHURIKEN, NOT_PREDICTABLE},
    {"KATANA", SeqBitNames::KATANA, NOT_PREDICTABLE},
    {"STAFF", SeqBitNames::STAFF, NOT_PREDICTABLE},
    {"SHOTGUN", SeqBitNames::SHOTGUN, NOT_PREDICTABLE},
    {"MACHINEGUN", SeqBitNames::MACHINEGUN, NOT_PREDICTABLE},
    {"RIFLE", SeqBitNames::RIFLE, NOT_PREDICTABLE},
    {"FLOATSTUN", SeqBitNames::FLOATSTUN, NOT_PREDICTABLE},
    {"SHOOT", SeqBitNames::SHOOT, NOT_PREDICTABLE},
    {"CONE", SeqBitNames::CONE, NOT_PREDICTABLE},
    {"BLAST", SeqBitNames::BLAST, NOT_PREDICTABLE},
    {"BOX", SeqBitNames::BOX, NOT_PREDICTABLE},
    {"POSE", SeqBitNames::POSE, NOT_PREDICTABLE},
    {"GREETING", SeqBitNames::GREETING, NOT_PREDICTABLE},
    {"EXCLAMATION", SeqBitNames::EXCLAMATION, NOT_PREDICTABLE},
    {"CAST", SeqBitNames::CAST, NOT_PREDICTABLE},
    {"SUMMON", SeqBitNames::SUMMON, NOT_PREDICTABLE},
    {"SPAWN", SeqBitNames::SPAWN, NOT_PREDICTABLE},
    {"KNIFE", SeqBitNames::KNIFE, NOT_PREDICTABLE},
    {"WAVE", SeqBitNames::WAVE, NOT_PREDICTABLE},
    {"BRAWL", SeqBitNames::BRAWL, NOT_PREDICTABLE},
    {"SHUT", SeqBitNames::SHUT, NOT_PREDICTABLE},
    {"OPEN", SeqBitNames::OPEN, NOT_PREDICTABLE},
    {"LEVITATE", SeqBitNames::LEVITATE, NOT_PREDICTABLE},
    {"FORCE", SeqBitNames::FORCE, NOT_PREDICTABLE},
    {"TELEKINESIS", SeqBitNames::TELEKINESIS, NOT_PREDICTABLE},
    {"PSIONIC", SeqBitNames::PSIONIC, NOT_PREDICTABLE},
    {"WALL", SeqBitNames::WALL, NOT_PREDICTABLE},
    {"CAGE", SeqBitNames::CAGE, NOT_PREDICTABLE},
    {"BOOST", SeqBitNames::BOOST, NOT_PREDICTABLE},
    {"BOOSTUP", SeqBitNames::BOOSTUP, NOT_PREDICTABLE},
    {"BOOSTDOWN", SeqBitNames::BOOSTDOWN, NOT_PREDICTABLE},
    {"TELEPORT", SeqBitNames::TELEPORT, NOT_PREDICTABLE},
    {"STEAL", SeqBitNames::STEAL, NOT_PREDICTABLE},
    {"HEAL", SeqBitNames::HEAL, NOT_PREDICTABLE},
    {"WRIGGLE", SeqBitNames::WRIGGLE, NOT_PREDICTABLE},
    {"LEAPKICK", SeqBitNames::LEAPKICK, NOT_PREDICTABLE},
    {"SPEEDPUNCH", SeqBitNames::SPEEDPUNCH, NOT_PREDICTABLE},
    {"TELEATTACK", SeqBitNames::TELEATTACK, NOT_PREDICTABLE},
    {"HEADCAST", SeqBitNames::HEADCAST, NOT_PREDICTABLE},
    {"PSIONICBLAST", SeqBitNames::PSIONICBLAST, NOT_PREDICTABLE},
    {"RADIUSEFFECT", SeqBitNames::RADIUSEFFECT, NOT_PREDICTABLE},
    {"RADIUSATTACK", SeqBitNames::RADIUSATTACK, NOT_PREDICTABLE},
    {"GROUNDSUMMON", SeqBitNames::GROUNDSUMMON, NOT_PREDICTABLE},
    {"SKYSUMMON", SeqBitNames::SKYSUMMON, NOT_PREDICTABLE},
    {"WEPSPAWN", SeqBitNames::WEPSPAWN, NOT_PREDICTABLE},
    {"WEPSPAWN2", SeqBitNames::WEPSPAWN2, NOT_PREDICTABLE},
    {"WEPSPAWN3", SeqBitNames::WEPSPAWN3, NOT_PREDICTABLE},
    {"POWERPUNCH", SeqBitNames::POWERPUNCH, NOT_PREDICTABLE},
    {"SHAKE", SeqBitNames::SHAKE, NOT_PREDICTABLE},
    {"DRAIN", SeqBitNames::DRAIN, NOT_PREDICTABLE},
    {"CHOKE", SeqBitNames::CHOKE, NOT_PREDICTABLE},
    {"HEADPAIN", SeqBitNames::HEADPAIN, NOT_PREDICTABLE},
    {"FEAR", SeqBitNames::FEAR, NOT_PREDICTABLE},
    {"BOW", SeqBitNames::BOW, NOT_PREDICTABLE},
    {"CROSSBOW", SeqBitNames::CROSSBOW, NOT_PREDICTABLE},
    {"THROWKNIFE", SeqBitNames::THROWKNIFE, NOT_PREDICTABLE},
    {"THROWSTAR", SeqBitNames::THROWSTAR, NOT_PREDICTABLE},
    {"GRENADE", SeqBitNames::GRENADE, NOT_PREDICTABLE},
    {"CHAINGUN", SeqBitNames::CHAINGUN, NOT_PREDICTABLE},
    {"BOOMERANG", SeqBitNames::BOOMERANG, NOT_PREDICTABLE},
    {"BALLCAST", SeqBitNames::BALLCAST, NOT_PREDICTABLE},
    {"SMASH", SeqBitNames::SMASH, NOT_PREDICTABLE},
    {"DISRUPT", SeqBitNames::DISRUPT, NOT_PREDICTABLE},
    {"KUNGFUPOWER", SeqBitNames::KUNGFUPOWER, NOT_PREDICTABLE},
    {"KUNGFUSPEED", SeqBitNames::KUNGFUSPEED, NOT_PREDICTABLE},
    {"KUNGFUAREA", SeqBitNames::KUNGFUAREA, NOT_PREDICTABLE},
    {"KUNGFUSTUN", SeqBitNames::KUNGFUSTUN, NOT_PREDICTABLE},
    {"MAGICTHROW", SeqBitNames::MAGICTHROW, NOT_PREDICTABLE},
    {"STONEARMOR", SeqBitNames::STONEARMOR, NOT_PREDICTABLE},
    {"STOMP", SeqBitNames::STOMP, NOT_PREDICTABLE},
    {"RAISEDEAD", SeqBitNames::RAISEDEAD, NOT_PREDICTABLE},
    {"FLAMETHROWER", SeqBitNames::FLAMETHROWER, NOT_PREDICTABLE},
    {"ROCKETLAUNCHER", SeqBitNames::ROCKETLAUNCHER, NOT_PREDICTABLE},
    {"WINDBLAST", SeqBitNames::WINDBLAST, NOT_PREDICTABLE},
    {"BLINDING", SeqBitNames::BLINDING, NOT_PREDICTABLE},
    {"MEGABLAST", SeqBitNames::MEGABLAST, NOT_PREDICTABLE},
    {"BLOW", SeqBitNames::BLOW, NOT_PREDICTABLE},
    {"FIREAURA", SeqBitNames::FIREAURA, NOT_PREDICTABLE},
    {"RAPIDBLAST", SeqBitNames::RAPIDBLAST, NOT_PREDICTABLE},
    {"POISON", SeqBitNames::POISON, NOT_PREDICTABLE},
    {"SUPERBOOST", SeqBitNames::SUPERBOOST, NOT_PREDICTABLE},
    {"CLAWSPEED", SeqBitNames::CLAWSPEED, NOT_PREDICTABLE},
    {"CLAWAREA", SeqBitNames::CLAWAREA, NOT_PREDICTABLE},
    {"LOOT", SeqBitNames::LOOT, NOT_PREDICTABLE},
    {"MENACE", SeqBitNames::MENACE, NOT_PREDICTABLE},
    {"THREAT", SeqBitNames::THREAT, NOT_PREDICTABLE},
    {"LOITER", SeqBitNames::LOITER, NOT_PREDICTABLE},
    {"PUSH", SeqBitNames::PUSH, NOT_PREDICTABLE},
    {"PURSETUG", SeqBitNames::PURSETUG, NOT_PREDICTABLE},
    {"GROUNDPUNCH", SeqBitNames::GROUNDPUNCH, NOT_PREDICTABLE},
    {"VANDALIZE", SeqBitNames::VANDALIZE, NOT_PREDICTABLE},
    {"RUNOUT", SeqBitNames::RUNOUT, NOT_PREDICTABLE},
    {"SPRAYCAN", SeqBitNames::SPRAYCAN, NOT_PREDICTABLE},
    {"THANK", SeqBitNames::THANK, NOT_PREDICTABLE},
    {"BOOSTAURA", SeqBitNames::BOOSTAURA, NOT_PREDICTABLE},
    {"LOOKOUT", SeqBitNames::LOOKOUT, NOT_PREDICTABLE},
    {"CHEER", SeqBitNames::CHEER, NOT_PREDICTABLE},
    {"DEALING", SeqBitNames::DEALING, NOT_PREDICTABLE},
    {"RECEIVING", SeqBitNames::RECEIVING, NOT_PREDICTABLE},
    {"OBSERVE", SeqBitNames::OBSERVE, NOT_PREDICTABLE},
    {"TALK", SeqBitNames::TALK, NOT_PREDICTABLE},
    {"LISTEN", SeqBitNames::LISTEN, NOT_PREDICTABLE},
    {"ROLLDICE", SeqBitNames::ROLLDICE, NOT_PREDICTABLE},
    {"CHANTING", SeqBitNames::CHANTING, NOT_PREDICTABLE},
    {"BOWING", SeqBitNames::BOWING, NOT_PREDICTABLE},
    {"MEDITATE", SeqBitNames::MEDITATE, NOT_PREDICTABLE},
    {"MOMENTGLORY", SeqBitNames::MOMENTGLORY, NOT_PREDICTABLE},
    {"GUTPUNCH", SeqBitNames::GUTPUNCH, NOT_PREDICTABLE},
    {"JUMPNSMASH", SeqBitNames::JUMPNSMASH, NOT_PREDICTABLE},
    {"BASEBALLPUNCH", SeqBitNames::BASEBALLPUNCH, NOT_PREDICTABLE},
    {"SHORTTOSS", SeqBitNames::SHORTTOSS, NOT_PREDICTABLE},
    {"WINDUPPERCUT", SeqBitNames::WINDUPPERCUT, NOT_PREDICTABLE},
    {"TAUNTA", SeqBitNames::TAUNTA, NOT_PREDICTABLE},
    {"TAUNTB", SeqBitNames::TAUNTB, NOT_PREDICTABLE},
    {"TAUNTC", SeqBitNames::TAUNTC, NOT_PREDICTABLE},
    {"STOPCAST", SeqBitNames::STOPCAST, NOT_PREDICTABLE},
    {"SCREAM", SeqBitNames::SCREAM, NOT_PREDICTABLE},
    {"TERRIFY", SeqBitNames::TERRIFY, NOT_PREDICTABLE},
    {"ENERGYTORRENT", SeqBitNames::ENERGYTORRENT, NOT_PREDICTABLE},
    {"HYPNOTIZE", SeqBitNames::HYPNOTIZE, NOT_PREDICTABLE},
    {"JUMPCLAW", SeqBitNames::JUMPCLAW, NOT_PREDICTABLE},
    {"WINDUP", SeqBitNames::WINDUP, NOT_PREDICTABLE},
    {"MAGICWINDUP", SeqBitNames::MAGICWINDUP, NOT_PREDICTABLE},
    {"POWERWINDUP", SeqBitNames::POWERWINDUP, NOT_PREDICTABLE},
    {"MYSICWINDUP", SeqBitNames::MYSICWINDUP, NOT_PREDICTABLE},
    {"TECHWINDUP", SeqBitNames::TECHWINDUP, NOT_PREDICTABLE},
    {"POINT", SeqBitNames::POINT, NOT_PREDICTABLE},
    {"LAUGH", SeqBitNames::LAUGH, NOT_PREDICTABLE},
    {"SAD", SeqBitNames::SAD, NOT_PREDICTABLE},
    {"HAPPY", SeqBitNames::HAPPY, NOT_PREDICTABLE},
    {"ANGRY", SeqBitNames::ANGRY, NOT_PREDICTABLE},
    {"LOVE", SeqBitNames::LOVE, NOT_PREDICTABLE},
    {"EVIL", SeqBitNames::EVIL, NOT_PREDICTABLE},
    {"FLEX", SeqBitNames::FLEX, NOT_PREDICTABLE},
    {"BECKON", SeqBitNames::BECKON, NOT_PREDICTABLE},
    {"CLAP", SeqBitNames::CLAP, NOT_PREDICTABLE},
    {"RUDE", SeqBitNames::RUDE, NOT_PREDICTABLE},
    {"YES", SeqBitNames::YES, NOT_PREDICTABLE},
    {"NO", SeqBitNames::NO, NOT_PREDICTABLE},
    {"SHRUG", SeqBitNames::SHRUG, NOT_PREDICTABLE},
    {"DANCE", SeqBitNames::DANCE, NOT_PREDICTABLE},
    {"SALUTE", SeqBitNames::SALUTE, NOT_PREDICTABLE},
    {"BUSY", SeqBitNames::BUSY, NOT_PREDICTABLE},
    {"STOP", SeqBitNames::STOP, NOT_PREDICTABLE},
    {"WAIT", SeqBitNames::WAIT, NOT_PREDICTABLE},
    {"HELP", SeqBitNames::HELP, NOT_PREDICTABLE},
    {"YELL", SeqBitNames::YELL, NOT_PREDICTABLE},
    {"CHAT", SeqBitNames::CHAT, NOT_PREDICTABLE},
    {"COMMAND", SeqBitNames::COMMAND, NOT_PREDICTABLE},
    {"JOBSDONE", SeqBitNames::JOBSDONE, NOT_PREDICTABLE},
    {"PLAN", SeqBitNames::PLAN, NOT_PREDICTABLE},
    {"WELDING", SeqBitNames::WELDING, NOT_PREDICTABLE},
    {"WORK", SeqBitNames::WORK, NOT_PREDICTABLE},
    {"BODY", SeqBitNames::BODY, NOT_PREDICTABLE},
    {"BOULDER", SeqBitNames::BOULDER, NOT_PREDICTABLE},
    {"ZOMBIE", SeqBitNames::ZOMBIE, NOT_PREDICTABLE},
    {"KNEEL", SeqBitNames::KNEEL, NOT_PREDICTABLE},
    {"PLAYER", SeqBitNames::PLAYER, PREDICTABLE},
    {"RECLAIM", SeqBitNames::RECLAIM, NOT_PREDICTABLE},
    {"CLIPBOARD", SeqBitNames::CLIPBOARD, NOT_PREDICTABLE},
    {"BRIEFCASE", SeqBitNames::BRIEFCASE, NOT_PREDICTABLE},
    {"CELLPHONE", SeqBitNames::CELLPHONE, NOT_PREDICTABLE},
    {"NEWSPAPER", SeqBitNames::NEWSPAPER, NOT_PREDICTABLE},
    {"EAT", SeqBitNames::EAT, NOT_PREDICTABLE},
    {"CARPET", SeqBitNames::CARPET, NOT_PREDICTABLE},
    {"INSEWERWATER", SeqBitNames::INSEWERWATER, NOT_PREDICTABLE},
    {"SLIDING", SeqBitNames::SLIDING, PREDICTABLE},
    {"BOUNCING", SeqBitNames::BOUNCING, PREDICTABLE},
    {"WARP", SeqBitNames::WARP, NOT_PREDICTABLE},
    {"RUNIN", SeqBitNames::RUNIN, NOT_PREDICTABLE},
    {"GAME", SeqBitNames::GAME, NOT_PREDICTABLE},
    {"RPS", SeqBitNames::RPS, NOT_PREDICTABLE},
    {"DICE", SeqBitNames::DICE, NOT_PREDICTABLE},
    {"ARGUE", SeqBitNames::ARGUE, NOT_PREDICTABLE},
    {"REASON", SeqBitNames::REASON, NOT_PREDICTABLE},
    {"QUESTION", SeqBitNames::QUESTION, NOT_PREDICTABLE},
    {"AFK", SeqBitNames::AFK, NOT_PREDICTABLE},
    {"ELEVATOR", SeqBitNames::ELEVATOR, NOT_PREDICTABLE},
    {"SCORE", SeqBitNames::SCORE, NOT_PREDICTABLE},
    {"ONE", SeqBitNames::ONE, NOT_PREDICTABLE},
    {"TWO", SeqBitNames::TWO, NOT_PREDICTABLE},
    {"THREE", SeqBitNames::THREE, NOT_PREDICTABLE},
    {"FOUR", SeqBitNames::FOUR, NOT_PREDICTABLE},
    {"FIVE", SeqBitNames::FIVE, NOT_PREDICTABLE},
    {"SIX", SeqBitNames::SIX, NOT_PREDICTABLE},
    {"SEVEN", SeqBitNames::SEVEN, NOT_PREDICTABLE},
    {"EIGHT", SeqBitNames::EIGHT, NOT_PREDICTABLE},
    {"NINE", SeqBitNames::NINE, NOT_PREDICTABLE},
    {"TEN", SeqBitNames::TEN, NOT_PREDICTABLE},
    {"ZERO", SeqBitNames::ZERO, NOT_PREDICTABLE},
};
bool moveAInterruptsAnyMemberOfB(const TranslatedMove &mvA, const TranslatedMove &mvB)
{
    for (uint32_t member_hash : mvB.members)
    {
        if (mvA.irqs.find(member_hash) != mvA.irqs.end())
            return true;
    }
    return false;
}
void cleanUpFxName(QByteArray &name)
{
    name = QDir::cleanPath(name.toUpper()).toLatin1();
    name.replace("/FX/", "");
    if (name.startsWith("FX/"))
    {
        name.remove(0, 3);
    }
}
static SeqBitNames seqBitNameToEnum(const QByteArray &src)
{
    QMetaEnum   metaEnum  = QMetaEnum::fromType<SEGS_Enums::SeqBitNames>();
    bool        converted = false;
    SeqBitNames val       = SeqBitNames(metaEnum.keyToValue(src.data(), &converted));
    if (!converted)
    {
        // check for special case
        if (src == "2HAND")
            return SeqBitNames::TWO_HAND;
        return SeqBitNames::NON_EXISTING;
    }
    return val;
}
/**
 * @brief sets the SeqBitSet bits based on sequencer bit names.
 * @param names a container with names of bits to set.
 * @param bset a SeqBitSet that will be modified.
 * @param move_name only used in case of an error to print debug info.
 */
void setBitsFromString(std::vector<QByteArray> &names, SeqBitSet &bset, const QByteArray &move_name)
{
    for (const QByteArray &nm : names)
    {
        SeqBitNames seq_bit = seqBitNameToEnum(nm.toUpper());
        if (seq_bit == SeqBitNames::NON_EXISTING)
        {
            qCritical() << "The move" << move_name << " is using a state" << nm << "which we don't process";
            assert(false);
        }
        else
        {
            bset.set(seq_bit);
        }
    }
}
/**
 * @brief check that only bits that are set on lhs, are the ones in rhs
 * @param lhs
 * @param rhs
 */
bool areOnlyTheseBitsSet(SeqBitSet &lhs, SeqBitSet &rhs)
{
    return (lhs.bits & ~rhs.bits).none();
}
/**
 * @brief checks if all bits that are set by the 'raw' move data are predictable, and if so, marks the whole move as
 * such.
 * @param raw move data to update the predictability of
 */
void setPredictability(SeqMoveRawData &raw)
{
    static bool      init = false;
    static SeqBitSet predictable_bits; // only predictable seq bits are set here.
    if (!init)
    {
        init = true;
        for (const SeqMetaData &data : seq_data)
        {
            if (data.predictable == PREDICTABLE)
                predictable_bits.set(data.bitname);
        }
    }
    // check if only predictable bits are set in raw.requires_bits
    raw.sets_bits.setVal(SeqBitNames::PREDICTABLE, areOnlyTheseBitsSet(raw.requires_bits, predictable_bits));
}
/**
 * @brief Convert all next move names to indices
 * @param seq the sequencer that owns those moves
 * @param raw target raw sequencer data
 * @param next_move_data contains the array of names of the next moves
 */
void setupNextMoveIndices(const SequencerData &seq, SeqMoveRawData &raw,
                          const std::vector<SeqNextMoveData> &next_move_data)
{
    assert(next_move_data.size() < std::numeric_limits<uint8_t>::max());

    raw.num_nextmoves = uint8_t(next_move_data.size());
    assert(raw.num_nextmoves < 4);
    for (uint8_t j = 0; j < raw.num_nextmoves; ++j)
    {
        raw.nextMove[j] = getSeqMoveIdxByName(next_move_data[j].name, seq);
        if (raw.nextMove[j] > MAXMOVES)
            raw.nextMove[j] = 0;
    }
    if (!raw.num_nextmoves)
    {
        raw.num_nextmoves = 1;
        raw.nextMove[0]   = 0;
    }
}
/**
 * @brief Convert all cycle move names to indices
 * @param seq the sequencer that owns those moves
 * @param raw target raw sequencer data
 * @param cycle_move_data contains the array of names of the cycle moves
 */
void setupCycleMoveIndices(const SequencerData &seq, SeqMoveRawData &raw,
                           const std::vector<SeqCycleMoveData> &cycle_move_data)
{
    assert(cycle_move_data.size() < std::numeric_limits<uint8_t>::max());

    raw.cycleMoveCnt = uint8_t(cycle_move_data.size());
    assert(raw.cycleMoveCnt <= 4);
    for (uint8_t j = 0; j < raw.cycleMoveCnt; ++j)
    {
        raw.cycleMove[j] = getSeqMoveIdxByName(cycle_move_data[j].name, seq);
        assert(raw.cycleMove[j] >= 0 && raw.cycleMove[j] <= MAXMOVES);
    }
}

void setupSequencerData(SequencerData &seq)
{
    size_t move_count = seq.m_Move.size();

    assert(move_count < std::numeric_limits<int16_t>::max());

    for (size_t i = 0; i < move_count; ++i)
    {
        SeqMoveData &move(seq.m_Move[i]);
        for (SeqMoveTypeData &move_type : move.m_Type)
        {
            if (move_type.PitchRate == 0.0f) // 0 means default pitch rate.
                move_type.PitchRate = 0.05f;
            for (SeqPlayFxData &fx : move_type.m_PlayFx)
            {
                cleanUpFxName(fx.name);
            }
        }
        // normalize the interrupts and member strings
        for (QByteArray &l : move.Interrupts)
            l = l.toUpper();
        for (QByteArray &m : move.Member)
            m = m.toUpper();
        // record the move index and pointer to source move
        move.m_raw.idx           = int(i);
        move.m_raw.m_source_data = &move;
        setupNextMoveIndices(seq, move.m_raw, move.m_NextMove);
        setupCycleMoveIndices(seq, move.m_raw, move.m_CycleMove);
        if (move.m_raw.cycleMoveCnt)
            move.Flags |= SeqMoveData::ComplexCycle;
        setBitsFromString(move.Sets, move.m_raw.sets_bits, move.name);
        setBitsFromString(move.SetsOnChild, move.m_raw.sets_child_bits, move.name);
        setBitsFromString(move.SticksOnChild, move.m_raw.sticks_on_child_bits, move.name);
        setBitsFromString(move.Requires, move.m_raw.requires_bits, move.name);
        setPredictability(move.m_raw);
    }
    QHash<QByteArray, SeqMoveData *> move_locator;
    QSet<QByteArray>                 group_locator;
    QSet<uint32_t>                   all_hashes;
    for (SeqMoveData &move : seq.m_Move)
    {
        move_locator[move.name.toLower()] = &move;
        uint32_t entry_hash               = qHash(move.name);
        if (all_hashes.contains(entry_hash))
        {
            qCritical() << "Hash collision for move/group names";
        }
        else
            all_hashes.insert(entry_hash);
    }
    for (SeqGroupNameData &grp : seq.m_Group)
    {
        group_locator.insert(grp.name.toLower());
        uint32_t entry_hash = qHash(grp.name);
        if (all_hashes.contains(entry_hash))
        {
            qCritical() << "Hash collision for move/group names";
        }
        else
            all_hashes.insert(entry_hash);
    }
    for (SeqMoveData &move : seq.m_Move)
    {
        for (const QByteArray &name : move.Interrupts)
        {
            bool interrupts_move  = move_locator.contains(name.toLower());
            bool interrupts_group = group_locator.contains(name.toLower());
            if (!interrupts_move && !interrupts_group)
            {
                qCritical()
                    << QString("%1 sequencer's %2 move's Interrupt field [%3] must reference either a move or a group")
                           .arg(QString(seq.name), QString(move.name), QString(name));
            }
        }
        for (const QByteArray &name : move.Member)
        {
            bool is_member_of_group = group_locator.contains(name.toLower());
            if (!is_member_of_group)
            {
                qCritical() << QString("%1 sequencer's %2 move's Group field [%3] must reference a group")
                                   .arg(QString(seq.name), QString(move.name), QString(name));
            }
        }
    }
    // collect all move interrupts/members to build interrupted_by relations
    TranslatedMove moves[MAXMOVES];
    for (size_t i = 0; i < move_count; ++i)
    {
        SeqMoveData &   move = seq.m_Move[i];
        TranslatedMove &mt(moves[i]);
        assert(move.Interrupts.size() < 20);
        assert(move.Member.size() < 20);

        for (const QByteArray &name : move.Interrupts)
            mt.irqs.insert(qHash(name));
        for (const QByteArray &name : move.Member)
            mt.members.insert(qHash(name));
        // move itself can be interrupted as well
        mt.members.insert(qHash(move.name));
    }
    // calculate interrupted_by indices.
    for (size_t i = 0; i < move_count; ++i)
    {
        SeqMoveRawData &      move = seq.m_Move[i].m_raw;
        const TranslatedMove &mt(moves[i]);
        for (int16_t j = 0; j < int16_t(move_count); ++j)
        {
            if (moveAInterruptsAnyMemberOfB(moves[j], mt))
                move.interrupted_by.push_back(j);
        }
    }
}
/**
 * @brief sets the m_anm_track to the default/idle animation
 * @param movetype to initialize
 * @return true on success
 */
bool seqAttachAnAnim(SeqMoveTypeData &movetype)
{
    bool success = true;
    SeqMoveDataTypeAnim &type_anim(movetype.m_Anim[0]);

    movetype.m_anm_track = getOrLoadAnimationTrack(type_anim.name);
    if (!movetype.m_anm_track)
    {
        qCritical("MISSING ANIMATION %s ", qPrintable(type_anim.name));
        movetype.m_anm_track = getOrLoadAnimationTrack("male/thumbsup");
        success              = false;
    }
    if (0 == type_anim.lastFrame)
    {
        SeqMoveDataTypeAnim &type_anim(movetype.m_Anim.front());
        type_anim.lastFrame = HAnimationTrack(movetype.m_anm_track)->m_length;
        if (type_anim.lastFrame < type_anim.firstFrame)
            qWarning() << "lastFrame < firstFrame -> need to handle reversed animations";
    }
    return success;
}
/**
 * @brief For every move type in every move, attach the default animation
 * @param seq the SequencerData to process.
 */
void setupSequencerAnimationLinks(SequencerData &seq)
{
    for (SeqMoveData &move : seq.m_Move)
    {
        for (SeqMoveTypeData &atype : move.m_Type)
        {
            if (!seqAttachAnAnim(atype))
            {
                QByteArray animname = atype.m_Anim[0].name;
                qCritical() << QString("Missing animation %s for move/sequencer [%2/%3]")
                                   .arg(QString(animname), QString(move.name), QString(seq.name));
            }
        }
    }
}
void prepareSequencer(SequencerData &seq)
{
    setupSequencerData(seq);
    setupSequencerAnimationLinks(seq);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // end of anonymous namespace
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SequencerData *SEGS::getInitializedSequencerData(const QByteArray &name)
{
    SequencerData *data = getGameData().m_seq_definitions.getSequencerData(name);
    if (!data)
        return nullptr;

    SequencerInstanceStorage &store(SequencerInstanceStorage::instance());
    auto                      iter = store.m_active_instances.find(data);
    if (iter != store.m_active_instances.end())
        return data; // already processed
    prepareSequencer(*data);
    store.m_active_instances.insert(data);
    return data;
}

SequencerInstance::SequencerInstance(SceneTreeNode *node)
{
    m_next_move_idx = rand();
    m_node          = node;
    m_part_colors.resize(MAX_BONES);
    for (ModelPartColor &mpc : m_part_colors)
    {
        mpc.color1 = {1, 1, 1, 1};
        mpc.color2 = {1, 1, 1, 1};
    }
}
EntitySequencerData *seqLoadSeqType(const QString &a1)
{
    GameDataStore &gd(getGameData());
    auto iter = gd.m_seq_types.find(QFileInfo(a1).fileName().toLatin1().toLower());
    if (gd.m_seq_types.end() == iter)
        return nullptr;
    return &(iter->second);
}

void convertCollisionType(EntitySequencerData *esd)
{
    static const QList<QByteArray> col_type_names = {"repulsion", "rubberball",   "bounceup", "bouncefacing",
                                                     "steadyup",  "steadyfacing", "door"};
    static constexpr CollisionType types[]        = {
        CollisionType::None,         CollisionType::Repulsion, CollisionType::Rubberball,   CollisionType::BounceUp,
        CollisionType::BounceFacing, CollisionType::SteadyUp,  CollisionType::SteadyFacing, CollisionType::Door
    };

    int idx                         = col_type_names.indexOf(esd->m_collision_type.toLower());
    // if name wasn't found, the CollisionType::None is selected.
    esd->m_converted_collision_type = types[idx + 1];
}

void convertSelectionMode(EntitySequencerData *esd)
{
    const static QList<QByteArray> sel_mode_names = {"collision", "worldgroup"};
    static constexpr SelectionMode types[]        = {
        SelectionMode::Collision, SelectionMode::Worldgroup, SelectionMode::Bones
    };

    int idx            = sel_mode_names.indexOf(esd->m_selection.toLower());
    esd->m_converted_selection_mode               = types[idx + 1];
}
int seqSetStateFromString(SeqBitSet *tgt, const QByteArray &src)
{
    using namespace SEGS_Enums;
    if (!tgt || src.isEmpty())
        return 1;

    QStringList bitnames = QString(src).split(QRegExp("[ ,\t\n]"));
    int         result   = 1;
    for (const QString &bitname : bitnames)
    {
        QMetaEnum   metaEnum = QMetaEnum::fromType<SEGS_Enums::SeqBitNames>();
        int         enumval  = metaEnum.keyToValue(qPrintable(bitname));
        SeqBitNames val      = enumval == -1 ? SeqBitNames::NON_EXISTING : SeqBitNames(enumval);
        if (val == SeqBitNames::NON_EXISTING)
            result = 0; // BUG: should this return at this point ?
        else
            tgt->set(val);
    }
    return result;
}

void seqGetSeqType(EntitySequencerData &tgt, const QByteArray &type_name)
{
    EntitySequencerData *esd = s_seq_types.value(type_name, nullptr);
    if (esd)
    {
        tgt = *esd;
        return;
    }
    esd = seqLoadSeqType(type_name);
    assert(esd);
    esd->m_name = type_name;
    if (esd->m_anim_scale == 0.0f)
        esd->m_anim_scale = 1.0f;
    // TODO: the orignal client always set
    esd->m_anim_scale = 1.0f; // FIXME: likely an error here, AnimScale always set to 1.0f
    if (esd->m_geometry_scale.x == 0.0f)
        esd->m_geometry_scale.x = 1.0f;
    if (esd->m_geometry_scale.y == 0.0f)
        esd->m_geometry_scale.y = 1.0f;
    if (esd->m_geometry_scale.z == 0.0f)
        esd->m_geometry_scale.z = 1.0f;
    if (esd->m_lod_dists[3] == 0.0f)
        esd->m_lod_dists[3] = 100000.0f;
    if (esd->m_lod_dists[2] == 0.0f)
        esd->m_lod_dists[2] = esd->m_lod_dists[3];
    if (esd->m_lod_dists[1] == 0.0f)
        esd->m_lod_dists[1] = esd->m_lod_dists[2];
    for (QByteArray &lod_name : esd->m_lod_names)
    { // BUG: this code is likely incorrect, since farther LODs are set to
        // esd->m_graphics even if lower lods are available, but original client is doing that
        if (lod_name.isEmpty())
            lod_name = esd->m_graphics;
    }
    // Default collision box size.
    if (esd->m_collision_size.x == 0.0f)
        esd->m_collision_size.x = 3.0f;
    if (esd->m_collision_size.y == 0.0f)
        esd->m_collision_size.y = 6.0f;
    if (esd->m_collision_size.z == 0.0f)
        esd->m_collision_size.z = 3.0f;
    if (!esd->m_ticks_to_linger_after_death)
        esd->m_ticks_to_linger_after_death = 600;
    if (esd->m_ticks_to_linger_after_death <= 1)
        esd->m_ticks_to_linger_after_death = 0;
    if (esd->m_ticks_to_fade_away_after_death == 0)
        esd->m_ticks_to_fade_away_after_death = 90;
    if (esd->m_ticks_to_fade_away_after_death == 1 || esd->m_ticks_to_fade_away_after_death < 0)
        esd->m_ticks_to_fade_away_after_death = 0;
    if (esd->m_visibility_sphere_radius == 0.0f)
        esd->m_visibility_sphere_radius = 5.0f;
    if (!esd->m_max_alpha)
        esd->m_max_alpha = 255;
    if (esd->m_reticle_mod.y == 0.0f)
        esd->m_reticle_mod.y = 0.5f;
    if (esd->m_reticle_mod.x == 0.0f)
        esd->m_reticle_mod.x = 1.0f;
    if (esd->m_fade_out_start == 0.0f)
        esd->m_fade_out_start = 350.0f;
    if (esd->m_fade_out_finish == 0.0f)
        esd->m_fade_out_finish = esd->m_fade_out_start + 100.0f;
    QByteArray shadow_type    = esd->m_shadow_type.toLower();
    QByteArray shadow_quality = esd->m_shadow_quality.toLower();
    if ("stencil" == shadow_type)
        esd->m_converted_shadow_type = 3; //SEQ_STENCIL_SHADOW
    else if ("none" == shadow_type)
        esd->m_converted_shadow_type = 2; //SEQ_NO_SHADOW
    else if ("bipedsplat" == shadow_type)
        esd->m_converted_shadow_type = 4; //SEQ_BIPED_SPLAT_SHADOW
    else
        esd->m_converted_shadow_type = 1; //SEQ_SPLAT_SHADOW
    if ("high" == shadow_quality)
        esd->m_converted_shadow_quality = 3;
    else if ("low" == shadow_quality)
        esd->m_converted_shadow_quality = 1;
    else
        esd->m_converted_shadow_quality = 2;
    esd->m_converted_flags   = 0;
    QByteArray lowered_flags = esd->m_flags.toLower();
    if (lowered_flags.contains("noshallowsplash"))
        esd->m_converted_flags |= 1;
    if (lowered_flags.contains("nodeepsplash"))
        esd->m_converted_flags |= 3;
    if (esd->m_shadow_size.x == 0.0f)
        esd->m_shadow_size.x = 3.6f;
    if (esd->m_shadow_size.z == 0.0f)
        esd->m_shadow_size.z = 3.6f;
    if (esd->m_shadow_size.y == 0.0f)
    {
        switch (esd->m_converted_shadow_quality)
        {
        case 1: esd->m_shadow_size.y = 3.0f; break;
        case 2:
            esd->m_shadow_size.y = 3.0f; // TODO: this original behaviour is just strange here
            break;
        case 3: esd->m_shadow_size.y = 30.0f; break;
        default: assert(false);
        }
    }
    if (esd->m_shadow_texture.isEmpty())
    {
        esd->m_shadow_texture = "PlayerShadowCircle";
    }
    if (esd->m_placement.toLower() == "deadon") // DeadOn
        esd->m_converted_placement = 2;
    else
        esd->m_converted_placement = 1;
    convertCollisionType(esd);
    convertSelectionMode(esd);

    if (!esd->m_constant_state.isEmpty())
        seqSetStateFromString(&esd->m_converted_constant_bits, esd->m_constant_state);
    if (esd->m_seq_type.isEmpty())
    {
        esd->m_seq_type = esd->m_sequencer_name.mid(0, esd->m_sequencer_name.lastIndexOf("."));
    }
    tgt = *esd;
}
static float rand_LCG_float(int *seed)
{
    *seed = 0x343FD * *seed + 0x269EC3;
    return (float)*seed / 2147483636.0f;
}
static int rand_LCG_int16_t(int *rand_seed)
{
    int new_val = 0x343FD * *rand_seed + 0x269EC3;
    *rand_seed  = new_val;
    return (new_val >> 16) & 0x7FFF;
}
static float getAnimScaleFromSeq(HSequencerInstance hseq)
{
    SeqMoveData *movedata = hseq->m_anim.move;
    if (!movedata)
        return 1.0f;
    if ((movedata->m_raw.sets_bits.isSet(SeqBitNames::PREDICTABLE) || movedata->Flags & SeqMoveData::AlwaysSizeScale) &&
        !(movedata->Flags & SeqMoveData::NoSizeScale))
    {
        if (!(movedata->Flags & SeqMoveData::FullSizeScale))
        {
            return glm::clamp(hseq->m_current_animation_scale, 0.5f, 1.5f);
        }
        return hseq->m_current_animation_scale;
    }
    return 1.0f;
}
static void seqCycleFrame(float *frame, float *prev_frame, int first_frame, int last_frame)
{
    *frame -= (last_frame - first_frame);
    if (*frame > last_frame)
        *frame = first_frame;
    *prev_frame = -1.0f;
}
SeqMoveData *seqClientStep(SeqMove *state, SequencerData *dat, float advance, int *rand_seed, uint32_t *tgt_flag)
{
    SeqMoveData *selected_move = nullptr;
    SeqMoveData *curr_move     = state->move;

    state->prev_frame = state->frame;
    state->frame += advance * curr_move->Scale;
    SeqMoveTypeData *move_def = state->type;

    int first_frame = move_def->m_Anim.front().firstFrame;
    int last_frame  = move_def->m_Anim.front().lastFrame;

    if (state->frame > last_frame)
        return nullptr;
    if (!(curr_move->Flags & SeqMoveData::Cycle))
    {
        *tgt_flag |= 0x10u;
        return &dat->m_Move[curr_move->m_raw.nextMove[0]];
    }
    if (curr_move->Flags & SeqMoveData::ComplexCycle)
        selected_move = &dat->m_Move[curr_move->m_raw.cycleMove[rand_LCG_int16_t(rand_seed) % curr_move->m_raw.cycleMoveCnt]];
    if (selected_move && selected_move != curr_move)
    {
        *tgt_flag |= 0x20u;
        return selected_move;
    }
    seqCycleFrame(&state->frame, &state->prev_frame, first_frame, last_frame);
    return nullptr;
}
SeqMoveTypeData *seqGetTypeGfx(SequencerData *data, SeqMoveData *md, const QByteArray &typeName)
{
    if (typeName.isEmpty() || typeName.toLower() == "none")
        return nullptr;

    for (SeqMoveTypeData &mtd : md->m_Type)
    {
        if (mtd.name.toLower() == typeName.toLower())
            return &mtd;
    }
    SeqTypeDefData *seq_typedef = getSeqTypedefByName(data, typeName);
    if (!seq_typedef)
        return nullptr;
    return seqGetTypeGfx(data, md, seq_typedef->pParentType);
}
static void seqSetMove(HSequencerInstance seq, SeqMoveData *move_data, bool is_obvious)
{
    float    lastframe  = 0;
    float    firstframe = 0;
    SeqMove *anim       = &seq->m_anim;
    if (move_data)
    {
        seq->m_anim.type = seqGetTypeGfx(seq->m_template, move_data, seq->m_seq_type_info.m_seq_type);
        if (!seq->m_anim.type)
        {

            qCritical() << QString("Invalid sequencer data: %1 sequencer %2 missing sequencer move type %3 in move %4.")
                               .arg(QString(seq->m_seq_type_info.m_name), QString(seq->m_template->name),
                                    QString(seq->m_seq_type_info.m_seq_type), QString(move_data->name));
            seq->m_anim.type = &move_data->m_Type.front();
        }
        firstframe = seq->m_anim.type->m_Anim.front().firstFrame;
        lastframe  = seq->m_anim.type->m_Anim.front().lastFrame;
    }
    bool frame_in_range = (seq->m_anim.frame < lastframe && seq->m_anim.frame >= firstframe);
    if (!anim->move || !((move_data->Flags & SeqMoveData::Cycle) && (anim->move->Flags & SeqMoveData::Cycle)) ||
        !frame_in_range || anim->move->Flags & SeqMoveData::ComplexCycle)
    {
        seq->m_anim.frame      = firstframe;
        seq->m_anim.prev_frame = -1.0f;
    }
    seq->m_anim.prev_move = seq->m_anim.move;
    anim->move            = move_data;
    if (!is_obvious)
        seq->m_anim.move_to_send = move_data;
}
int seqProcessClientInst(HSequencerInstance seq, float step_size, int idx, bool changed)
{
    int            num_moves_in_arr;
    uint32_t       flags = 0;
    SeqMove *      anim  = &seq->m_anim;
    SeqMoveData *  move  = nullptr;
    SequencerData *tpl = seq->m_template;
    float          adv = getAnimScaleFromSeq(seq) * step_size;
    if (changed)
    {
        num_moves_in_arr = tpl->m_Move.size();
        if (idx >= num_moves_in_arr)
            idx = 0;
        move  = &tpl->m_Move[idx];
        flags = 0x40;
    }
    if (!move || move == anim->move)
        move = seqClientStep(anim, tpl, adv, &seq->m_rand_seed, &flags);
    if (!move || move == anim->move)
        return false;
    seqSetMove(seq, move, false);
    //dumpDebugSeqState(seq,move, flags,SeqSide::Client)
    return true;
}

namespace SEGS
{
void seqResetSeqType(HSequencerInstance seq_handle, FSWrapper &fs, const char *entType_filename, int seed)
{
    float bonescale_ratio;
    SequencerInstance &seq(seq_handle.get());
    // consider releasing previous data underlying the seq_handle

    seqGetSeqType(seq.m_seq_type_info, entType_filename);
    if (seq.m_seq_type_info.m_geometry_scale_max != glm::vec3(0, 0, 0))
    {
        float scaleRatio = std::fabs(rand_LCG_float(&seed));
        // lerp from m_geometry_scale to m_geometry_scale_max based on scaleRatio
        seq.m_seq_type_info.m_geometry_scale =
            glm::mix(seq.m_seq_type_info.m_geometry_scale, seq.m_seq_type_info.m_geometry_scale_max, scaleRatio);
    }
    else
        seq.m_current_geom_scale = seq.m_seq_type_info.m_geometry_scale;

    seq.m_current_animation_scale = seq.m_seq_type_info.m_anim_scale;
    if (seq.m_current_geom_scale.y != 0.0f)
        seq.m_current_animation_scale *= (1.0f / seq.m_current_geom_scale.y);

    seq.m_template = getInitializedSequencerData(seq.m_seq_type_info.m_sequencer_name);
    if (!seq.m_template)
    {
        qCritical() << "Server side sequencer" << seq.m_seq_type_info.m_sequencer_name << "missing for"
                    << entType_filename;
    }
    seq.m_current_state_bits.bits.reset();
    seq.m_anim.prev_frame = 0;
    seqProcessClientInst(seq_handle, 0.0, 0, true);
    // load skinny/fat bodytype animations
    if (!seq.m_seq_type_info.m_bone_scale_fat.isEmpty())
    {
        GeoSet *anim_lst = animLoad(fs, seq.m_seq_type_info.m_bone_scale_fat);
        if (anim_lst)
            seq.m_fat_bodytype_animation = anim_lst;
    }
    if (!seq.m_seq_type_info.m_bone_scale_skinny.isEmpty())
    {
        GeoSet *anim_lst = animLoad(fs, seq.m_seq_type_info.m_bone_scale_skinny);
        if (anim_lst)
            seq.m_skinny_bodytype_animation = anim_lst;
    }
    bonescale_ratio = seq.m_seq_type_info.m_random_bone_scale ? rand_LCG_float(&seed) : 0.0f;
    assert(bonescale_ratio >= -1.0f && bonescale_ratio <= 1.0f);
    changeBoneScale(seq_handle, bonescale_ratio);
    // TODO: if custom per-character lighting is in use, reset it here.
    for (const QByteArray &fxname : seq.m_seq_type_info.m_effect_names)
    {
        // TODO: handle effects attached to a sequencer ?
        qDebug() << "TODO: not applying FX" << fxname;
    }
    assert(seq.m_seq_type_info.m_world_group.isEmpty());
}
SeqTypeDefData *getSeqTypedefByName(SequencerData *data, const QByteArray &name)
{
    for (SeqTypeDefData &entr : data->m_TypeDef)
    {
        if (0 == qstricmp(entr.name.data(), name.data()))
            return &entr;
    }
    return nullptr;
}

bool changeSequencerScale(HSequencerInstance seq, const glm::vec3 &scale)
{
    assert(seq);
    if (scale == glm::vec3(0, 0, 0))
        return false;

    seq->m_current_geom_scale = scale * seq->m_seq_type_info.m_geometry_scale;
    if (seq->m_current_geom_scale.y == 0.0f)
        seq->m_current_animation_scale = seq->m_seq_type_info.m_anim_scale;
    else
        seq->m_current_animation_scale = seq->m_seq_type_info.m_anim_scale * (1.0f / seq->m_current_geom_scale.y);
    seq->m_updated_appearance = 1;
    return 1;
}
Model *findBoneInGeoSet(GeoSet *g_set, int id)
{
    for (Model *v : g_set->subs)
    {
        if (id == v->m_id)
            return v;
    }
    return nullptr;
}
void changeBoneScale(HSequencerInstance seq, float newbonescale)
{
    GeoSet *g_set = nullptr;
    int     bone_idx;

    assert(seq);
    assert(newbonescale >= -1.0f && newbonescale <= 1.0f);
    seq->m_bone_scale_ratio = newbonescale;
    if (newbonescale > 0.0f)
    {
        g_set = seq->m_fat_bodytype_animation;
    }
    else if (newbonescale < 0.0f)
    {
        g_set = seq->m_skinny_bodytype_animation;
    }
    for (bone_idx = 0; bone_idx < 70; ++bone_idx)
    {
        seq->m_bone_scales[bone_idx] = glm::vec3(1, 1, 1);
        if (g_set)
        {
            Model *model = findBoneInGeoSet(g_set, bone_idx);
            if (model)
            {
                glm::vec3 model_scale        = (model->scale - 1.0f) * std::abs(newbonescale);
                seq->m_bone_scales[bone_idx] = model_scale + 1.0f;
            }
        }
        assert(seq->m_bone_scales[bone_idx].x != 0.0f);
    }
    seq->m_updated_appearance = 1;
}

} // namespace SEGS
