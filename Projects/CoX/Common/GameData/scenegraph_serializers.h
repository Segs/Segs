#pragma once
#include <stdint.h>
#include <vector>

class BinStore;
class QString;
struct SceneGraph_Data;

static constexpr uint32_t scenegraph_i0_requiredCrc=0xD3432007;
bool loadFrom(BinStore *s,SceneGraph_Data &target);
bool loadFrom(const QString &filepath,SceneGraph_Data &target);

void saveTo(const SceneGraph_Data &target,const QString &baseName,bool text_format=false);
