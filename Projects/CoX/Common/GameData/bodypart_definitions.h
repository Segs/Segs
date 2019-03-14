/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <QtCore/QString>
#include <array>

struct BodyPart_Data
{
    QByteArray m_Name;
    QByteArray m_GeoName;
    QByteArray m_TexName;
    QByteArray m_BaseName;
    std::array<int,2> boneIndices;
    int part_idx;
    int m_BoneCount;
    int m_InfluenceCost;
};
