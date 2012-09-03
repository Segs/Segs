/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

/************************************************************************
File: ControlCodes.h
Description: Defines an enumerated list of control codes, which are a
			 sub-set opcode list used only for controlling and managing
			 the RUDP network connection.  These opcodes are not relevant
			 to the game-play itself.
************************************************************************/

#pragma once

typedef enum
{
	CTRL_IDLE = 0x00,
	CTRL_CONNECT,
	CTRL_KEY_REQUEST,
	CTRL_KEY_REPLY,
	CTRL_CONNECTED,	//	?
	CTRL_DISCONNECT_REQ,
	CTRL_DISCONNECT_ACK,
	CTRL_RESIZE,		//	?
	CTRL_UNK8,
	CTRL_UNK9,
	CTRL_NET_CMD,			//	Contains a textual command string
	CTRL_UNK11,
	CTRL_UNK12,
	CTRL_UNK13
}eControlCodes;

/*
//	For use with CTRL_NET_CMD
static const char *netCommands[] =
{
	"autosave",
	"group",
	"ungroup",
	"attach",
	"detach",
	"delete",
	"paste",
	"new",
	"save",
	"savesel",
	"savelibs",
	"load",
	"updatetrackers",
	"updatetracker",
	"updatedef",
	"undo",
	"scenefile",
	"defload",
	"trayswap",
	"groupall",
	"ungroupall"
};
*/
