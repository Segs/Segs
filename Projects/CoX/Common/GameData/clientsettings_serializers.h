#pragma once
#include <stdint.h>

class QString;
struct ClientSettingsData;

template<class Archive>
void serialize(Archive & archive, ClientSettingsData & m, uint32_t const version);

void saveTo(const ClientSettingsData &target,const QString &baseName,bool text_format=false);

void serializeToDb(const ClientSettingsData &data, QString &tgt);
void serializeFromDb(ClientSettingsData &data, const QString &src);
