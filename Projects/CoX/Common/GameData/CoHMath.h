/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/constants.hpp>
#include <algorithm>

// AngleRadians for use in PosUpdate etc
// TODO: we can replace this with glm::vec and other methods
struct AngleRadians
{
    static AngleRadians fromDeg(float deg) { return AngleRadians(deg*glm::pi<float>()/180.0f);}
    float toDeg() { return AngleRadians((v*180.0f)/ glm::pi<float>()).v;}
    explicit AngleRadians(float x=0.0f) : v(x) {}
    AngleRadians operator-(const AngleRadians&ot) const
    {
        AngleRadians result(v);
        return result-=ot;
    }
    AngleRadians operator-() const {
        return AngleRadians(-v);
    }
    float operator/(AngleRadians &other) const {
        return v/other.v;
    }
    AngleRadians operator+(const AngleRadians &ot) const
    {
        AngleRadians result(v);
        result+=ot;
        result.fixup();
        return result;
    }
    AngleRadians operator*(float scale) const
    {
        return AngleRadians(v*scale);
    }
    AngleRadians &operator*=(float scale)
    {
        v*=scale;
        return *this;
    }
    AngleRadians &operator+=(const AngleRadians &ot)
    {
        v += ot.v;
        fixup();
        return *this;
    }
    AngleRadians &operator-=(const AngleRadians &ot)
    {
        v -= ot.v;
        fixup();
        return *this;
    }
    bool operator==(float other) const { return v == other; }
    bool operator==(const AngleRadians &other) const { return v == other.v; }
    bool operator!=(const AngleRadians &other) const { return v != other.v; }
    AngleRadians &fixup() {
        if( v > glm::pi<float>())
            v -= glm::two_pi<float>();
        if( v <= -glm::pi<float>())
            v += glm::two_pi<float>();
        return *this;
    }
    bool operator<( const AngleRadians &o) const {
        return v<o.v;
    }
    bool operator>( const AngleRadians &o) const {
        return v>o.v;
    }
    AngleRadians lerp(AngleRadians towards,float factor) const {

        float diff(towards.v - v);
        if ( diff > glm::pi<float>())
            diff = diff - glm::two_pi<float>();
        if ( diff <= -glm::pi<float>())
            diff = diff + glm::two_pi<float>();
        return AngleRadians(diff * factor + v);

    }
    //    operator float()
    //    { return v;}
    float v;
    int toIntegerForm() const
    {
        return int((v + glm::pi<float>()) * 2048.0f / (glm::two_pi<float>()));
    }
    float fromIntegerForm(/*int v*/) const
    {
        return (float(v)/2048.0f)*glm::two_pi<float>() - glm::pi<float>();
    }
    explicit operator float() const {
        return v;
    }
};

