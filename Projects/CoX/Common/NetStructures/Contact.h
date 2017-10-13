#pragma once

#include <QString>

struct Contact
{
    QString m_name;
    QString m_display_name;

    // for scripting language access.
    std::string getName() const { return m_name.toStdString();}
    void setName(const char *n) { m_name = n; }
};
