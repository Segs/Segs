#include <QtGui/QApplication>
#include "clientwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientWindow w;
    w.show();

    return a.exec();
}
