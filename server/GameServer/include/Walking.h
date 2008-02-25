/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 * $Id: Walking.h 253 2006-08-31 22:00:14Z malign $
 */

// Inclusion guards
#pragma once
#ifndef WALKING_H
#define WALKING_H

class Walking
{
 public:
  // Constructor/Destructor
  Walking();
  ~Walking();

  GetPosition(char *client);
  UpdatePosition(int x, int y, int z);
};

#endif // WALKING_H
