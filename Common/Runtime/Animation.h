#pragma once
#include "Common/Runtime/HandleBasedStorage.h"

struct FSWrapper;
class QByteArray;
namespace SEGS
{
struct AnimTrack;
using HAnimationTrack = SingularStoreHandleT<20,12,AnimTrack>;
struct GeoSet;
}

SEGS::HAnimationTrack getOrLoadAnimationTrack(const QByteArray &name);
SEGS::GeoSet *animLoad(FSWrapper &fs, const QByteArray &filename, bool background_load =false, bool header_only =false);
