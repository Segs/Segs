#pragma once

#include <stdint.h>

struct PlayerData;

template<class Archive>
void serialize(Archive &archive, PlayerData &fr, uint32_t const version);


