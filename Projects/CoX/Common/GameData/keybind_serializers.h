#pragma once

#include <stdint.h>
#include <vector>
#include "keybind_definitions.h"

class BinStore;
class QString;
class KeybindSettings;

using Parse_AllKeyProfiles = std::vector<struct Keybind_Profiles>;
using Parse_AllCommandCategories = std::vector<struct CommandCategory_Entry>;

constexpr const static uint32_t keyprofile_i0_requiredCrc = 0x4513EAB7;
bool loadFrom(BinStore *s,Parse_AllKeyProfiles &target) ;
void saveTo(const Parse_AllKeyProfiles &target,const QString &baseName,bool text_format=false);

constexpr const static uint32_t keycommands_i0_requiredCrc = 0xA918C191;
bool loadFrom(BinStore *s,Parse_AllCommandCategories &target) ;
void saveTo(const Parse_AllCommandCategories &target,const QString &baseName,bool text_format=false);

template<class Archive>
void serialize(Archive &archive, Keybind &k);

template<class Archive>
void serialize(Archive &archive, Keybind_Profiles &kp);

template<class Archive>
void serialize(Archive &archive, KeybindSettings &kbds, uint32_t const version);

void saveTo(const KeybindSettings &target,const QString &baseName,bool text_format=false);

void serializeToDb(const KeybindSettings &data, QString &tgt);
void serializeFromDb(KeybindSettings &data, const QString &src);
