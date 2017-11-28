#pragma once
#include <QtCore/QString>

struct hp_base_values {
    hp_base_values() : mod(), base() {}
    hp_base_values(float m, float b)
            : mod(m), base(b) {}
    
    float mod;
    float base;
};

struct hp_archetype_values {
    hp_archetype_values() : mod(), cap() {}
    hp_archetype_values(float m, float c)
            : mod(m), cap(c) {}
    
    float mod;
    float cap;
};

QMap<int level, hp_base_values> hp_base_table = {
    {1, {0.2, 100.0000}},
    {2, {0.21, 110.5000}},
    {3, {0.23, 121.7997}},
    {4, {0.25, 133.9275}},
    {5, {0.27, 146.9091}},
    {6, {0.3, 160.7676}},
    {7, {0.35, 175.5225}},
    {8, {0.4, 191.1898}},
    {9, {0.45, 207.7812}},
    {10, {0.5, 225.3041}},
    {11, {0.55, 243.7607}},
    {12, {0.6, 263.1478}},
    {13, {0.65, 283.4566}},
    {14, {0.7, 304.6722}},
    {15, {0.75, 326.7734}},
    {16, {0.8, 349.7323}},
    {17, {0.85, 373.5140}},
    {18, {0.9, 398.0770}},
    {19, {0.95, 423.3724}},
    {20, {1, 449.3441}},
    {21, {1, 475.9290}},
    {22, {1, 503.0570}},
    {23, {1, 530.6509}},
    {24, {1, 558.6270}},
    {25, {1, 586.8953}},
    {26, {1, 615.3598}},
    {27, {1, 643.9188}},
    {28, {1, 672.4658}},
    {29, {1, 700.8901}},
    {30, {1, 729.0768}},
    {31, {1, 756.9086}},
    {32, {1, 784.2654}},
    {33, {1, 811.0261}},
    {34, {1, 837.0691}},
    {35, {1, 862.2729}},
    {36, {1, 886.5175}},
    {37, {1, 909.6852}},
    {38, {1, 931.6613}},
    {39, {1, 952.3353}},
    {40, {1, 971.6017}},
    {41, {1, 989.3612}},
    {42, {1, 1005.5210}},
    {43, {1, 1019.9950}},
    {44, {1, 1032.7080}},
    {45, {1, 1043.5910}},
    {46, {1, 1052.5850}},
    {47, {1, 1059.6440}},
    {48, {1, 1064.7290}},
    {49, {1, 1067.8130}},
    {50, {1, 1070.8970}},
};

QMap<QString m_archetype, hp_archetype_values> hp_archetype_table = {
    {"Blaster", {1.125, 1}},
    {"Controller", {0.95, 1}},
    {"Defender", {0.95, 1}},
    {"Scrapper", {1.25, 1.5}},
    {"Tanker", {1.75, 2.2}},
    {"Peacebringer", {1, 1.5}},
    {"Warshade", {1, 1.5}},
    {"Corruptor", {1, 1}},
    {"Dominator", {0.95, 1}},
    {"Mastermind", {0.75, 1}},
    {"Brute", {1.4, 2}},
    {"Stalker", {1.125, 1.3}},
    {"Arachnos Soldier", {1, 1.5}},
    {"Arachnos Widow", {1, 1.5}},
};

inline float get_HP_base(const Entity &src)
{
    // ( ( (Base HP Archetype Modifier - 1) * Level Modifier) + 1) * Level HP
    float hp;
    hp_archetype_values a_val = hp_archetype_table[QString(src.m_class_name)].value;
    hp_base_values b_val = hp_base_table[src.m_level].value;
    hp = (((a_val.mod - 1) * b_val.mod) + 1) * b_val.base;
    
    return hp;
}

inline float get_HP_cap(const Entity &src)
{
    // ( ( (Max HP Cap Archetype Modifier - 1) * Level Modifier) + 1) * 1.5 * Level HP
    float hp;
    hp_archetype_values a_val = hp_archetype_table[QString(src.m_class_name)].value;
    hp_base_values b_val = hp_base_table[src.m_level].value;
    hp = (((a_val.cap - 1) * b_val.mod) + 1) * 1.5 * b_val.base;
    
    return hp;
}