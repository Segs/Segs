#ifndef GENERATECONFIGFILEDIALOG_H
#define GENERATECONFIGFILEDIALOG_H

#include <QDialog>

namespace Ui {
class GenerateConfigFileDialog;
}

class GenerateConfigFileDialog : public QDialog
{
    Q_OBJECT
    class GetIPDialog *m_get_ip;

public:
    explicit GenerateConfigFileDialog(QWidget *parent = nullptr);
    ~GenerateConfigFileDialog();
    void capture_input();

public slots:
    void on_generate_config_file();
    void auto_populate_ip_gen_config(QString local_ip);

signals:
    void sendInputConfigFile(QString ip);

private:
    Ui::GenerateConfigFileDialog *ui;



};

#endif // GENERATECONFIGFILEDIALOG_H
