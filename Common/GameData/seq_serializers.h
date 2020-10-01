/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "seq_definitions.h"
#include <stdint.h>

class BinStore;
class QString;

constexpr const static uint32_t seqencerlist_i0_requiredCrc = 0x117184DE;
bool loadFrom(BinStore *s, struct SequencerList &target);
void saveTo(const SequencerList &target, const QString &baseName, bool text_format=false);

bool loadFrom(const QString &path, SequencerTypeMap &target);
void saveTo(const SequencerTypeMap &target, const QString &baseName, bool text_format = false);
