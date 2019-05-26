#include "bodypart_definitions.h"

using namespace SEGS;

//TODO: most of the bone related data should be loaded and not hardcoded in here.
static const char * s_bone_names[70] ={
    "HIPS",   // 0
    "WAIST",
    "CHEST",
    "NECK",
    "HEAD",
    "COL_R",
    "COL_L",
    "UARMR",
    "UARML",
    "LARMR",
    "LARML",  // 10
    "HANDR",
    "HANDL",
    "F1_R",
    "F1_L",
    "F2_R",
    "F2_L",
    "T1_R",
    "T1_L",
    "T2_R",
    "T2_L",  //20
    "T3_R",
    "T3_L",
    "ULEGR",
    "ULEGL",
    "LLEGR",
    "LLEGL",
    "FOOTR",
    "FOOTL",
    "TOER",
    "TOEL",  //30
    "FACE",
    "DUMMY",
    "BREAST",
    "BELT",
    "GLOVEL",
    "GLOVER",
    "BOOTL",
    "BOOTR",
    "RINGL",
    "RINGR", //40
    "WEPL",
    "WEPR",
    "HAIR",
    "EYES",
    "EMBLEM",
    "SPADL",
    "SPADR",
    "BACK",
    "NECKLINE",
    "CLAWL",  //50
    "CLAWR",
    "GUN",
    "RWING1",
    "RWING2",
    "RWING3",
    "RWING4",
    "LWING1",
    "LWING2",
    "LWING3",
    "LWING4", //60
    "MYSTIC",
    "SLEEVEL",
    "SLEEVER",
    "ROBE",
    "BENDMYSTIC", // 65
    // the rest are unnamed
    nullptr
};
static int boneNameToIdx(const QByteArray &name)
{
    if ( name.isEmpty() )
        return -1;

    int i;
    QByteArray upcase=name.toUpper();
    for(i=0; s_bone_names[i]!= nullptr; ++i)
    {
        if(upcase.startsWith(s_bone_names[i]))
            break;
    }
    if ( i == 3 )
    {
        if ( upcase.startsWith("NECKLINE") )
            return 49;
    }
    return i;
}

void BodyPartsStorage::postProcess()
{
    for (int i = 0; i < m_parts.size(); ++i )
    {
        BodyPart_Data &bp(m_parts[i]);
        if ( !bp.m_BoneCount )
            qFatal("body part %s with no associated bones!", qPrintable(bp.m_Name));
        bp.part_idx = i;
        if ( bp.m_BoneCount == 2 )
        {
            bp.boneIndices[0] = boneNameToIdx(bp.m_GeoName + "R");
            bp.boneIndices[1] = boneNameToIdx(bp.m_GeoName + "L");
        }
        else
        {
            bp.boneIndices[0] = boneNameToIdx(bp.m_GeoName);
        }
    }
}

BodyPart_Data *BodyPartsStorage::getBodyPartFromName(const QByteArray &name)
{
    for (BodyPart_Data &bp : m_parts )
    {
        if ( 0==bp.m_Name.compare(name,Qt::CaseInsensitive) )
            return &bp;
    }
    return nullptr;
}

namespace SEGS
{

bool legitBone(int idx)
{
    return idx >= 0 && idx < MAX_BONES;
}
/**
 * @brief convert a valid bone index to it's name
 * @param idx [0,MAX_BONES)
 * @return a name for give bone, or nullptr
 */
const char *boneName(int idx)
{
    if (!legitBone(idx))
        return nullptr;
    return s_bone_names[idx];
}

} //end of SEGS namespace
