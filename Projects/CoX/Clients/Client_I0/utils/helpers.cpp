#include "helpers.h"

#include <cstring>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <windows.h>

const char *strcasestr(const char *haystack, const char *needle)
{
    do
    {
        const char *h = haystack;
        const char *n = needle;
        while (tolower((unsigned char)*h) == tolower((unsigned char)*n) && *n)
        {
            h++;
            n++;
        }
        if (*n == 0)
        {
            return haystack;
        }
    } while (*haystack++);
    return nullptr;
}

bool stringContains(const char * haystack, const char * needle)
{
    return strcasestr(haystack,needle)!=nullptr;
}

void mulVecMat4Transpose(Vector3 *src, Matrix4x3 *mat, Vector3 *dst)
{
    float dx;
    float dy;
    float dz;

    dx = src->x - mat->TranslationPart.x;
    dy = src->y - mat->TranslationPart.y;
    dz = src->z - mat->TranslationPart.z;
    dst->x = dz * mat->r1.z + dy * mat->r1.y + dx * mat->r1.x;
    dst->y = dz * mat->r2.z + dy * mat->r2.y + dx * mat->r2.x;
    dst->z = dz * mat->r3.z + dy * mat->r3.y + dx * mat->r3.x;
}

bool isVectorDistanceLessThenEps(Vector3* a, Vector3* b, float eps) 
{
    return (*a - *b).lengthNonSqrt() < eps;
}

void multVertexByMatrix(Vector3 *src, Matrix3x3 *mat, Vector3 *dst) {
    dst->x = src->x * mat->r1.x + src->y * mat->r2.x + src->z * mat->r3.x;
    dst->y = src->x * mat->r1.y + src->y * mat->r2.y + src->z * mat->r3.y;
    dst->z = src->x * mat->r1.z + src->y * mat->r2.z + src->z * mat->r3.z;
}

void printfDebug(const char * fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    TCHAR szBuffer[1024]; // get rid of this hard-coded buffer
    vsnprintf(szBuffer, 1024-1, fmt, args);
    OutputDebugString(szBuffer);
    va_end(args);
}
