#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT
    class GetIPDialog *m_get_ip;

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

public slots:
    void open_settings_dialog();
    void send_maps_dir();
    void send_maps_dir_config_check();
    void read_config_file(QString filePath);
    void generate_default_config_file(QString server_name, QString ip);
    void save_changes_config_file();
    void auto_populate_ip_main(QString local_ip);

signals:
   void checkForConfigFile();
   void check_data_and_dir(QString maps_dir);
   void sendMapsDir(QString maps_dir);
   void sendMapsDirConfigCheck(QString maps_dir);

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
