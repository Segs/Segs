#include "seq_definitions.h"
#include "anim_definitions.h"

#include "Components/Logging.h"

#include <QDir>
#include <QMetaEnum>
#include <set>

int16_t getSeqMoveIdxByName(const QByteArray &name, const SequencerData &seq)
{
    int cnt = seq.m_Move.size();
    QByteArray compare_against=name.toLower();
    for (int i = 0; i < cnt; ++i )
    {
        if ( seq.m_Move[i].name.toLower()==compare_against )
            return i;
    }
    qDebug() << "Cannot find move" << name;
    return -1;
}

void cleanSeqFileName(QByteArray &filename)
{
    filename = QDir::cleanPath(filename.toUpper()).toLatin1();
    int loc = filename.indexOf("SEQUENCERS/");

    if(loc!=-1)
    {
        filename = filename.mid(loc+strlen("SEQUENCERS/"));
    }
}
SequencerData *SequencerList::getSequencerData(const QByteArray &seq_name)
{
    SequencerData *seq_data=nullptr;
    QByteArray modifiable_name = seq_name;
    cleanSeqFileName(modifiable_name);
    auto iter = m_Sequencers.find(modifiable_name.toLower());
    if(iter!=m_Sequencers.end())
        seq_data = &sq_list[*iter];
    if(!seq_data)
    {
        qWarning()<<"Missing sequencer data for"<<QString(seq_name)<<":"<<QString(modifiable_name);
        return nullptr;
    }
    return seq_data;
}

