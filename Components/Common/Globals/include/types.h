/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

//	types.h
//
//	This header defines the data types that will be used in this code.  I(Darawk)
//	*believe* credit for these goes to CatId...but i'm not sure.
#pragma once
//////// Typedefs

#ifdef WIN32
typedef	unsigned __int8		u8;
typedef unsigned __int16	u16;
typedef unsigned __int32	u32;
typedef unsigned __int64	u64;

typedef signed __int8		s8;
typedef signed __int16		s16;
typedef signed __int32		s32;
typedef signed __int64		s64;

#else  // For UNIX/Linux (malign)
typedef unsigned    char        u8;
typedef unsigned    short       u16;
typedef unsigned    int         u32;
typedef unsigned    long long   u64;

typedef signed      char        s8;
typedef signed      short       int16_t;
typedef signed      int         int32_t;
typedef signed      long long   s64;
#endif

typedef float   f32;
typedef double  f64;
