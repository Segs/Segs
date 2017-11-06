/* 
* SEGS dbtool v0.2 dated 2017-11-04
* A database creation and management tool.
*/
#include <iostream>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

QString segs = "segs";
QString segs_game = "segs_game";

bool fileExists(QString path)
{
    QFileInfo check_file("./" + path);
    return check_file.exists() && check_file.isFile();
}

void Pause(void)
{
    qInfo() << endl << "Press ENTER to continue...";
    std::cin.ignore(100000, '\n');  // Ignore characters until an ENTER (newline) is received.
    return;
}

void errorHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();

    #ifdef Q_OS_WIN
    const char *colorTable[4] = {"","","",""};
    const char * resetColor="";
    #else
    // assume all other platforms use VT color codes
    const char * colorTable[4] = {"","\033[1;33m","\033[91m","\033[41m"};
    const char * resetColor="\033[0m";
    #endif

    switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "%s\n", localMsg.constData());
            break;
        case QtInfoMsg:
            fprintf(stderr, "%s\n", localMsg.constData());
            break;
        case QtWarningMsg:
            fprintf(stderr, "%sWARNING!%s %s\n", colorTable[1], resetColor, localMsg.constData());
            break;
        case QtCriticalMsg:
            fprintf(stderr, "%sCRITICAL!%s (%s:%u, %s)\n", colorTable[2], resetColor, localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtFatalMsg:
            fprintf(stderr, "%sFATAL!%s (%s:%u, %s)\n", colorTable[3], resetColor, localMsg.constData(), context.file, context.line, context.function);
            abort();
    }
}

int main(int argc, char **argv)
{
    QLoggingCategory::setFilterRules("*.debug=true\nqt.*.debug=false");
    qInstallMessageHandler(errorHandler);
    QCoreApplication app(argc,argv);
    QCoreApplication::setApplicationName("segs-dbtool");
    QCoreApplication::setApplicationVersion("0.2");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("SEGS database management utility");
    parser.addHelpOption();
    parser.addVersionOption();

    qInfo().noquote().nospace() << parser.applicationDescription() << " v" << QCoreApplication::applicationVersion() << endl;
    
    parser.addPositionalArgument("file", QCoreApplication::translate("main", "Database Script file to import."));
    
    // A boolean option with multiple names (-f, --force)
    QCommandLineOption forceOption(QStringList() << "f" << "force",
        QCoreApplication::translate("main", "Overwrite existing database files. THIS CANNOT BE UNDONE."));
    parser.addOption(forceOption);

    parser.process(app);
    
    const QStringList args = parser.positionalArguments();

    bool forced = parser.isSet(forceOption);
    bool nofile = args.isEmpty();
    
    // Check if dbtool is being run from server directory
    qInfo() << "Checking current directory for authserver...";
    if(!fileExists("./authserver")) {
        qDebug() << "SEGS dbtool must be run from the SEGS root folder (where authserver resides)";
        Pause();
        return 0;
    }
    qInfo() << "Authserver Found!";

    // Check if database already exists
    qInfo() << "Checking for existing databases OR -f command...";
    if((fileExists(segs) || fileExists(segs_game)) && !forced) {
        if(fileExists(segs))
            qWarning() << "Database" << segs << "already exists.";
        if(fileExists(segs_game))
            qWarning() << "Database" << segs_game << "already exists.";
        qDebug() << "Run dbtool with -f option to overwrite existing databases. THIS CANNOT BE UNDONE.";
        Pause();
        return 0;
    }

    if(forced)
        qWarning() << "Forced flag used '-f'. Existing databases may be overwritten.";

    qInfo() << "Creating database files...";
    QStringList db_files;
    if(nofile)
        db_files << "./default_dbs/segs" << "./default_dbs/segs_game";
    else
        db_files = args;

    QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE" );
    // Let's itterate over db_files and create database files
    for (const QString &db_file : db_files)
    {
        if( ! QFileInfo(db_file).isReadable() )
        {
          qCritical() << db_file << "is not readable! Please check that the file is present and not corrupted.";
          break;
        }

        QFile db_template(db_file);
        int last_slash = db_file.lastIndexOf('/',-1);
        QFile db_path("./" + db_file.midRef(last_slash+1,-1)); // filename only: ./segs
        
        // Otherwise, import contents of db_template into db_path
        db.setDatabaseName(db_path.fileName());

        if( !db.open() )
        {
            qDebug() << db.lastError();
            qFatal("Failed to connect to database. Please check error messages for details.");
        }

        QSqlQuery qry;
        if(db_template.open(QFile::ReadOnly))
        {
            qry.prepare(db_template.readAll());
            if(!qry.exec())
            {
                qCritical() << qry.lastError();
                break;
            }
        }

        qInfo() << "COMPLETED importing" << db_template.fileName() << "to" << db_path.fileName();
    }

    // Close and remove our db
    db.close();
    db.removeDatabase("QSQLITE");

    Pause();
    return 0;
}
