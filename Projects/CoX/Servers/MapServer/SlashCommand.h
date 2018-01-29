#include "Entity.h"
#include "Character.h"

class QString;

struct SlashCommand
{
    QStringList m_valid_prefixes;
    std::function<void(const QString &,Entity *)> m_handler;
    int m_required_access_level;
};

bool canAccessCommand(const Entity &e, SlashCommand &cmd);
