#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QFile>
#include <QObject>

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    explicit ThemeManager(QObject *parent = nullptr);

public slots:
    void setLightTheme();
    void setDarkTheme();

private:
    void setTheme(QFile &theme);

signals:

};

#endif // THEMEMANAGER_H
