/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once
#include "Common/Runtime/HandleBasedStorage.h"

#include <glm/vec2.hpp>
#include <QString>

#include <stdint.h>

struct ModelModifiers;
struct TextureModifiers;
class QFile;
struct FSWrapper;
namespace SEGS
{
struct GeoSet;

enum class CoHBlendMode : uint8_t
{
    MULTIPLY                = 0,
    MULTIPLY_REG            = 1,
    COLORBLEND_DUAL         = 2,
    ADDGLOW                 = 3,
    ALPHADETAIL             = 4,
    BUMPMAP_MULTIPLY        = 5,
    BUMPMAP_COLORBLEND_DUAL = 6,
    INVALID                 = 255,
};
struct TextureStorage;
struct TextureWrapper
{
    using StorageClass = SEGS::TextureStorage; //tells the handle template to look up
    enum
    {
        ALPHA          = 0x0001,
        RGB8           = 0x0002,
        COMP4          = 0x0004,
        COMP8          = 0x0008,
        DUAL           = 0x0010,
        CLAMP          = 0x0080,
        CUBEMAPFACE    = 0x0200,
        REPLACEABLE    = 0x0400,
        BUMPMAP        = 0x0800,
        BUMPMAP_MIRROR = 0x1000,
    };
    QByteArray detailname;
    QByteArray bumpmap;
    int flags {0};
    glm::vec2 scaleUV0 {0,0};
    glm::vec2 scaleUV1 {0,0};
    CoHBlendMode BlendType = CoHBlendMode(0);
    TextureModifiers *info {nullptr};
};
using HTexture = SingularStoreHandleT<20,12,TextureWrapper>;
struct TextureStorage : public HandleBasedStorage<TextureWrapper>
{
    static TextureStorage &instance() 
    {
        static TextureStorage s_instance;
        return s_instance;
    }
};
void loadTexHeader(FSWrapper *fs, const QByteArray &fname);
} // end of SEGS namespace