// All conversion will use YPR order of rotations,
// but the used values are passed/returned in the PYR order
inline glm::vec3 toCoH_YPR(const glm::quat &q)
{
    float qxx(q.x * q.x);
    float qyy(q.y * q.y);
    float qzz(q.z * q.z);
    float qxz(q.x * q.z);
    float qxy(q.x * q.y);
    float qyz(q.y * q.z);
    float qwx(q.w * q.x);
    float qwy(q.w * q.y);
    float qwz(q.w * q.z);
    //glm::mat3 mat(mat3_cast(q));
    //float mat00 = 1 - 2 * (qyy + qzz);
    float mat01 = 2 * (qxy + qwz);
    //float mat02 = 2 * (qxz - qwy);
    float mat10 = 2 * (qxy - qwz);
    float mat11 = 1 - 2 * (qxx + qzz);
    float mat12 = 2 * (qyz + qwx);
    //float mat20 = 2 * (qxz + qwy);
    float mat21 = 2 * (qyz - qwx);
    //float mat22 = 1 - 2 * (qxx + qyy);
    float pitchVal;
    float yawVal;
    float rollVal;

    if(std::abs(1.0f - std::abs(mat21)) >= 0.00001f)
    {
        rollVal = std::atan2(-mat01, mat11);
        const float cos_roll = std::cos(rollVal);
        if(cos_roll == 0.0f)
        {
            if(rollVal <= 0.0f)
            {
                pitchVal = std::atan2(mat21, mat01);
                yawVal   = std::atan2(-mat12, mat10);
            }
            else
            {
                pitchVal = std::atan2(mat21, -mat01);
                yawVal   = std::atan2(mat12, -mat10);
            }
        }
        else
        {
            float tan_val = mat11 / cos_roll;
            pitchVal = std::atan2(mat21, tan_val);
            yawVal   = std::atan2(2 * (qxz + qwy) / tan_val, (1 - 2 * (qxx + qyy)) / tan_val);
        }
    }
    else
    {
        yawVal = std::atan2(-(2 * (qxz - qwy)), 1 - 2 * (qyy + qzz));
        if(mat21 <= 0.0f)
            pitchVal = -glm::radians(90.0f);
        else
            pitchVal = glm::radians(90.0f);
        rollVal = 0.0;
    }

    assert(pitchVal > -16.0f);
    assert(yawVal > -16.0f);
    assert(rollVal > -16.0f);
    return {pitchVal,yawVal,rollVal};
}

inline glm::vec3 CoHYprFromMat(const glm::mat3 &mat)
{
    float pitchVal;
    float yawVal;
    float rollVal;

    if(std::abs(1.0f - std::abs(mat[2][1])) >= 0.00001f)
    {
        rollVal = std::atan2(-mat[0][1], mat[1][1]);
        const float cos_roll = std::cos(rollVal);
        if(cos_roll == 0.0f)
        {
            if(rollVal <= 0.0f)
            {
                pitchVal = std::atan2(mat[2][1], mat[0][1]);
                yawVal   = std::atan2(-mat[1][2], mat[1][0]);
            }
            else
            {
                pitchVal = std::atan2(mat[2][1], -mat[0][1]);
                yawVal   = std::atan2(mat[1][2], -mat[1][0]);
            }
        }
        else
        {
            float m11 = mat[1][1] / cos_roll;
            pitchVal = std::atan2(mat[2][1], m11);
            yawVal   = std::atan2(mat[2][0] / m11, mat[2][2] / m11);
        }
    }
    else
    {
        yawVal = std::atan2(-mat[2][0],mat[0][0]);
        if(mat[2][1] <= 0.0f)
            pitchVal = -1.570796326794897f;
        else
            pitchVal = 1.570796326794897f;
        rollVal = 0.0;
    }

    assert(pitchVal > -16.0f);
    assert(yawVal > -16.0f);
    assert(rollVal > -16.0f);
    return {pitchVal,yawVal,rollVal};
}

inline glm::quat fromCoHYpr(glm::vec3 pyr)
{
    const float cp = std::cos(pyr.x);
    const float sp = std::sin(pyr.x);
    const float cy = std::cos(pyr.y);
    const float sy = std::sin(pyr.y);
    const float cr = std::cos(pyr.z);
    const float sr = std::sin(pyr.z);
    const float m00 = cr * cy + sy * sp * sr;
    const float m11 = cr * cp;
    const float m22 = cy * cp;
    const float m21 = sp;
    const float m12 = - cy * sp * cr - sr * sy;
    const float m02 = cy * sp * sr - cr * sy;
    const float m20 = sy * cp;
    const float m01 = - sr * cp;
    const float m10 = sr * cy - sy * sp * cr;
    glm::quat zc;
    zc.w = std::sqrt(std::max(0.f, 1 + m00 + m11 + m22)) / 2;
    zc.x = std::sqrt(std::max(0.f, 1 + m00 - m11 - m22)) / 2;
    zc.y = std::sqrt(std::max(0.f, 1 - m00 + m11 - m22)) / 2;
    zc.z = std::sqrt(std::max(0.f, 1 - m00 - m11 + m22)) / 2;
    zc.x = std::copysign(zc.x, m12 - m21);
    zc.y = std::copysign(zc.y, m20 - m02);
    zc.z = std::copysign(zc.z, m01 - m10);
    return zc;
}

