#include "Globals.h"
#include <QStringList>


// List of all known maps
QStringList g_map_names = {
    "City_00_01",
    "City_01_01",
    "City_23_01"
};

// List of supported DB drivers
QStringList g_db_drivers = {
    "QSQLITE",
    "QMYSQL",
    "QPSQL"
};

QVector<SegsReleaseInfo> g_segs_release_info;
