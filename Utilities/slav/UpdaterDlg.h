/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
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
