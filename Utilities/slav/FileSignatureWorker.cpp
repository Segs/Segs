/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup slav Utilities/slav
 * @{
 */

#include "FileSignatureWorker.h"

#include "ProjectManifest.h"
#include <QtCore/QCryptographicHash>
#include <QFile>
FileSignatureWorker::FileSignatureWorker(QObject *parent) : QObject(parent)
{

}

void FileSignatureWorker::process()
{
    if(m_manifest)
    {
      float prev_percentage=-1.0f;
      int current_idx=0;
      int count = m_manifest->m_files.size();
      for(auto &fileentry : m_manifest->m_files) {
          QCryptographicHash crypto(QCryptographicHash::Sha1);
          QFile file(fileentry.m_relativePath);

          current_idx++;

          if(!file.open(QFile::ReadOnly))
          {
              emit error(tr("Failed to open file %1").arg(fileentry.m_relativePath));
              continue;
          }

          while(!file.atEnd())
          {
            crypto.addData(file.read(8192));
          }
          QByteArray hash = crypto.result();
          fileentry.m_signature  = hash.toHex();
          float current_percentage = 100.0f * float(current_idx)/count;
          if(prev_percentage+1<=current_percentage) {
              emit progress(m_manifest->m_version,current_percentage);
              prev_percentage = current_percentage;
          }
      }
    }
    emit progress(m_manifest->m_version,100.0);
    emit finished();
}

//! @}
