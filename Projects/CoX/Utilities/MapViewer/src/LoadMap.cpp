#include <QtGui>
#include "mainwindow.h"
//#include <ace/ACE.h>
int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    MainWindow wnd;
    wnd.show();
    app.exec();
    return 0;
};
