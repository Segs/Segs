#include "StateInterpolator.h"
#include "Entity.h"
#include <glm/vec3.hpp>
#include <glm/common.hpp>
#include <glm/glm.hpp>

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
struct bintree_in {
    int16_t x=0,y=0,z=0;
    uint8_t has_height :1;
    uint8_t has_other  :1;
};
#pragma pack(pop)

//struct Entity {
//    PosUpdatePair m_pos_updates[64];
//    int pos_up_tgt=0;
//    std::vector<PosUpdatePair> interpResults;
//    void addPosUpdate(const PosUpdatePair &p) {
//        pos_up_tgt = (pos_up_tgt+1) % 64;
//        m_pos_updates[pos_up_tgt] = p;
//    }
//    void addInterp(const PosUpdatePair &p) {
//        interpResults.emplace_back(p);
//    }
//};
// Narayana's cows sequence  in use here, why though ?
// Likely a typo in original code, and fibbonacci was meant to be used ?
// a(0) = a(1) = a(2) = 1; thereafter a(n) = a(n-1) + a(n-3).
static float s_coding_sequence[131] ={1,1,1};
static void buildErrorTable() {
    static bool dword_769AD4=false;
    if ( !dword_769AD4 )
    {
        dword_769AD4 = true;
        for (int i = 3; i < 131; ++i )
        {
            s_coding_sequence[i] = s_coding_sequence[i-1] + s_coding_sequence[i-3];
        }
        for (float &v : s_coding_sequence )
            v = std::sqrt(v) / 20.0f;
    }
};
float get_interpolator_perturbation(int16_t a1,int level)
{
    buildErrorTable();
    if ( a1 )
        return (2 * (a1 >= 0) - 1) * s_coding_sequence[int(std::abs(a1))]*(1<<level);
    return 0.0f;
}
static int16_t encodePerturbation(float error_val,float &value,int level) {
    if(error_val==0.0f) {
        value=0;
        return 0;
    }
    buildErrorTable();
    // not using binary search since the array is small-ish, and most looked-up values are in the beginning anyway.
    error_val /= (1<<level);
    int16_t signval = (2 * (error_val >= 0) - 1);
    error_val = std::abs(error_val);
    for(int16_t i=1; i<131; ++i) {
        if(error_val<s_coding_sequence[i]) {
            value = signval*s_coding_sequence[i];
            return signval*i;
        }
    }
    value = signval*s_coding_sequence[130];
    return signval*130;
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
void interpolate_pos_updates(Entity *a1, std::array<bintree_in,7> &a2)
{
    PosUpdate *current_pos_update;
    float lerp_factor;
    unsigned int v11;
    vec3 midpoint;
    bool v17[7];
    int prev_timestamp;
    PosUpdate prev_update;
    vec3 v21[9];
    PosUpdate current;
    int num_interpolation_coeffs;

    static const int breadth_first_order[] = {3, 1, 5, 0, 2, 4, 6, 7, 8};
    static const std::pair<int,int> s_source_idx_0[] = {{7,8},{7,0},{0,8},{7,1},{1,0},{0,2},{2,8}};
    static const uint8_t byte_680958[7]= {0, 1, 1, 2, 2, 2, 2};

    current = a1->m_pos_updates[a1->m_update_idx];
    prev_update = a1->m_pos_updates[(a1->m_update_idx + -1 + 64) % 64];
    prev_timestamp = prev_update.m_timestamp;
    num_interpolation_coeffs = 0;
    v21[7] = prev_update.posvec;
    v21[8] = current.posvec;
    for (uint8_t i = 0; i < 7; ++i )
    {
        if ( a2[i].has_height || a2[i].has_other)
        {
            midpoint = (v21[breadth_first_order[s_source_idx_0[i].first]] + v21[breadth_first_order[s_source_idx_0[i].second]]) * 0.5f;
            const int level = byte_680958[i];
            if ( a2[i].has_height )
                midpoint.y += get_interpolator_perturbation(a2[i].y,level);
            if ( a2[i].has_other )
            {
                midpoint.x += get_interpolator_perturbation(a2[i].x,level);
                midpoint.z += get_interpolator_perturbation(a2[i].z,level);
            }
            v21[breadth_first_order[i]] = midpoint;
            v17[breadth_first_order[i]] = true;
            ++num_interpolation_coeffs;
        }
        else
        {
            v17[breadth_first_order[i]] = false;
        }
    }
    a1->addInterp(prev_update);
    if ( num_interpolation_coeffs )
    {
        v11 = current.m_timestamp - prev_timestamp;
        // interpolate into upto 8 updates
        for (int i = 0; i < 7; ++i )
        {
            if ( v17[i] )
            {
                lerp_factor = (float)(i + 1) / 8.0f;
                PosUpdate step;
                current_pos_update = &step;
                current_pos_update->m_timestamp = prev_timestamp + (v11 * (i + 1) / 8);
                current_pos_update->posvec = v21[i];
                for (int j = 0; j < 3; ++j )
                {
                    current_pos_update->PitchYawRoll[j] = prev_update.PitchYawRoll[j].lerp(current.PitchYawRoll[j],lerp_factor);
                }
                a1->addInterp(step);
            }
        }
    }
    a1->addInterp(current);
}

// Take a set of PositionUpdates S . . . . . . . E
// Assume client will use linear interpolation for in-between values.
// Encode fixups for in-between values
std::array<bintree_in,7> testEncVec(std::array<PosUpdate,9> vals,float min_error)
{
    std::array<bintree_in,7> enc;
    std::array<PosUpdate,9> work_area=vals;
    static const std::pair<uint8_t,uint8_t> src_val_idx[] = {
        {0,8}, // Start and End elements
        {0,4}, // Start and middle point
        {4,8}, // Middle and End ...
        {0,2},{2,4},{4,6},{6,8}
    };
    // enc values are put in tree order 3, 1, 5, 0, 2, 4, 6
    static const uint8_t enc_level[7]= {0, 1, 1, 2, 2, 2, 2};
    for(int i=0; i<7; ++i) {
        vec3 quant_delta; // quantized error values will be put here
        // linear interpolation between value's sources
        vec3 predicted = (work_area[src_val_idx[i].first].posvec+work_area[src_val_idx[i].second].posvec)/2.0f;
        // location of calculated value in the array
        uint8_t target_idx = (src_val_idx[i].first + src_val_idx[i].second)/2;
        // linear interpolation will be wrong by 'this' much
        vec3 error_val = work_area[target_idx].posvec - predicted;
        // if the error is small enough, setting it to 0 will cause encodePerturbation to return 0 -> mark value as not-sent
        if(std::abs(error_val.x)<min_error)
            error_val.x=0;
        if(std::abs(error_val.y)<min_error)
            error_val.y=0;
        if(std::abs(error_val.z)<min_error)
            error_val.z=0;
        enc[i].x = encodePerturbation(error_val.x,quant_delta.x,enc_level[i]);
        enc[i].y = encodePerturbation(error_val.y,quant_delta.y,enc_level[i]);
        enc[i].z = encodePerturbation(error_val.z,quant_delta.z,enc_level[i]);
        // update the work area value with actual value we'll have after decoding
        work_area[target_idx].posvec = predicted + quant_delta;
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
        bintree_in &checked_parent(enc[required_value_order[to_check]]);
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
    for(bintree_in & v : enc) {
        v.has_height = v.y!=0;
        v.has_other = v.x!=0 || v.z!=0;
    }
    return enc;
}
int runTest() {
    Entity ent;
    // Linear move from 0 to 2, over 10 time units
    std::array<bintree_in,7> tgt;
    std::array<PosUpdate,9> pos_vals;
    int t=0;
    int t_start=1100;
    for(PosUpdate &puv : pos_vals) {
        puv.posvec = {
            std::sin(float(M_PI/12)*t)*10,
            std::sin(float(M_PI/22)*t)*10,
            std::sin(float(M_PI/8)*t)*10
        };
        puv.m_timestamp = t_start+t*10;
        t++;
    }
    tgt=testEncVec( pos_vals,0.02f );
    ent.interpResults.clear();
    // add start and end to prime interpolator.
    ent.addPosUpdate(pos_vals.front());
    ent.addPosUpdate(pos_vals.back());
    interpolate_pos_updates(&ent,tgt);
    vec3 errsum;
    for(int i=0; i<9; ++i) {
        errsum +=  glm::abs(ent.interpResults[i].posvec-pos_vals[i].posvec);
    }
    errsum = errsum/9.0f;
    printf("After transition - L1 error is %f %f %f\n",errsum.x,errsum.y,errsum.z);
    return 0;
}
