/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup GameData Projects/CoX/Common/GameData
 * @{
 */

#include "StateInterpolator.h"
#include "Entity.h"
#include "Movement.h"
#include <glm/vec3.hpp>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <utility>
#include <array>
#include <vector>
#include <bitset>

using namespace glm;

#pragma pack(push, 1)

#pragma pack(pop)

// struct Entity
// {
//     PosUpdatePair m_pos_updates[64];
//     int pos_up_tgt = 0;
//     std::vector<PosUpdatePair> interpResults;
//     void addPosUpdate(const PosUpdatePair &p)
//     {
//         pos_up_tgt = (pos_up_tgt+1) % 64;
//         m_pos_updates[pos_up_tgt] = p;
//     }
//     void addInterp(const PosUpdatePair &p)
//         interpResults.emplace_back(p);
// };

// Narayana's cows sequence  in use here, why though ?
// Likely a typo in original code, and fibbonacci was meant to be used ?
// a(0) = a(1) = a(2) = 1; thereafter a(n) = a(n-1) + a(n-3).
static float s_coding_sequence[131] ={1,1,1};

// enc values are put in tree order 3, 1, 5, 0, 2, 4, 6
static const uint8_t s_enc_level[7]= {0, 1, 1, 2, 2, 2, 2};
static const int s_breadth_first_order[] = {3, 1, 5, 0, 2, 4, 6, 7, 8};
static const std::pair<uint8_t,uint8_t> s_source_val_idx[] = {
    {0,8}, // Start and End elements
    {0,4}, // Start and middle point
    {4,8}, // Middle and End ...
    {0,2},{2,4},{4,6},{6,8}
};

bool        g_interpolating         = true;
uint8_t     g_interpolation_level   = 2;
uint8_t     g_interpolation_bits    = 1;

static void buildErrorTable()
{
    static bool table_built = false;
    if( !table_built )
    {
        table_built = true;
        for (int i = 3; i < 131; ++i )
        {
            s_coding_sequence[i] = s_coding_sequence[i-1] + s_coding_sequence[i-3];
        }
        for (float &v : s_coding_sequence )
            v = std::sqrt(v) / 20.0f;
    }
}

float get_interpolator_perturbation(int16_t val,int level)
{
    buildErrorTable();
    if ( val )
        return (2 * (val >= 0) - 1) * s_coding_sequence[int(std::abs(val))]*(1<<level);

    return 0.0f;
}

static int16_t encodePerturbation(float error_val, float &value, int level)
{
    if(error_val==0.0f)
    {
        value=0;
        return 0;
    }

    buildErrorTable();

    // not using binary search since the array is small-ish, and most looked-up values are in the beginning anyway.
    error_val /= (1<<level);
    int16_t signval = (2 * (error_val >= 0) - 1);
    error_val = std::abs(error_val);

    for(int16_t i=1; i<131; ++i)
    {
        if(error_val < s_coding_sequence[i])
        {
            value = signval * s_coding_sequence[i];
            return signval * i;
        }
    }

    value = signval * s_coding_sequence[130];

    return signval * 130;
}

// Position interpolation values
// 0  1  2  3  4  5  6     7  8
// V(3) = V(7) + V(8) / 2  + perturb(0) * 1
// V(1) = V(7) + V(3) / 2  + perturb(1) * 2
// V(5) = V(3) + V(8) / 2  + perturb(2) * 2
// V(0) = V(7) + V(1) / 2  + perturb(3) * 4
// V(2) = V(1) + V(3) / 2  + perturb(4) * 4
// V(4) = V(3) + V(5) / 2  + perturb(5) * 4
// V(6) = V(5) + V(8) / 2  + perturb(6) * 4
void interpolatePosUpdates(Entity *e, std::array<BinTreeEntry,7> &server_pos_update)
{
    PosUpdate   prev_pos;
    PosUpdate   cur_pos;
    PosUpdate   *cur_pos_update;
    vec3        pos_update_arr[9];
    vec3        midpoint;
    float       lerp_factor;
    uint32_t    dt;
    bool        needs_update[7];
    int         prev_timestamp;
    int         num_interp_coeffs = 0;

    cur_pos             = e->m_pos_updates[e->m_update_idx];
    prev_pos            = e->m_pos_updates[(e->m_update_idx + -1 + 64) % 64];
    prev_timestamp      = prev_pos.m_timestamp;
    pos_update_arr[7]   = prev_pos.m_position;
    pos_update_arr[8]   = cur_pos.m_position;

    for(uint8_t i = 0; i < 7; ++i)
    {
        if(server_pos_update[i].m_has_height || server_pos_update[i].m_has_other)
        {
            midpoint = (pos_update_arr[s_breadth_first_order[s_source_val_idx[i].first]] + pos_update_arr[s_breadth_first_order[s_source_val_idx[i].second]]) * 0.5f;
            const int level = s_enc_level[i];
            if( server_pos_update[i].m_has_height )
                midpoint.y += get_interpolator_perturbation(server_pos_update[i].y,level);
            if( server_pos_update[i].m_has_other )
            {
                midpoint.x += get_interpolator_perturbation(server_pos_update[i].x,level);
                midpoint.z += get_interpolator_perturbation(server_pos_update[i].z,level);
            }
            pos_update_arr[s_breadth_first_order[i]] = midpoint;
            needs_update[s_breadth_first_order[i]] = true;
            ++num_interp_coeffs;
        }
        else
            needs_update[s_breadth_first_order[i]] = false;
    }

    addPosUpdate(*e, prev_pos); //addInterp(*e, prev_pos);

    if( num_interp_coeffs )
    {
        dt = cur_pos.m_timestamp - prev_timestamp;
        // interpolate into up to 64 updates
        for (int i = 0; i < 7; ++i )
        {
            if( needs_update[i] )
            {
                lerp_factor = (float)(i + 1) / 8.0f;
                PosUpdate step;
                cur_pos_update = &step;
                cur_pos_update->m_timestamp = prev_timestamp + (dt * (i + 1) / 8);
                cur_pos_update->m_position = pos_update_arr[i];
                for (int j = 0; j < 3; ++j )
                {
                    cur_pos_update->m_pyr_angles[j] = glm::mix(cur_pos_update->m_pyr_angles[j], prev_pos.m_pyr_angles[j], lerp_factor);
                }
                addPosUpdate(*e, step); //addInterp(*e, step);
            }
        }
    }
    addPosUpdate(*e, cur_pos); //addInterp(*e, cur_pos);
}

// Take a set of PositionUpdates S . . . . . . . E
// Assume client will use linear interpolation for in-between values.
// Encode fixups for in-between values
std::array<BinTreeEntry,7> interpolateBinTree(std::array<PosUpdate, 64> vals, float min_error)
{
    std::array<BinTreeEntry, 7> enc;
    std::array<PosUpdate, 7>    work_area;

    // We only really need the last 7 posupdates
    std::copy(vals.end()-7, vals.end(), work_area.begin());

    for(int i=0; i<7; ++i)
    {
        vec3 quant_delta; // quantized error values will be put here
        // linear interpolation between value's sources
        vec3 predicted = (work_area[s_source_val_idx[i].first].m_position + work_area[s_source_val_idx[i].second].m_position)/2.0f;
        // location of calculated value in the array
        uint8_t target_idx = (s_source_val_idx[i].first + s_source_val_idx[i].second)/2;
        // linear interpolation will be wrong by 'this' much
        vec3 error_val = work_area[target_idx].m_position - predicted;

        // if the error is small enough, setting it to 0 will cause encodePerturbation to return 0 -> mark value as not-sent
        if(std::abs(error_val.x) < min_error)
            error_val.x=0;
        if(std::abs(error_val.y) < min_error)
            error_val.y=0;
        if(std::abs(error_val.z) < min_error)
            error_val.z=0;

        enc[i].x = encodePerturbation(error_val.x, quant_delta.x, s_enc_level[i]);
        enc[i].y = encodePerturbation(error_val.y, quant_delta.y, s_enc_level[i]);
        enc[i].z = encodePerturbation(error_val.z, quant_delta.z, s_enc_level[i]);

        // update the work area value with actual value we'll have after decoding
        work_area[target_idx].m_position = predicted + quant_delta;
    }

    /////////////////////////////////////////////
    // Enc X means Encoding for Value error at index X
    // Nodes used to calculate enc - Enc location is Halfway between those nodes
    //  |  ----------------------/
    //  | /
    // {0,8}    - Enc 3 needs  S,E => None    - both given
    // {0,4}    - Enc 1 needs  S,3 => 3
    // {4,8}    - Enc 5 needs  3,E => 3
    // {0,2}    - Enc 0 Needs  S,1 => 1
    // {2,4}    - Enc 2 Needs  1,3 => 1,3
    // {4,6}    - Enc 4 Needs  3,5 => 3,5
    // {6,8}    - Enc 6 Needs  5,E => 5
    ////////////////////////////////////
    // When traversing in reverse tree order
    // Enc 6 -> needs 5, marks it, 5 needs 3, marks it -> whole path to tree root marked
    // Enc 4 -> needs 5, marks it, 5 needs 3, marks it -> whole path to tree root marked
    // Enc 2 -> needs 1, marks it, 1 needs 3, marks it -> whole path to tree root marked
    //3, 1, 5, 0, 2, 4, 6
    int required_value_order[] = {2,2,1,1,0,0,0};
    for(int to_check=enc.size()-1; to_check>0; --to_check) {
        BinTreeEntry &checked_parent(enc[required_value_order[to_check]]);
        if(enc[to_check].x && !checked_parent.x) {
            checked_parent.x=1;
        }
        if(enc[to_check].y && !checked_parent.y) {
            checked_parent.y=1;
        }
        if(enc[to_check].z && !checked_parent.z) {
            checked_parent.z=1;
        }
    }

    // mark
    for(BinTreeEntry & v : enc) {
        v.m_has_height = v.y != 0;
        v.m_has_other = v.x != 0 || v.z != 0;
    }

    return enc;
}

void entCalcInterp(Entity *ent, glm::mat4 *mat4, uint32_t time, glm::vec3 *next_pyr)
{
    std::array<PosUpdate, 64> posupdate_arr = ent->m_pos_updates;
    PosUpdate *last=nullptr;
    PosUpdate *next;
    glm::vec3 pos;
    glm::vec3 pyr;
    int i;

    for(i = 0; i < 64; ++i)
    {
        last = &posupdate_arr[(64 + ent->m_update_idx - i) % 64];
        if(time > posupdate_arr[(64 + ent->m_update_idx - i) % 64].m_timestamp)
            break;
    }

    if(i > 0 && i < 64 && last->m_timestamp)
    {
        next = &posupdate_arr[(ent->m_update_idx - i + 64 + 1) % 64];
        float timestep = (float)(unsigned int)(posupdate_arr[(ent->m_update_idx - i + 64 + 1) % 64].m_timestamp - last->m_timestamp);
        float ratio = (float)(time - last->m_timestamp) / timestep;

        for (int j = 0; j < 3; ++j)
        {
            pos[j] = (1.0f - ratio) * last->m_position[j] + ratio * next->m_position[j];
            //pyr[j] = last->m_pyr_angles[j].lerp(next->m_pyr_angles[j], ratio);
            pyr[j] = glm::mix(last->m_pyr_angles[j], next->m_pyr_angles[j], ratio);
        }
    }
    else
    {
        next = &posupdate_arr[ent->m_update_idx];
        last = &posupdate_arr[(ent->m_update_idx + 64 - 1) % 64];

        if(ent->m_type == EntType::CAR || !ent->m_is_fading) // possibly: e->m_fading_direction != FadeDirection::Out
        {
            last->m_position = next->m_position;
            last->m_pyr_angles = next->m_pyr_angles;
            pos = next->m_position;
            next->m_timestamp = time;
        }
        else
        {
            float scale = float(time - last->m_timestamp) / std::fabs(float(next->m_timestamp - last->m_timestamp));
            glm::vec3 distance = next->m_position - last->m_position;
            glm::vec3 magnitude = distance * scale;
            pos = magnitude + last->m_position;
        }
        pyr = next->m_pyr_angles;
    }

    //quat = fromCoHYpr(pos); // below does this
    transformFromYPRandTranslation(*mat4, pyr, pos);

    if(next_pyr)
        *next_pyr = pyr;
}

int storeBinTreesResult(BitStream &bs, const std::array<BinTreeEntry, 7> &bintree)
{
    // this least means that midpoint ( idx 0 ) depends on midpoint, but that's check with idx==0
    // then checking S - * - M - - - E   (idx 1)  depends on actual midpoint, and we check it
    // then checking S - - - M - * - E   (idx 2)  same as above
    // then checking S * - - M - - - E   (idx 3)  checking that we had SM/2 midpoint
    // then checking S * - - M - - - E   (idx 4)  checking that we had SM/2 midpoint
    // ...
    static const uint8_t dependency_indices[] = {0,0,0,1,1,2,2};
    static const uint8_t tree_depth_bits_mod[8] = { 1,2,2,3,3,3,3 };
    int     res = 0;
    uint8_t num_bits;
    uint8_t base_bitcount;

    std::array<BinTreeEntry, 7> tree_copy = bintree;
    base_bitcount = g_interpolation_bits + 5;

    if(tree_copy[0].m_has_height && tree_copy[0].m_has_other)
        bs.StoreBits(1, 1); // we have both values
    else
        bs.StoreBits(1, tree_copy[0].m_has_height); // otherwise it is assumed that has_other is true

    for (int idx = 0; idx < 7; ++idx )
    {
        if( (idx && !tree_copy[dependency_indices[idx]].m_has_height) || tree_depth_bits_mod[idx] > g_interpolation_level )
            tree_copy[idx].m_has_height = 0; // mark dependency as missing
        else
        {
            if( idx )
                bs.StoreBits(1, tree_copy[idx].m_has_height);

            if( tree_copy[idx].m_has_height )
            {
                // make sure we don't encounter a case when we try to send 0 or less bits.
                assert(base_bitcount > tree_depth_bits_mod[idx]);
                num_bits = base_bitcount - tree_depth_bits_mod[idx];
                res = 1;

                bs.StoreBits(1, tree_copy[idx].y < 0);
                bs.StoreBits(num_bits, std::abs(tree_copy[idx].y));
            }
        }
    }

    for (int idx = 0; idx < 7; ++idx )
    {
        // check if the 'source' value for the one located at idx has xz values,
        if( (idx && !tree_copy[dependency_indices[idx]].m_has_other) || tree_depth_bits_mod[idx] > g_interpolation_level )
            tree_copy[idx].m_has_other = false;
        else
        {
            if( idx )
                bs.StoreBits(1, tree_copy[idx].m_has_other);

            if( tree_copy[idx].m_has_other )
            {
                num_bits = base_bitcount - tree_depth_bits_mod[idx];
                res = 1;

                bs.StoreBits(1, tree_copy[idx].x < 0);
                bs.StoreBits(num_bits, std::abs(tree_copy[idx].x));
                bs.StoreBits(1, tree_copy[idx].z < 0);
                bs.StoreBits(num_bits, std::abs(tree_copy[idx].z));
            }
        }
    }

    return res;
}

int runTest(Entity &e)
{
    // Linear move from 0 to 2, over 10 time units
    std::array<BinTreeEntry,7> tgt;
    std::array<PosUpdate, 64> pos_vals;
    int t=0;
    int t_start=1100;

    for(PosUpdate &puv : pos_vals) {
        puv.m_position = {
            std::sin((glm::pi<float>() / 12.0f) * t) * 10,
            std::sin((glm::pi<float>() / 22.0f) * t) * 10,
            std::sin((glm::pi<float>() /  9.0f) * t) * 10
        };
        puv.m_timestamp = t_start+t*10;
        t++;
    }

    tgt = interpolateBinTree(pos_vals, 0.02f);

    // add start and end to prime interpolator.
    addPosUpdate(e, pos_vals.front());
    addPosUpdate(e, pos_vals.back());

    interpolatePosUpdates(&e, tgt);

    vec3 errsum;
    for(int i=0; i<64; ++i)
    {
        errsum +=  glm::abs(e.m_pos_updates[i].m_position - pos_vals[i].m_position);
    }
    errsum = errsum/9.0f;
    printf("After transition - L1 error is %f %f %f\n",errsum.x,errsum.y,errsum.z);
    return 0;
}

//! @}
