/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include <QMap>

enum FloatingInfoMsgKey : uint32_t
{
    FloatingMsg_NotEnoughEndurance  = 0,
    FloatingMsg_OutOfRange          = 1,
    FloatingMsg_Recharging          = 2,
    FloatingMsg_NoEndurance         = 3,
    FloatingMsg_Leveled             = 4,
    FloatingMsg_FoundClue           = 5,
    FloatingMsg_FoundEnhancement    = 6,
    FloatingMsg_FoundInspiration    = 7,
    FloatingMsg_MissionComplete     = 8,
    FloatingMsg_TaskForceComplete   = 9,
    FloatingMsg_MissionFailed       = 10,
};

enum FloatingInfoStyle : int
{
    FloatingInfo_Damage         = 0,
    FloatingInfo_Chat           = 1,
    FloatingInfo_Chat_Private   = 2,
    FloatingInfo_Icon           = 3,
    FloatingInfo_Emote          = 4,
    FloatingInfo_Info           = 5,
    FloatingInfo_Attention      = 6,
    FloatingInfo_AFK            = 7,
    FloatingInfo_8              = 8,
    FloatingInfo_Count          = 9,
};

static const QMap<FloatingInfoMsgKey, QString> FloatingInfoMsg = {
    {FloatingMsg_NotEnoughEndurance,"FloatNotEnoughEndurance"}, // FloatingInfo_Info
    {FloatingMsg_OutOfRange,        "FloatOutOfRange"},         // FloatingInfo_Info
    {FloatingMsg_Recharging,        "FloatRecharging"},         // FloatingInfo_Info
    {FloatingMsg_NoEndurance,       "FloatNoEndurance"},        // FloatingInfo_Info
    {FloatingMsg_Leveled,           "FloatLeveled"},            // FloatingInfo_Attention
    {FloatingMsg_FoundClue,         "FloatFoundClue"},          // FloatingInfo_Attention
    {FloatingMsg_FoundEnhancement,  "FloatFoundEnhancement"},   // FloatingInfo_Attention
    {FloatingMsg_FoundInspiration,  "FloatFoundInspiration"},   // FloatingInfo_Attention
    {FloatingMsg_MissionComplete,   "FloatMissionComplete"},    // FloatingInfo_Attention
    {FloatingMsg_TaskForceComplete, "FloatTaskForceComplete"},  // FloatingInfo_Attention
    {FloatingMsg_MissionFailed,     "FloatMissionFailed"},      // FloatingInfo_Attention
};
