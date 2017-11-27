#pragma once

#include <stdint.h>
#include <vector>

class BinStore;
class QString;

using Parse_AllKeyProfiles = std::vector<struct KeyProfiles_Entry>;
using Parse_AllCommandCategories = std::vector<struct CommandCategory_Entry>;

constexpr const static uint32_t keyprofile_i0_requiredCrc = 0x4513EAB7;
bool loadFrom(BinStore *s,Parse_AllKeyProfiles &target) ;
void saveTo(const Parse_AllKeyProfiles &target,const QString &baseName,bool text_format=false);

constexpr const static uint32_t keycommands_i0_requiredCrc = 0xA918C191;
bool loadFrom(BinStore *s,Parse_AllCommandCategories &target) ;
void saveTo(const Parse_AllCommandCategories &target,const QString &baseName,bool text_format=false);
