#pragma once

class BinStore;
class QString;
struct TextureAnim_Data;

template<class Archive>
void serialize(Archive & archive, TextureAnim_Data & m);

bool loadFrom(BinStore *s,TextureAnim_Data &target);
void saveTo(const TextureAnim_Data &target,const QString &baseName,bool text_format=false);
