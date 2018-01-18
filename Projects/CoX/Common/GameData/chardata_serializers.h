#pragma once

class QString;
struct CharacterData;

template<class Archive>
void serialize(Archive & archive, CharacterData & m);

void saveTo(const CharacterData &target,const QString &baseName,bool text_format=false);
