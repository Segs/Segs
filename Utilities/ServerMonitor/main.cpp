#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("Segs");
    a.setApplicationName("SegsMonitor");
    MainWindow w;
    w.show();

    return a.exec();
}
