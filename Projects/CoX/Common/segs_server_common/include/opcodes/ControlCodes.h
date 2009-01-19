/*
 * Super Entity Game Server
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: ControlCodes.h 253 2006-08-31 22:00:14Z malign $
 */

/************************************************************************
File: ControlCodes.h
Description: Defines an enumerated list of control codes, which are a
			 sub-set opcode list used only for controlling and managing
			 the RUDP network connection.  These opcodes are not relevant
			 to the game-play itself.
************************************************************************/

// Inclusion guards
#pragma once
#ifndef CONTROLCODES_H
#define CONTROLCODES_H

typedef enum 
{
	CTRL_IDLE = 0x00,
	CTRL_CONNECT,
	CTRL_KEY_REQUEST, //COMMCONTROL_CONNECT_SERVER_ACK
	CTRL_KEY_REPLY, // COMMCONTROL_CONNECT_CLIENT_ACK
	CTRL_CONNECTED,	//COMMCONTROL_CONNECT_SERVER_ACK_ACK
	CTRL_UNK5, //COMMCONTROL_DISCONNECT
	CTRL_DISCONNECT, //COMMCONTROL_DISCONNECT_ACK
	CTRL_RESIZE,		//	?
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

#endif // CONTROLCODES_H
