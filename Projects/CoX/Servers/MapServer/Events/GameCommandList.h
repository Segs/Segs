/*
 * Super Entity Game Server Project
 * http://segs.sf.net/
 * Copyright (c) 2006 - 2017 Super Entity Game Server Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 *
 */
#pragma once

#include "MapEventTypes.h"
#include "MapLink.h"

#include <QtCore/QString>
//TODO: those must support chaining
class GameCommand
{
    const size_t    m_type;
public:
                GameCommand(size_t type) : m_type(type) {}
        size_t  type() const {return m_type;}
virtual void    serializeto(BitStream &bs) const = 0;
};

class PreUpdateCommand : public MapLinkEvent
{
    std::vector<std::unique_ptr<GameCommand> > m_contents;
public:
    PreUpdateCommand() : MapLinkEvent(MapEventTypes::evPreUpdateCommand) {}
    template <typename... Commands>
    PreUpdateCommand(Commands &&... items) : PreUpdateCommand()
    {
        std::unique_ptr<GameCommand> cmdArr[] = {std::unique_ptr<GameCommand>(std::move(items))...};
        m_contents = std::vector<std::unique_ptr<GameCommand>>{std::make_move_iterator(std::begin(cmdArr)),
                                                               std::make_move_iterator(std::end(cmdArr))};
    }

    void serializeto(BitStream &bs) const
    {
        bs.StorePackedBits(1, 13);
        for(const auto &command : m_contents)
            command->serializeto(bs);
        bs.StorePackedBits(1,0); // finalize the command list
    }
    void serializefrom(BitStream &/*src*/)
    {
        assert(false);
        // TODO: trouble, we need a second GameCommand Factory at this point !
        //uint32_t game_command = src.GetPackedBits(1);
    }
};
#include "Events/ChatMessage.h"
#include "Events/StandardDialogCmd.h"
#include "Events/InfoMessageCmd.h"
#include "Events/FloatingDamage.h"
