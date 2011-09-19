/*
 * Super Entity Game Server Project 
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id$
 */

// Inclusion guards
#pragma once
#ifndef CHAT_H
#define CHAT_H

class Chat
{
  public:

   // (Con/De)structor
   Database(void);
   ~Database(void);

   // Class Methods
   int LocalMsg(char *player1, char *message);
   int PlayerTell(char *charname1, char *charname2, char *message);
   int TeamSay(char *charname, char *team[7], char *message);
   int GenericSpeech(char *charname, int channel, char *message);
};

#endif // CHAT_H
