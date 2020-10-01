#include "particle_definitions.h"

void cleanupPSystemName(QByteArray &name)
{
    int idx = name.indexOf("/FX/",0);
    if(idx==-1 && name.startsWith("FX/"))
        idx=0;
    if(idx!=-1)
    {
        name = name.mid(idx+3).toUpper();
    }
}

