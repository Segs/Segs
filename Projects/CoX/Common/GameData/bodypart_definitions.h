/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <QtCore/QString>
#include <array>
#include <vector>

enum BoneIds
{
    MAX_BONES=70
};

struct BodyPart_Data
{
    QByteArray m_Name;
    QByteArray m_GeoName;
    QByteArray m_TexName;
    QByteArray m_BaseName;
    int m_BoneCount;
    int m_InfluenceCost;
    // Transient data.
    std::array<int,2> boneIndices;
    int part_idx;
};
struct BodyPartsStorage
{
    std::vector<BodyPart_Data> m_parts;

    BodyPart_Data *getBodyPartFromName(const QByteArray &name);

    void postProcess();
};

namespace SEGS
{
bool legitBone(int idx);
const char *boneName(int idx);
}
