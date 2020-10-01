#include "anim_definitions.h"
#include "Common/Runtime/AnimationEngine.h" // allows HAnimationTrack `->` calls

using namespace SEGS;

BoneAnimTrack *getTrackForBone(HAnimationTrack trk, int bone)
{
    for(BoneAnimTrack & track : trk->m_bone_tracks)
    {
        if(bone==track.tgtBoneOrTexId)
            return &track;
    }
    return nullptr;
}
