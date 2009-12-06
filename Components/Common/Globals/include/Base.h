/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */
 
//	Base.h - This file defines all of the global includes
//	constants, typdefs, structs, etc. throughout the project
//	Base.h is to be included in every source file.

#pragma once

//	Exclude unneccessary and rarely used portions of the windows API
#define WIN32_LEAN_AND_MEAN

//	Macros
#define FN_DEFINE(a, b, c) typedef a (*fn##b##)##c##;

#ifdef WIN32
#pragma warning(disable: 4231)
#endif

//	Includes
#include <stdio.h>
#include <vector>
#include <string>
#include <map>

using namespace std;


//segs Includes
#include "types.h"
#include "Buffer.h"
//#include "auth/AuthOpcodes.h"
