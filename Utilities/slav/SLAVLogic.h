#ifndef SLAVLOGIC_H
#define SLAVLOGIC_H

#include <QObject>

class AppVersionManifest;
struct ServerDescriptor;

class SLAVLogic : public QObject
{
    Q_OBJECT
    AppVersionManifest *m_current_manifest;
    class ServerConnection *m_serv_conn;
    class UpdaterDlg *m_ui_impl;

    bool needToOverwriteBaseInstall();
    void createBackupIfNeeded();
    void copyFilesOverBase();
    void calculateHashes(AppVersionManifest *manifest);
public:
    explicit SLAVLogic(QObject *parent = 0);

    void connectUI(class UpdaterDlg *dlg);
    void start();
signals:
    // Notifying the UI
    void needUpdate(AppVersionManifest *manifest,const QString &changeLog);
private slots:
    void onHashingProgress(QString chan,float percent);
    void onThreadError(QString v);
public slots:
    // called from Network
    void onManifestReceived(const QString & manifest_url, const QString &manifestData);
    // called from UI
    void onUpdateRequested(ServerDescriptor * manifest);

};


#endif // SLAVLOGIC_H
