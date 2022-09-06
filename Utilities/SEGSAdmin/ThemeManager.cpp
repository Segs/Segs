#include "ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QDebug>

QFile m_dark_theme(":/styles/Resources/styles/dark-theme.css");
QFile m_light_theme(":/styles/Resources/styles/light-theme.css");

ThemeManager::ThemeManager(QObject *parent)
    : QObject{parent}
{
    ThemeManager::setTheme(m_light_theme);
}

void ThemeManager::setTheme(QFile &theme)
{
    theme.open(QFile::ReadOnly);
    QString style_sheet = QLatin1String(theme.readAll());
    theme.close();

    qApp->setStyleSheet(style_sheet);
}

// TODO: Save theme selection using QSettings
// TODO: Move theme setting to MenuBar
void ThemeManager::setDarkTheme()
{
    this->setTheme(m_dark_theme);
}

void ThemeManager::setLightTheme()
{
    this->setTheme(m_light_theme);
}


