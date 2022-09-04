#include "SegsOriginEditor.h"
#include "MainWindow.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char **argv) {
    QApplication app(argc,argv);
    MainWindow main_wnd;
    qDebug() << "Starting up";
    main_wnd.show();
    return app.exec();
}
