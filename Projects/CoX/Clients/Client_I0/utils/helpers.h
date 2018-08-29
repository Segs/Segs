#pragma once
#include <stdint.h>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

////// COH memory management wrapper macros
extern "C"
{
__declspec(dllimport) void *malloc1(int sz, int, const char *, int);
__declspec(dllimport) void FreeMem(void *, int);
__declspec(dllimport) void * dbgCalloc(int strct_size, int count, int blockType, const char *fname, int line);
}
#define COH_ARRAY_SIZE(x) ( x ? ((int *)x)[-2] : 0)
#define COH_MALLOC(x) malloc1((x),1,__FILE__,__LINE__)
#define COH_CALLOC(x, y) dbgCalloc((x), (y), 1, __FILE__, __LINE__)
#define COH_FREE(x) FreeMem(x,1);

//////// COH data loader support
#pragma pack(push, 8)
struct CharAttr_Nested1
{
    const char *name;
    int type;
    int offset;
    int param;
    CharAttr_Nested1 *ptr_sub;
};
#pragma pack(pop)
//////// COH math support
struct Vector2
{
    float x,y;
    float &operator[](int idx) { assert(idx >= 0 && idx<2); return (&x)[idx]; }
    float *data() { return &x; }
    glm::vec2 toGLM() const { return { x,y }; }
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
    Vector3 operator-() const
    {
        return { -x,-y,-z };
    }
    Vector3 operator+(Vector3 other) const
    {
        return { x + other.x,y + other.y,z + other.z };
    }
    Vector3 &operator+=(Vector3 other)
    {

        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    Vector3 &operator-=(Vector3 other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
    constexpr Vector3 operator*(float v) const
    {
        return { x*v, y*v, z*v };
    }
    constexpr Vector3 &operator*=(float v)
    {
        x*=v;
        y*=v;
        z*=v;
        return *this;
    }
    float distance_non_sqrt(const Vector3 &b) const
    {
        return (*this - b).lengthNonSqrt();
    }
    bool close_enough_to(const Vector3 &b) const
    {
        return distance_non_sqrt(b)<0.001f;
    }
    constexpr Vector3 operator*(Vector3 v) const
    {
        return { x*v.x, y*v.y, z*v.z };
    }
    constexpr Vector3 operator/(float v) const
    {
        return { x/v, y/v, z/v };
    }

    Vector3 min_val(const Vector3 &v) const {
        return { std::min(x, v.x), std::min(y, v.y), std::min(z, v.z) };
    }
    Vector3 max_val(const Vector3 &v) const {
        return { std::max(x, v.x), std::max(y, v.y), std::max(z, v.z) };
    }
    glm::vec3 toGLM() const { return glm::vec3{ x,y,z }; }
    float *data() { return &x; }
    float lengthNonSqrt() const { return x * x + y * y + z * z; }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
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
    bool operator!=(Vector3 oth) const
    {
        return x!=oth.x || y!=oth.y || z!=oth.z;
    }
    bool operator==(Vector3 oth) const
    {
        return x==oth.x && y==oth.y && z==oth.z;
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
    float x=0,y=0,z=0,w=0;
    Vector4() = default;
    constexpr Vector4(float x_,float y_,float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    Vector4(Vector3 v3,float w_) : x(v3.x),y(v3.y),z(v3.z),w(w_) {}
    Vector4 &operator=(const Vector4 &v) = default;
    Vector4 &operator=(const Vector3 &v) {
        x=v.x;
        y=v.y;
        z=v.z;
        w=0;
        return *this;
    }
    void fromGLM(glm::vec4 v) {
        x = v.x;
        y = v.y;
        z = v.z;
        w = v.w;
    }
    Vector3 &ref3() { return *(Vector3 *)&x;}
    float &operator[](int idx) { assert(idx>=0 && idx<4); return (&x)[idx];}
    float *data() { return &x; }
    bool operator!=(Vector4 oth) const
    {
        return x!=oth.x || y!=oth.y || z!=oth.z || w!=oth.w;
    }
    bool operator==(Vector4 oth) const
    {
        return x==oth.x && y==oth.y && z==oth.z && w==oth.w;
    }
};
struct Matrix3x3 {
    Vector3 r1 {1, 0, 0};
    Vector3 r2 {0, 1, 0};
    Vector3 r3 {0, 0, 1};
    Vector3 operator *(const Vector3 &val) const
    {
        return {
            val.x * r1.x + val.y * r2.x + val.z * r3.x,
            val.x * r1.y + val.y * r2.y + val.z * r3.y,
            val.x * r1.z + val.y * r2.z + val.z * r3.z
        };
    }
    Matrix3x3  operator *(const Matrix3x3 &rhs) const
    {
        return { {
            r1.x * rhs.r1.x + r1.y * rhs.r2.x + r1.z * rhs.r3.x,
            r1.x * rhs.r1.y + r1.y * rhs.r2.y + r1.z * rhs.r3.y,
            r1.x * rhs.r1.z + r1.y * rhs.r2.z + r1.z * rhs.r3.z
        },{
            r2.x * rhs.r1.x + r2.y * rhs.r2.x + r2.z * rhs.r3.x,
            r2.x * rhs.r1.y + r2.y * rhs.r2.y + r2.z * rhs.r3.y,
            r2.x * rhs.r1.z + r2.y * rhs.r2.z + r2.z * rhs.r3.z
        },{
            r3.x * rhs.r1.x + r3.y * rhs.r2.x + r3.z * rhs.r3.x,
            r3.x * rhs.r1.y + r3.y * rhs.r2.y + r3.z * rhs.r3.y,
            r3.x * rhs.r1.z + r3.y * rhs.r2.z + r3.z * rhs.r3.z
        }};
    }
    bool operator==(const Matrix3x3 &oth) const
    {
        return r1==oth.r1 && r2==oth.r2 && r3==oth.r3;
    }
    Matrix3x3 transpose() const
    {
        return { {r1.x,r2.x,r3.x},
        {r1.y,r2.y,r3.y},
        {r1.z,r2.z,r3.z }
        };
    }
    void scaleBy(float f)
    {
        r1.x *= f;
        r1.y *= f;
        r1.z *= f;
        r2.x *= f;
        r2.y *= f;
        r2.z *= f;
        r3.x *= f;
        r3.y *= f;
        r3.z *= f;
    }
};
struct Matrix4x3
{
    Vector3          r1{1, 0, 0};
    Vector3          r2{0, 1, 0};
    Vector3          r3{0, 0, 1};
    Vector3          TranslationPart {0,0,0};
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
    bool operator==(const Matrix4x3 &oth) const
    {
        return r1==oth.r1 && r2==oth.r2 && r3==oth.r3 && TranslationPart==oth.TranslationPart;
    }
    Vector3 &operator[](int r) { return (&r1)[r];}
    glm::mat4 toGLM() const
    {
        return glm::mat4(r1.x, r1.y, r1.z, 0,
            r2.x, r2.y, r2.z, 0,
            r3.x, r3.y, r3.z, 0,
            TranslationPart.x, TranslationPart.y, TranslationPart.z, 1.0f);
    }
};
extern const Matrix4x3 Unity_Matrix;

struct Matrix4x4
{
    Vector4 r1{1, 0, 0, 0};
    Vector4 r2{0, 1, 0, 0};
    Vector4 r3{0, 0, 1, 0};
    Vector4 TranslationPart{0, 0, 0, 1};
    Matrix4x4 &operator=(const Matrix4x3 &v) {
        r1 = v.r1;
        r2 = v.r2;
        r3 = v.r3;
        TranslationPart = v.TranslationPart;
        TranslationPart.w = 1.0;
        return *this;
    }
    explicit Matrix4x4() {}
    Matrix4x4(const Matrix4x3 &v)
    {
        *this = v;
    }
    float *data() { return (float *)this; }
    glm::mat4 toGLM() const
    {
        return glm::mat4(r1.x, r1.y, r1.z, r1.w,
            r2.x, r2.y, r2.z, r2.w,
            r3.x, r3.y, r3.z, r3.w,
            TranslationPart.x, TranslationPart.y, TranslationPart.z, TranslationPart.w);
    }
    Matrix4x4 & operator=(glm::mat4 from)
    {
        r1.fromGLM(from[0]);
        r2.fromGLM(from[1]);
        r3.fromGLM(from[2]);
        TranslationPart.fromGLM(from[3]);
        return *this;
    }
};
struct Array
{
    unsigned int size;
    int capacity;
    void **contents;
};

template<class T>
T lerp(T a,T b, float factor)
{
    return a*(1.0f - factor) + b*factor;
}
template<class T>
T clamp(T val,T min_,T max_)
{
    return std::max(min_,std::min(max_,val));
}
bool stringContains(const char *haystack,const char *needle);
inline void transposeMat4Copy(Matrix4x3 *inp, Matrix4x3 *res)
{
    res->r1.x = inp->r1.x;
    res->r1.y = inp->r2.x;
    res->r1.z = inp->r3.x;
    res->r2.x = inp->r1.y;
    res->r2.y = inp->r2.y;
    res->r2.z = inp->r3.y;
    res->r3.x = inp->r1.z;
    res->r3.y = inp->r2.z;
    res->r3.z = inp->r3.z;
    Vector3 neg_translate = - inp->TranslationPart;
    res->TranslationPart = res->ref3() * neg_translate;
}
void mulVecMat4Transpose(Vector3 *src, Matrix4x3 *mat, Vector3 *dst);
bool isVectorDistanceLessThenEps(Vector3 *a, Vector3 *b, float eps);
void multVertexByMatrix(Vector3 *src, Matrix3x3 *mat, Vector3 *dst);
void printfDebug(const char *fmt, ...);
void segs_fxCleanFileName(char *dst, const char *src);

void patch_helperutils();
