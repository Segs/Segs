/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006-2010 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
#pragma once
#if !defined WIN32 || defined __MINGW32__
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#define hash_map std::tr1::unordered_map
#define hash_set std::tr1::unordered_set
#else
#include <hash_map>
#include <hash_set>
using namespace stdext;
#endif // WIN32
