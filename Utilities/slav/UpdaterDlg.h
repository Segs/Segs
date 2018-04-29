/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see Authors.txt)
 * This software is licensed! (See License.txt for details)
 */

#ifndef UPDATERDLG_H
#define UPDATERDLG_H

#include <QDialog>
#include <vector>

struct ProjectDescriptor;
struct ServerDescriptor;
class AppVersionManifest;

namespace Ui
{
  class UpdaterDlg;
}

class UpdaterDlg : public QDialog
{
    Q_OBJECT

public:
    explicit UpdaterDlg(QWidget *parent = 0);
    ~UpdaterDlg();
public Q_SLOTS:
    void projectListChanged(const std::vector<ProjectDescriptor> & projects);
    void onDownloadProgressed(const QString &fname,quint64 bytes,quint64 overall);
    void onUpdateAvailable(AppVersionManifest * manifest, const QString &changelog);
signals:
    void requestUpdateForServer(ServerDescriptor *sd);
private:
    Ui::UpdaterDlg *ui;
};

#endif // UPDATERDLG_H
