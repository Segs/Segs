/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
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
