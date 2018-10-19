/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "Character.h"
#include "Logging.h"


void addContact(CharacterData &cd, Contact &contact)
{
    qCDebug(logMapEvents) << "Character addContact";
    cd.m_contacts.push_back(contact);
}
