/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

// This module contains player chat related functions

#include "Chat.h"

// Player (charname) says something (message) in Local channel
Chat::LocalMsg(char *player1, char *message)
{
  // use sprintf() ?
  return 0;
}

// Character (charname1) sends another player (charname2) a private message
Chat::PlayerTell(char *charname1, char *charname2, char *message)
{
  // sprintf() ?
  return 0;
}

// Character (charname) says something to their teammates
Chat::TeamSay(char *charname, char *team[7], char *message)
{
  // sprintf() ?
  return 0;
}

// Character (charname) says something in a channel (broadcast, super group, request, etc.)
Chat::GenericSpeech(char *charname, int channel, char *message)
{
  // sprintf() ?
  return 0;
}
