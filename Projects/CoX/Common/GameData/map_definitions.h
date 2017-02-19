#pragma once

#include <glm/vec2.hpp>
#include <QtCore/QString>
#include <vector>

struct Map_Data
{
    glm::vec2 Location;
    glm::vec2 TextLocation;
    QString Name;
    QString Icon;
};
typedef std::vector<Map_Data> AllMaps_Data;
