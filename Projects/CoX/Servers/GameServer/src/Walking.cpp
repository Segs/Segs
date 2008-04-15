/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Walking.cpp 253 2006-08-31 22:00:14Z malign $
 */

// Contains functions for entity movement

#include "Walking.h"

// Could do something like client->UpdatePosition(x, y, z); ? (sample args)
// Using nemerle's findings located within Trac for entity positioning

// Get the current position of a client
Walking::GetPosition(char *client)
{
  int x,y,z;
  // These members perhaps we can add to the client structure?
/*  x = client.xPosition;
  y = client.yPosition;
  z = client.zPosition; */

  return 0;
}

Walking::UpdatePosition(int x, int y, int z) // Assuming Cartesian mapping
{
  return 0;
}
