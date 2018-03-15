#pragma once
#include <stdint.h>
#include <cassert>
#include <cmath>

struct Vector2
{
    float x,y;
    float &operator[](int idx) { assert(idx >= 0 && idx<2); return (&x)[idx]; }
};
struct Vector3
{
    float x,y,z;
    static Vector3 lerp(const Vector3 &a,const Vector3 &b,float alpha)
    {
        return {
            a.x*alpha+(1-alpha)*b.x,
            a.y*alpha+(1-alpha)*b.y,
            a.z*alpha+(1-alpha)*b.z
        };
    }
    float &operator[](int idx) { assert(idx>=0 && idx<3); return (&x)[idx];}
    Vector3 operator-(Vector3 other) const
    {
        return { x - other.x,y - other.y,z - other.z };
    }
    Vector3 operator+(Vector3 other) const
    {
        return { x + other.x,y + other.y,z + other.z };
    }
    constexpr Vector3 operator*(float v) const
    {
        return { x*v, y*v, z*v };
    }
    constexpr Vector3 operator/(float v) const
    {
        return { x/v, y/v, z/v };
    }

    float lengthNonSqrt() const { return x * x + y * y + z * z; }
    float normalize()
    {
        float factor = std::sqrt(lengthNonSqrt());
        x /= factor;
        y /= factor;
        z /= factor;
        return factor;
    }
    float dot(Vector3 with) const
    {
        return x * with.x + y * with.y + z * with.z;
    }
    Vector3 cross(const Vector3 &with) const
    {
        return {
            y * with.z - z * with.y,
            z * with.x - x * with.z,
            x * with.y - y * with.x };
    }

};
struct Vector2si
{
    short i0,i1;
};
struct Vector2su
{
    uint16_t i0,i1;
};
struct Vector3i
{
    int i0,i1,i2;
    int &operator[](uint32_t idx) { assert(idx < 3); return ((int *)this)[idx]; }
};
struct Vector4
{
    float x,y,z,w;
    Vector4 &operator=(const Vector4 &v) = default;
    Vector4 &operator=(const Vector3 &v) {
        x=v.x;
        y=v.y;
        z=v.z;
        w=0;
        return *this;
    }
    Vector3 &ref3() { return *(Vector3 *)&x;}
    float &operator[](int idx) { assert(idx>=0 && idx<4); return (&x)[idx];}
    float *data() { return &x; }
};
struct Matrix3x3 {
    Vector3 r1;
    Vector3 r2;
    Vector3 r3;
};
struct Matrix4x3
{
    Vector3 r1;
    Vector3 r2;
    Vector3 r3;
    Vector3 TranslationPart;
    Matrix3x3 &ref3() { return (Matrix3x3&)*this; }
    const Matrix3x3 &ref3() const { return (const Matrix3x3&)*this; }
    Vector3 operator *(const Vector3 &val) const
    {
        return {
            val.x * r1.x + val.y * r2.x + val.z * r3.x + TranslationPart.x,
            val.x * r1.y + val.y * r2.y + val.z * r3.y + TranslationPart.y,
            val.x * r1.z + val.y * r2.z + val.z * r3.z + TranslationPart.z
        };
    }
    Matrix4x3 operator*(const Matrix4x3 &oth) const
    {
        Matrix4x3 res;
        res.r1.x = oth.r1.x * r1.x + oth.r1.y * r2.x + oth.r1.z * r3.x;
        res.r1.y = oth.r1.x * r1.y + oth.r1.y * r2.y + oth.r1.z * r3.y;
        res.r1.z = oth.r1.x * r1.z + oth.r1.y * r2.z + oth.r1.z * r3.z;
        res.r2.x = oth.r2.x * r1.x + oth.r2.y * r2.x + oth.r2.z * r3.x;
        res.r2.y = oth.r2.x * r1.y + oth.r2.y * r2.y + oth.r2.z * r3.y;
        res.r2.z = oth.r2.x * r1.z + oth.r2.y * r2.z + oth.r2.z * r3.z;
        res.r3.x = oth.r3.x * r1.x + oth.r3.y * r2.x + oth.r3.z * r3.x;
        res.r3.y = oth.r3.x * r1.y + oth.r3.y * r2.y + oth.r3.z * r3.y;
        res.r3.z = oth.r3.x * r1.z + oth.r3.y * r2.z + oth.r3.z * r3.z;
        res.TranslationPart.x = oth.TranslationPart.x * r1.x + oth.TranslationPart.y * r2.x + oth.TranslationPart.z * r3.x + TranslationPart.x;
        res.TranslationPart.y = oth.TranslationPart.x * r1.y + oth.TranslationPart.y * r2.y + oth.TranslationPart.z * r3.y + TranslationPart.y;
        res.TranslationPart.z = oth.TranslationPart.x * r1.z + oth.TranslationPart.y * r2.z + oth.TranslationPart.z * r3.z + TranslationPart.z;
        return res;
    }

};
struct Matrix4x4
{
    Vector4 r1;
    Vector4 r2;
    Vector4 r3;
    Vector4 TranslationPart;
    Matrix4x4 &operator=(const Matrix4x3 &v) {
        r1 = v.r1;
        r2 = v.r2;
        r3 = v.r3;
        TranslationPart = v.TranslationPart;
        return *this;
    }
    Matrix4x4(const Matrix4x3 &v)
    {
        *this = v;
    }
    float *data() { return (float *)this; }
};
struct Array
{
    unsigned int size;
    int capacity;
    void **contents;
};

bool stringContains(const char *haystack,const char *needle);
void mulVecMat4Transpose(Vector3 *src, Matrix4x3 *mat, Vector3 *dst);
bool isVectorDistanceLessThenEps(Vector3 *a, Vector3 *b, float eps);
void multVertexByMatrix(Vector3 *src, Matrix3x3 *mat, Vector3 *dst);
void printfDebug(const char *fmt, ...);
