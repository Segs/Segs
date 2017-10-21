#ifndef FILESIGNATUREWORKER_H
#define FILESIGNATUREWORKER_H

#include <QObject>

///
/// \brief The FileSignatureWorker class is meant as a worker that calculates file hashes
///
/// The instances of this class should be ran by QThread
/// \todo Consider allowing work-splitting
class FileSignatureWorker : public QObject
{
    Q_OBJECT
    class AppVersionManifest *m_manifest;
public:
    explicit FileSignatureWorker(QObject *parent = 0);
    void setDataToProcess(AppVersionManifest *m) { m_manifest= m; }
signals:
    void finished();
    void progress(QString channel,float percentage);
    void error(QString err);
public slots:
    void process();
};

#endif // FILESIGNATUREWORKER_H
