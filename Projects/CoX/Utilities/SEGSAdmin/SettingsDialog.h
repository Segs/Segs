#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QValidator>
#include <QCheckBox>

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
    void generate_default_config_file(QString ip);
    void save_changes_config_file();
    void auto_populate_ip_main(QString local_ip);
    void field_validator();
    void set_default_values();
    void text_edit_updater();
    void slider_updater();
    void auto_logout_checkbox_validator();
    void xp_mod_checkbox_validator();
    void purge_logs();
    void remove_files(QString dir, QStringList files);

signals:
   void checkForConfigFile();
   void check_data_and_dir(QString maps_dir);
   void sendMapsDir(QString maps_dir);
   void sendMapsDirConfigCheck(QString maps_dir);

private:
    Ui::SettingsDialog *ui;


};

#endif // SETTINGSDIALOG_H
