#include "SlashCommand.h"
#include "DataHelpers.h"

#include <QtCore/QString>
#include <QtCore/QDebug>

bool canAccessCommand(const Entity &e, SlashCommand &cmd)
{
    int alvl = getAccessLevel(e);

    if(alvl >= cmd.m_required_access_level)
        return true;

    return false;
}

SlashCommand getSlashCommand(const Entity &src, QString &str)
{
    SlashCommand cmd;

    return cmd;
}
