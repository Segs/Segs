#pragma once

class BinStore;
class QString;
struct CharacterData;

template<class Archive>
void serialize(Archive & archive, CharacterData & m);

bool loadFrom(BinStore *s,CharacterData &target);
void saveTo(const CharacterData &target,const QString &baseName,bool text_format=false);
