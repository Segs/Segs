/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

#pragma once
#include "GameCommand.h"
#include "MapEventTypes.h"
#include "GameData/Contact.h"
#include "Components/BitStream.h"

#include <QtCore/QString>

namespace SEGSEvents
{

// [[ev_def:type]]
class ContactDialog final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    QString m_msgbody;
    // [[ev_def:field]]
    std::vector<ContactEntry> m_active_contacts;

    explicit    ContactDialog() : GameCommandEvent(evContactDialog) {}
                ContactDialog(QString msgbody, std::vector<ContactEntry> active_contacts) : GameCommandEvent(evContactDialog),
                    m_msgbody(msgbody),
                    m_active_contacts(active_contacts)
                {
                }

                void serializeto(BitStream &bs) const override
                {
                    bs.StorePackedBits(1, type()-evFirstServerToClient); // 39

                    bs.StoreString(m_msgbody);
                    bs.StorePackedBits(1, m_active_contacts.size());

                    for(const ContactEntry &contact : m_active_contacts)
                    {
                        bs.StorePackedBits(1, contact.m_link);
                        bs.StoreString(contact.m_response_text);
                    }
                }

    EVENT_IMPL(ContactDialog)
};

// [[ev_def:type]]
class ContactDialogClose final : public GameCommandEvent
{
public:
    ContactDialogClose() : GameCommandEvent(evContactDialogClose)
    {}

    void serializeto(BitStream &bs) const override
    {
        bs.StorePackedBits(1, type()-evFirstServerToClient); // 40
        // send nothing
    }

    EVENT_IMPL(ContactDialogClose)
};

// [[ev_def:type]]
class ContactDialogOk final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    QString     m_msgbody;

    explicit    ContactDialogOk() : GameCommandEvent(evContactDialogOk) {}
                ContactDialogOk(QString msgbody) : GameCommandEvent(evContactDialogOk),
                    m_msgbody(msgbody)
                {
                }

                void serializeto(BitStream &bs) const override
                {
                    bs.StorePackedBits(1, type()-evFirstServerToClient); // 41

                    bs.StoreString(m_msgbody);
                    bs.StorePackedBits(1, 0); // TODO: doesn't appear to do anything?
                }

    EVENT_IMPL(ContactDialogOk)
};

// [[ev_def:type]]
class ContactDialogYesNo final : public GameCommandEvent
{
public:
    // [[ev_def:field]]
    QString     m_msgbody;

    explicit    ContactDialogYesNo() : GameCommandEvent(evContactDialogYesNo) {}
                ContactDialogYesNo(QString msgbody) : GameCommandEvent(evContactDialogYesNo),
                    m_msgbody(msgbody)
                {
                }

                void serializeto(BitStream &bs) const override
                {
                    bs.StorePackedBits(1, type()-evFirstServerToClient); // 42

                    bs.StoreString(m_msgbody);
                    bs.StorePackedBits(1, 0); // TODO: doesn't appear to do anything?
                }

    EVENT_IMPL(ContactDialogYesNo)
};

} // end of SEGSEvents namespace
