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

void errorHandler(QtMsgType type, const char *msg)
{
    switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "%s\n", msg);
            break;
        case QtWarningMsg:
            fprintf(stderr, "\033[1;33mWarning\033[0m: %s\n", msg);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "\033[31mCritical\033[0m: %s\n", msg);
            break;
        case QtFatalMsg:
            fprintf(stderr, "\033[31mFatal\033[0m: %s\n", msg);
            abort();
    }
}

int main(int argc, char **argv)
{
    qInstallMsgHandler(errorHandler);
    QCoreApplication app(argc,argv);
    QCoreApplication::setApplicationName("segs-dbtool");
    QCoreApplication::setApplicationVersion("0.2");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("SEGS database management utility");
    parser.addHelpOption();
    parser.addVersionOption();

    qDebug() << parser.applicationDescription() << " v" << QCoreApplication::applicationVersion() << endl;
    
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
    qDebug() << "Authserver Found!";

    // Check if database already exists
    qInfo() << "Checking for existing databases...";
    if((fileExists(segs) || fileExists(segs_game)) && !forced) {
        if(fileExists(segs))
            qWarning() << "WARNING! Database " << segs << " already exists.";
        if(fileExists(segs_game))
            qWarning() << "WARNING! Database " << segs_game << " already exists.";
        qDebug() << "Run dbtool with -f option to overwrite existing databases. THIS CANNOT BE UNDONE.";
        Pause();
        return 0;
    }

    qInfo() << "Creating database files...";
    QStringList db_files;
    if(nofile)
        db_files << "./default_dbs/segs" << "./default_dbs/segs_game";
    else
        db_files << args;

    // Let's itterate over db_files and create database files
    for (int i = 0; i < db_files.size(); ++i)
    {
        QFile db_template(db_file.at(i));
        int last_slash = db_file.at(i).lastIndexOf('/',-1);
        QFile db_path("./" + db_file.at(i).midRef(last_slash+1,-1)); // filename only: ./segs
        
        // Otherwise, import contents of db_template into db_path
        QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE" );
        db.setDatabaseName( db_path );

        if( !db.open() )
        {
            qDebug() << db.lastError();
            qFatal() << "Failed to connect to database. Please check error messages for details.";
            Pause();
            return 0;
        }
        qDebug() << "Created new database!";

        QSqlQuery qry;
        if(db_template.open(QFile::ReadOnly))
            qry.exec(db_template.readAll());

        // Close our db
        db.close();
        qInfo() << "COMPLETED importing " << db_template << "!";
    }

    Pause();
    return 0;
}
