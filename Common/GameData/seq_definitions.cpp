#include "seq_definitions.h"
#include "anim_definitions.h"
#include "Utils/string_utils.h"
#include "Components/Logging.h"

int16_t getSeqMoveIdxByName(const String &name, const SequencerData &seq)
{
    int cnt = (int)seq.m_Move.size();
    String compare_against=name.to_lower();
    for (int i = 0; i < cnt; ++i )
    {
        if ( seq.m_Move[i].name.to_lower()==compare_against )
            return (int16_t)i;
    }
    sDebug() << "Cannot find move" << name;
    return -1;
}

void cleanSeqFileName(String &filename)
{
    filename = PathUtils::simplify_path(filename.to_upper());
    auto loc = filename.find("SEQUENCERS/");

    if(loc!=String::npos)
    {
        filename = filename.substr(loc+strlen("SEQUENCERS/"));
    }
}

SequencerData *SequencerList::getSequencerData(const String &seq_name)
{
    SequencerData *seq_data=nullptr;
    String modifiable_name = seq_name;
    cleanSeqFileName(modifiable_name);
    auto iter = m_Sequencers.find(modifiable_name.to_lower());
    if(iter!=m_Sequencers.end())
        seq_data = &sq_list[iter->second];
    if(!seq_data)
    {
        sWarning()<<"Missing sequencer data for"<<seq_name<<":"<<modifiable_name;
        return nullptr;
    }
    return seq_data;
}

