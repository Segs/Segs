#include "Globals.h"
#include <QStringList>


// List of all known maps
QStringList g_map_names = {
    "City_00_01",
    "City_01_01",
    "City_01_02",
    "City_01_03",
    "City_02_01",
    "City_02_02",
    "City_03_01",
    "City_03_02",
    "City_04_01",
    "City_04_02",
    "City_05_01",
    "Hazard_01_01",
    "Hazard_02_01",
    "Hazard_03_01",
    "Hazard_04_01",
    "Hazard_04_02",
    "Hazard_05_01",
    "Trial_01_01",
    "Trial_01_02",
    "Trial_02_01",
    "Trial_03_01",
    "Trial_04_01",
    "Trial_04_02",
    "Trial_05_01"
};

// List of supported DB drivers
QStringList g_db_drivers = {
    "QSQLITE",
    "QMYSQL",
    "QPSQL"
};

QVector<SegsReleaseInfo> g_segs_release_info;
