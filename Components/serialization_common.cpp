#include "Components/serialization_common.h"

#include <QtCore/QDir>

QIODevice *QFSWrapper::open(const QString &path, bool read_only, bool text_only)
{
    QFile *res = new QFile(path);
    if (!res->open((read_only ? QFile::ReadOnly : QFile::ReadWrite) | (text_only ? QFile::Text : QFile::NotOpen)))
    {
        delete res;
        return nullptr;
    }
    return res;
}

bool QFSWrapper::exists(const QString &path)
{
    return QFile::exists(path);
}

QStringList QFSWrapper::dir_entries(const QString &path)
{
    return QDir(path).entryList(QDir::Files | QDir::NoSymLinks);
}
