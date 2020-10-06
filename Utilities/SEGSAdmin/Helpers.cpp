#include "Helpers.h"
#include "Components/Settings.h"

#include <QSettings>
#include <QFileInfo>

Helpers::Helpers()
{

}

QString Helpers::getMapsDir()
{
    // Get maps_dir from settings.cfg
    QFileInfo config_file_info = Settings::getSettingsPath();
    QString config_file_path = config_file_info.absoluteFilePath();
    QSettings config_file(config_file_path, QSettings::IniFormat);
    config_file.beginGroup("MapServer");
    QString maps_dir = config_file.value("maps","").toString();
    config_file.endGroup();

    return maps_dir;
}
