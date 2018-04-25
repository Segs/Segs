/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#pragma once
#include <QtCore/QString>
#include <array>

struct BodyPart_Data
{
    QString m_Name;
    std::array<int,2> boneIndices;
    int part_idx;
    int m_BoneCount;
    int m_InfluenceCost;
    QString m_GeoName;
    QString m_TexName;
    QString m_BaseName;
};
