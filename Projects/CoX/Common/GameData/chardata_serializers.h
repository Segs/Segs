#pragma once
#include <stdint.h>

class QString;
struct CharacterData;

template<class Archive>
void serialize(Archive & archive, CharacterData & m, uint32_t const version);

void saveTo(const CharacterData &target,const QString &baseName,bool text_format=false);

void serializeToDb(const CharacterData &data, QString &tgt);
void serializeFromDb(CharacterData &data, const QString &src);
