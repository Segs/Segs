#ifndef GETIPDIALOG_H
#define GETIPDIALOG_H

#include <QDialog>

namespace Ui {
class GetIPDialog;
}

class GetIPDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GetIPDialog(QWidget *parent = 0);
    ~GetIPDialog();
    void get_local_ip();
    void send_local_ip();

signals:
    void sendIP(QString local_ip);

private:
    Ui::GetIPDialog *ui;
    QStringList *m_local_ip;

};

#endif // GETIPDIALOG_H
