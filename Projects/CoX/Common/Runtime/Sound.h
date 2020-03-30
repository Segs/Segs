/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */
#pragma once
#include "Common/Runtime/HandleBasedStorage.h"

#include <glm/vec2.hpp>

#include <QByteArray>

namespace SEGS
{
struct SoundStorage;
struct SoundInfo
{
    using StorageClass = SEGS::SoundStorage; //tells the handle template to look up
    enum {
        Exclude = 1 //!< If this flag is set, sound 'source' is a 'muffler' that will quite down other close sources
    };

    QByteArray name;
    float radius=0;
    float ramp_feet=0;
    uint16_t flags;
    uint8_t vol=0; // sound source volume 0-255
};
using HSound = SingularStoreHandleT<20,12,SoundInfo>;
struct SoundStorage : public HandleBasedStorage<SoundInfo>
{
    static SoundStorage &instance() 
    {
        static SoundStorage s_instance;
        return s_instance;
    }
};

}
