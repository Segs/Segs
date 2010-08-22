#pragma once
#ifndef WIN32
#include <tr1/unordered_map>
#include <tr1/unordered_set>
using namespace std::tr1;
#define hash_map unordered_map
#define hash_set unordered_set
#else
#include <hash_map>
#include <hash_set>
using namespace stdext;
#endif // WIN32
