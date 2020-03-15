#pragma once
#include "Common/Runtime/HandleBasedStorage.h"
#include <GameData/anim_definitions.h>

#include <QHash>

namespace SEGS
{

class AnimationStorage : public HandleBasedStorage<AnimTrack>
{
public:
    static AnimationStorage &instance()
    {
        static AnimationStorage instance;
        return instance;
    }

};
using HAnimationTrack = AnimationStorage::HType;

class AnimationEngine
{
private:
    AnimationEngine() = default;
public:
    static AnimationEngine &get();

    QHash<QByteArray,HAnimationTrack> m_loaded_tracks;
};

} // end of SEGS namespace
