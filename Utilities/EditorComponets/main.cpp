#include "SegsOriginEditor.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char **argv) {
    QApplication app(argc,argv);
    SegsOriginEditor wnd;
    qDebug() << "Starting up";
    wnd.show();
    return app.exec();
}
