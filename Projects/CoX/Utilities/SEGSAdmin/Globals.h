#ifndef GLOBALS_H
#define GLOBALS_H
#include <QStringList>
#include <QVector>

extern QStringList g_map_names; // List of all known maps
extern QStringList g_db_drivers; // List of supported DB drivers
struct SegsReleaseInfo
{
    QString tag_name;
    QString release_name;
    QString id;
    QString release_notes;
    QString github_url;
};
extern QVector<SegsReleaseInfo> g_segs_release_info;

#endif // GLOBALS_H
