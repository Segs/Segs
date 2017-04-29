#include <QtGui/QSurfaceFormat>
#include <QtWidgets/QApplication>
#include "SLAVLogic.h"
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    SLAVLogic app_logic;
    app_logic.start();
    app.exec();
    return 0;
}
