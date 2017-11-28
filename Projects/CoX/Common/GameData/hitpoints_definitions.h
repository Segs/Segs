#pragma once
#include "Character.h"
#include <QtCore/QString>
#include <QtCore/QMap>

struct HPBaseValues {
    HPBaseValues() : mod(), base() {}
    HPBaseValues(float m, float b)
            : mod(m), base(b) {}
    
    float mod;
    float base;
} hp_base_table[50];

struct HPArchetypeValues {
    HPArchetypeValues() : mod(), cap() {}
    HPArchetypeValues(float m, float c)
            : mod(m), cap(c) {}
    
    float mod;
    float cap;
};

inline void getBaseHP()
{
    hp_base_table[1] = HPBaseValues(0.2,100.0000);
    hp_base_table[2] = HPBaseValues(0.21, 110.5000);
    hp_base_table[3] = HPBaseValues(0.23, 121.7997);
    hp_base_table[4] = HPBaseValues(0.25, 133.9275);
    hp_base_table[5] = HPBaseValues(0.27, 146.9091);
    hp_base_table[6] = HPBaseValues(0.3, 160.7676);
    hp_base_table[7] = HPBaseValues(0.35, 175.5225);
    hp_base_table[8] = HPBaseValues(0.4, 191.1898);
    hp_base_table[9] = HPBaseValues(0.45, 207.7812);
    hp_base_table[10] = HPBaseValues(0.5, 225.3041);
    hp_base_table[11] = HPBaseValues(0.55, 243.7607);
    hp_base_table[12] = HPBaseValues(0.6, 263.1478);
    hp_base_table[13] = HPBaseValues(0.65, 283.4566);
    hp_base_table[14] = HPBaseValues(0.7, 304.6722);
    hp_base_table[15] = HPBaseValues(0.75, 326.7734);
    hp_base_table[16] = HPBaseValues(0.8, 349.7323);
    hp_base_table[17] = HPBaseValues(0.85, 373.5140);
    hp_base_table[18] = HPBaseValues(0.9, 398.0770);
    hp_base_table[19] = HPBaseValues(0.95, 423.3724);
    hp_base_table[20] = HPBaseValues(1, 449.3441);
    hp_base_table[21] = HPBaseValues(1, 475.9290);
    hp_base_table[22] = HPBaseValues(1, 503.0570);
    hp_base_table[23] = HPBaseValues(1, 530.6509);
    hp_base_table[24] = HPBaseValues(1, 558.6270);
    hp_base_table[25] = HPBaseValues(1, 586.8953);
    hp_base_table[26] = HPBaseValues(1, 615.3598);
    hp_base_table[27] = HPBaseValues(1, 643.9188);
    hp_base_table[28] = HPBaseValues(1, 672.4658);
    hp_base_table[29] = HPBaseValues(1, 700.8901);
    hp_base_table[30] = HPBaseValues(1, 729.0768);
    hp_base_table[31] = HPBaseValues(1, 756.9086);
    hp_base_table[32] = HPBaseValues(1, 784.2654);
    hp_base_table[33] = HPBaseValues(1, 811.0261);
    hp_base_table[34] = HPBaseValues(1, 837.0691);
    hp_base_table[35] = HPBaseValues(1, 862.2729);
    hp_base_table[36] = HPBaseValues(1, 886.5175);
    hp_base_table[37] = HPBaseValues(1, 909.6852);
    hp_base_table[38] = HPBaseValues(1, 931.6613);
    hp_base_table[39] = HPBaseValues(1, 952.3353);
    hp_base_table[40] = HPBaseValues(1, 971.6017);
    hp_base_table[41] = HPBaseValues(1, 989.3612);
    hp_base_table[42] = HPBaseValues(1, 1005.5210);
    hp_base_table[43] = HPBaseValues(1, 1019.9950);
    hp_base_table[44] = HPBaseValues(1, 1032.7080);
    hp_base_table[45] = HPBaseValues(1, 1043.5910);
    hp_base_table[46] = HPBaseValues(1, 1052.5850);
    hp_base_table[47] = HPBaseValues(1, 1059.6440);
    hp_base_table[48] = HPBaseValues(1, 1064.7290);
    hp_base_table[49] = HPBaseValues(1, 1067.8130);
    hp_base_table[50] = HPBaseValues(1, 1070.8970);
}

static QMap<QString, HPArchetypeValues> hp_archetype_table;

inline void getArchetypeHPMod()
{
    hp_archetype_table["Blaster"] = HPArchetypeValues(1.125, 1);
    hp_archetype_table["Controller"] = HPArchetypeValues(0.95, 1);
    hp_archetype_table["Defender"] = HPArchetypeValues(0.95, 1);
    hp_archetype_table["Scrapper"] = HPArchetypeValues(1.25, 1.5);
    hp_archetype_table["Tanker"] = HPArchetypeValues(1.75, 2.2);
    hp_archetype_table["Peacebringer"] = HPArchetypeValues(1, 1.5);
    hp_archetype_table["Warshade"] = HPArchetypeValues(1, 1.5);
    hp_archetype_table["Corruptor"] = HPArchetypeValues(1, 1);
    hp_archetype_table["Dominator"] = HPArchetypeValues(0.95, 1);
    hp_archetype_table["Mastermind"] = HPArchetypeValues(0.75, 1);
    hp_archetype_table["Brute"] = HPArchetypeValues(1.4, 2);
    hp_archetype_table["Stalker"] = HPArchetypeValues(1.125, 1.3);
    hp_archetype_table["Arachnos Soldier"] = HPArchetypeValues(1, 1.5);
    hp_archetype_table["Arachnos Widow"] = HPArchetypeValues(1, 1.5);
}

inline float setHPBase(const Character *src)
{
    // ( ( (Base HP Archetype Modifier - 1) * Level Modifier) + 1) * Level HP
    float hp;
    QString m_class_name = src->getClass();
    HPArchetypeValues a_val = hp_archetype_table[m_class_name];
    HPBaseValues b_val = hp_base_table[src->getLevel()];
    hp = (((a_val.mod - 1) * b_val.mod) + 1) * b_val.base;
    
    return hp;
}

inline float setArchetypeHPMod(const Character *src)
{
    // ( ( (Max HP Cap Archetype Modifier - 1) * Level Modifier) + 1) * 1.5 * Level HP
    float hp;
    QString m_class_name = src->getClass();
    HPArchetypeValues a_val = hp_archetype_table[m_class_name];
    HPBaseValues b_val = hp_base_table[src->getLevel()];
    hp = (((a_val.cap - 1) * b_val.mod) + 1) * 1.5 * b_val.base;
    
    return hp;
}
