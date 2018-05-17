#include "GetIPDialog.h"
#include "ui_GetIPDialog.h"
#include <QNetworkInterface>
#include <QDebug>

GetIPDialog::GetIPDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GetIPDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox,&QDialogButtonBox::accepted,this,&GetIPDialog::send_local_ip);
}

GetIPDialog::~GetIPDialog()
{
    delete ui;
}

void GetIPDialog::get_local_ip() // Return list of local IPs
{
    show();
    QStringList m_local_ip;
    ui->local_ips->clear();
    foreach (const QNetworkInterface &netInterface, QNetworkInterface::allInterfaces())
    {
        QNetworkInterface::InterfaceFlags flags = netInterface.flags();
        if( (bool)(flags & QNetworkInterface::IsRunning) && !(bool)(flags & QNetworkInterface::IsLoopBack))
        {
            foreach (const QNetworkAddressEntry &address, netInterface.addressEntries())
            {
                if(address.ip().protocol() == QAbstractSocket::IPv4Protocol)
                    m_local_ip << address.ip().toString();
            }
        }
    }
    ui->local_ips->addItems(m_local_ip);
}

void GetIPDialog::send_local_ip() // Sends chosen IP
{
    QString local_ip = ui->local_ips->currentText();
    emit sendIP(local_ip);
}


