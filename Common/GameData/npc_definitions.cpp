#include "npc_definitions.h"

namespace
{
struct BodyTypeName
{
    BodyType type;
    const char *m_ent_typename;
    const char *m_tex_prefix; // used to build texture names for costume parts
};
// note: original game was using a fixed set of possible BodyTypes, maybe we should consider making this more flexible
// using a dictionary ?
static const BodyTypeName s_BodyTypes[7] =
{
  { BodyType::Male, "male", "SM" },
  { BodyType::Female, "fem", "SF" },
  { BodyType::BasicMale, "bm", "BM" },
  { BodyType::BasicFemale, "bf", "BF" },
  { BodyType::Huge, "huge", "SH" },
  { BodyType::Enemy, "enemy", "EY" },
  { BodyType::Villain, "enemy", "EY" }
};
} // end of anonymous namespace

BodyType bodyTypeForEntType(const QString &enttypename)
{
    for (const BodyTypeName &bdt : s_BodyTypes  )
    {
        if ( 0==enttypename.compare(bdt.m_ent_typename,Qt::CaseInsensitive) )
            return bdt.type;
    }
    return BodyType::Male;
}

QString entTypeFileName(const Parse_Costume *costume)
{
    if (!costume->m_EntTypeFile.isEmpty())
    {
        assert(costume->m_BodyType == bodyTypeForEntType(costume->m_EntTypeFile));
        return costume->m_EntTypeFile;
    }

    if (costume->m_BodyType != BodyType::Villain)
        return s_BodyTypes[int(costume->m_BodyType)].m_ent_typename;

    return QString();
}
QByteArray bodytype_prefix_fixup(const Parse_Costume *a1, const QByteArray &a2)
{
    QByteArray name;
    QByteArray str;

    if ( a1->m_BodyType == BodyType::Villain )
    {
        name = entTypeFileName(a1).toLatin1();
        str = name+"_"+a2+".tga";
    }
    else
    {
        str = QByteArray(s_BodyTypes[(int)a1->m_BodyType].m_tex_prefix) + "_"+a2+".tga";
    }
    return str.left(str.size()-4);
}
