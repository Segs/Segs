#pragma once

class QString;
struct EntityData;

template<class Archive>
void serialize(Archive & archive, EntityData & m);

void saveTo(const EntityData &target,const QString &baseName,bool text_format=false);
