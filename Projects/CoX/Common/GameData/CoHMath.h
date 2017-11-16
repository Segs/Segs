#pragma once
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>


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
    glm::mat3 mat(mat3_cast(q));
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

    if (std::abs(1.0f - std::abs(mat21)) >= 0.00001f)
    {
        rollVal = std::atan2(-mat01, mat11);
        const float cos_roll = std::cos(rollVal);
        if (cos_roll == 0.0f)
        {
            if (rollVal <= 0.0f)
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
            float v11 = mat11 / cos_roll;
            pitchVal = std::atan2(mat21, v11);
            yawVal   = std::atan2(2 * (qxz + qwy) / v11, (1 - 2 * (qxx + qyy)) / v11);
        }
    }
    else
    {
        yawVal = std::atan2(-(2 * (qxz - qwy)), 1 - 2 * (qyy + qzz));
        if (mat21 <= 0.0f)
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
inline glm::vec3 CoHYprFromMat(const glm::mat3 &mat)
{
    float pitchVal;
    float yawVal;
    float rollVal;

    if (std::abs(1.0f - std::abs(mat[2][1])) >= 0.00001f)
    {
        rollVal = std::atan2(-mat[0][1], mat[1][1]);
        const float cos_roll = std::cos(rollVal);
        if (cos_roll == 0.0f)
        {
            if (rollVal <= 0.0f)
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
            float v11 = mat[1][1] / cos_roll;
            pitchVal = std::atan2(mat[2][1], v11);
            yawVal   = std::atan2(mat[2][0] / v11, mat[2][2] / v11);
        }
    }
    else
    {
        yawVal = std::atan2(-mat[2][0],mat[0][0]);
        if (mat[2][1] <= 0.0f)
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
inline glm::mat3 CoHYprToMat3(glm::vec3 pyr)
{
    const float cp = std::cos(pyr.x);
    const float sp = std::sin(pyr.x);
    const float cy = std::cos(pyr.y);
    const float sy = std::sin(pyr.y);
    const float cr = std::cos(pyr.z);
    const float sr = std::sin(pyr.z);
    glm::mat3 mat;
    mat[0][0] = cr * cy + sy * sp * sr;
    mat[1][0] = sr * cy - sy * sp * cr;
    mat[2][0] = sy * cp;
    mat[0][1] = - sr * cp;
    mat[1][1] = cr * cp;
    mat[2][1] = sp;
    mat[0][2] = cy * sp * sr - cr * sy;
    mat[1][2] = - cy * sp * cr - sr * sy;
    mat[2][2] = cy * cp;

    {
        float sp = -std::sin(pyr.x);
        float sy = -std::sin(pyr.y);
        float sr = -std::sin(pyr.z);
        mat[0][0] = cr * cy - sy * sp * sr;
        mat[1][0] = -(sr * cy) - sy * sp * cr;
        mat[2][0] = -(sy * cp);
        mat[0][1] = sr * cp;
        mat[1][1] = cr * cp;
        mat[2][1] = -sp;
        mat[0][2] = cy * sp * sr + cr * sy;
        mat[1][2] = cy * sp * cr - sr * sy;
        mat[2][2] = cy * cp;
    }
    return mat;
}
