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

#include "ProjectDescriptor.h"
#include "ProjectManifest.h"
#include "UpdaterDlg.h"
#include "ui_UpdaterDlg.h"

#include <QtWidgets/QMessageBox>

UpdaterDlg::UpdaterDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdaterDlg)
{
    ui->setupUi(this);
}

UpdaterDlg::~UpdaterDlg()
{
    delete ui;
}

void UpdaterDlg::projectListChanged(const std::vector<ProjectDescriptor> &projects)
{
    ui->projectSelector->clear();
    for(const ProjectDescriptor &pd : projects)
    {
        ui->projectSelector->addItem(pd.displayName(),QVariant::fromValue((void *)&pd));
    }
}

void UpdaterDlg::onDownloadProgressed(const QString &fname, quint64 bytes, quint64 overall)
{
    ui->progressBar->setMaximum(overall);
    ui->progressBar->setValue(bytes);
    ui->debug_text->appendPlainText(tr("Downloading %1 %2/%3").arg(fname).arg(bytes).arg(overall));
}

void UpdaterDlg::onUpdateAvailable(AppVersionManifest *manifest, const QString &changelog)
{
    // EIther a
    auto resp=QMessageBox::information(this,tr("Update for %1 - %2 is available")
                                       .arg(manifest->m_associated_server ? manifest->m_associated_server->m_name : "SLAV" )
                                       .arg(manifest->m_version),
                                       tr(" New version : %1\n Changelog: \n%2").arg(manifest->m_version).arg(changelog),
                                       QMessageBox::Ok|QMessageBox::Cancel);
    if(resp==QMessageBox::Ok)
    {
        emit requestUpdateForServer(nullptr);
    }
}

//! @}
