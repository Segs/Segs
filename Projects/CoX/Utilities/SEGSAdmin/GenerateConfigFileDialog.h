#ifndef GENERATECONFIGFILEDIALOG_H
#define GENERATECONFIGFILEDIALOG_H

#include <QDialog>

namespace Ui {
class GenerateConfigFileDialog;
}

class GenerateConfigFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GenerateConfigFileDialog(QWidget *parent = nullptr);
    ~GenerateConfigFileDialog();
    void capture_input();

public slots:
    void on_generate_config_file();

signals:
    void sendInputConfigFile(QString server_name, QString ip);

private:
    Ui::GenerateConfigFileDialog *ui;


};

#endif // GENERATECONFIGFILEDIALOG_H
