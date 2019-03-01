/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once

#include "EventProcessor.h"
#include "Common/Servers/HandlerLocator.h"
#include "Common/Servers/ClientManager.h"
#include "Servers/GameDatabase/GameDBSyncHandler.h"
#include "Servers/MapServer/MapClientSession.h"
#include <QVector>

namespace SEGSEvents
{
class SelectKeybindProfile;
class ResetKeybinds;
class SetKeybind;
class RemoveKeybind;
} // end of namespace SEGSEvents

class SettingsService
{
private:
    using SessionStore = ClientSessionStore<MapClientSession>;
    SessionStore& m_session_store;

public:
    SettingsService(SessionStore& session_store) : m_session_store(session_store) {}
    void on_select_keybind_profile(SEGSEvents::SelectKeybindProfile *ev);
    void on_reset_keybinds(SEGSEvents::ResetKeybinds *ev);
    void on_set_keybind(SEGSEvents::SetKeybind *ev);
    void on_remove_keybind(SEGSEvents::RemoveKeybind *ev);
protected:
};