inline void transformFromYPRandTranslation(glm::mat4 & mat, glm::vec3 pyr,glm::vec3 translation)
{
    float   cos_p     =  std::cos(pyr.x);
    float   neg_sin_p = -std::sin(pyr.x);
    float   cos_y     =  std::cos(pyr.y);
    float   neg_sin_y = -std::sin(pyr.y);
    float   cos_r     =  std::cos(pyr.z);
    float   neg_sin_r = -std::sin(pyr.z);
    float   tmp       =  - cos_y * neg_sin_p;
    glm::mat3 rotmat;
    rotmat[0][0] = cos_r * cos_y - neg_sin_y * neg_sin_p * neg_sin_r;
    rotmat[0][1] = neg_sin_r * cos_p;
    rotmat[0][2] = tmp * neg_sin_r + cos_r * neg_sin_y;
    rotmat[1][0] = -(neg_sin_r * cos_y) - neg_sin_y * neg_sin_p * cos_r;
    rotmat[1][1] = cos_r * cos_p;
    rotmat[1][2] = tmp * cos_r - neg_sin_r * neg_sin_y;
    rotmat[2][0] = -(neg_sin_y * cos_p);
    rotmat[2][1] = -neg_sin_p;
    rotmat[2][2] = cos_y * cos_p;

    mat[0]= glm::vec4(rotmat[0],0);
    mat[1]= glm::vec4(rotmat[1],0);
    mat[2]= glm::vec4(rotmat[2],0);
    mat[3]= glm::vec4(translation,1);
}

// XYZ
inline void createMat3RYP(glm::mat3 *mat, const glm::vec3 *vec)
{
    float cx = std::cos(vec->x);
    float nsx = -std::sin(vec->x);
    float cy = std::cos(vec->y);
    float nsy = -std::sin(vec->y);
    float cz = std::cos(vec->z);
    float nsz = -std::sin(vec->z);
    float tmp = -(cz * nsy);
    float tmp2 = -(nsz * nsy);
    mat[0][0].x = cz * cy;
    mat[0][1].y = nsz * cy;
    mat[0][2].z = nsy;
    mat[1][0].x = tmp * nsx - nsz * cx;
    mat[1][1].y = tmp2 * nsx + cz * cx;
    mat[1][2].z = cy * nsx;
    mat[2][0].x = nsz * nsx + tmp * cx;
    mat[2][1].y = tmp2 * cx - cz * nsx;
    mat[2][2].z = cy * cx;
}

inline void camLookAt(glm::vec3 *vec, glm::mat3 *mat)
{
    glm::vec3 result;

    result.x = std::atan2(vec->y,sqrt(vec->z * vec->z + vec->x * vec->x));
    result.y = std::atan2(vec->x,vec->z);
    result.z = 0;
    createMat3RYP(mat, &result);
}

inline float normalizeRadAngle(float ang)
{
    float res = ang;
    if( ang > glm::pi<float>() )
        res -= glm::two_pi<float>();
    if( res <= -glm::pi<float>() )
        res += glm::two_pi<float>();
    return res;
}

inline uint32_t countBits(uint32_t val)
{
    uint32_t r = 0;
    while (val >>= 1)
        r++;

    return r; // log2(v)
}

inline static float AngleDequantize(uint32_t val,int numb_bits) {
    float v = val;
    v = v/(1<<numb_bits);
    v *= glm::two_pi<float>();
    v -= glm::pi<float>();
    return v;
}

inline uint32_t AngleQuantize(float val,int numb_bits)
{
    int max_val = 1<<numb_bits;

    float v = normalizeRadAngle(val); // ensure v falls within -pi..pi
    v = (v+glm::pi<float>())/glm::two_pi<float>(); // 0..1
    v *= max_val; // 0..max_val
    return uint32_t(v);
}
