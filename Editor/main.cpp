#include "AuthClient.h"

#include "core/os/os.h"
#include "core/class_db.h"
#include "core/version.h"
#include "main/main.h"
#include "EASTL/unique_ptr.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <climits>
#include <clocale>
#include <cstdlib>


#ifdef Q_OS_WIN32
#include <windows.h>
static HINSTANCE godot_hinstance = nullptr;
#else
static void *godot_hinstance = nullptr;
#endif

/* NOTE: enable this to set breakpoints on qdebug messages. */
#define WRAP_QT_MESSAGES
#ifdef WRAP_QT_MESSAGES
static void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &lMessage)
{
    QString text;
    switch (type)
    {
    case QtDebugMsg:
        text = QString("Debug: %1").arg(lMessage.constData());
        break;

    case QtWarningMsg:
        text = QString("Warning: %1").arg(lMessage.constData());
        break;

    case QtCriticalMsg:
        text = QString("Critical: %1").arg(lMessage.constData());
        break;

    case QtFatalMsg:
        text = QString("Fatal: %1").arg(lMessage.constData());
        abort();
    }
    QByteArray az = text.toUtf8();
    printf("%s\n",az.data());
}

#endif

void register_segs_types() {
    ClassDB::register_class<AuthClient>();
}
static int mainT(int argc, char *argv[]) {

#ifdef WRAP_QT_MESSAGES
    qInstallMessageHandler(myMessageOutput);
#endif
    QCoreApplication app(argc,argv);
    QCoreApplication::setApplicationName("SegsEditor");
    QCoreApplication::setApplicationVersion("0.0.1");
    QCoreApplication::setOrganizationName("Segs");

    eastl::unique_ptr<OS> os(instantiateOS(godot_hinstance));
    setlocale(LC_CTYPE, "");

    QString cwd = QDir::currentPath();

    Error err = Main::setup();

    register_segs_types();

    if (err != OK) {
        return 255;
    }

    if (Main::start())
        os->run(); // it is actually the OS that decides how to run
    Main::cleanup();

    if (!QDir::setCurrent(cwd)) {
        ERR_PRINT("Couldn't return to previous working directory.");
    }

    return os->get_exit_code();
}

int main(int argc, char *argv[]) {
#ifdef CRASH_HANDLER_EXCEPTION
    godot_hinstance = GetModuleHandle(nullptr);
    __try {
        return mainT<OS_Windows>(argc,argv);
    } __except (CrashHandlerException(GetExceptionInformation())) {
        return 1;
    }
#else
    return mainT(argc,argv);
#endif
}
