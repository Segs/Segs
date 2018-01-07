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

void createDatabases()
{
    QDir db_dir(QDir::currentPath());
    qInfo() << "Creating database files...";
    QStringList db_files;
    db_files << db_dir.currentPath() + "/default_dbs/sqlite/segs_sqlite_create.sql" << db_dir.currentPath() + "./default_dbs/sqlite/segs_game_sqlite_create.sql";
//    if(nofile)
//        db_files << db_dir.currentPath() + "/default_dbs/sqlite/segs_sqlite_create.sql" << db_dir.currentPath() + "./default_dbs/sqlite/segs_game_sqlite_create.sql";
//    else
//        db_files = args;
    int i = 0;
    for(const QString &db_file_string : db_files)
    {
        if(!QFileInfo(db_file_string).isReadable())
        {
          qCritical() << db_file_string << "is not readable! Please check that the file is present and not corrupted.";
          break;
        }
        QFile scriptFile(db_file_string);
        QString path;
        if(i == 0)
            path = db_dir.currentPath() + "/segs";
        else
            path = db_dir.currentPath() + "/segs_game";
        QFile db_file(path);
        QSqlDatabase segs_db(QSqlDatabase::addDatabase("QSQLITE"));
        QSqlQuery query(segs_db);
        if(db_file.exists())
            db_file.remove();
        segs_db.setDatabaseName(path);
        segs_db.open(); // /*INSERT INTO accounts VALUES(1,'segsadmin',1,'2017-11-11 17:41:19',X'7365677331323300000000000000');*/ <- ignore this
        if(scriptFile.open(QIODevice::ReadOnly))                                                // Execute each command in the source file.
        {
            // The SQLite driver executes only a single (the first) query in the QSqlQuery.
            // If the script contains more queries, it needs to be split.
            QStringList scriptQueries = QTextStream(&scriptFile).readAll().split(';');

            foreach(QString queryTxt, scriptQueries)
            {
                if(queryTxt.trimmed().isEmpty())
                {
                    continue;
                }
                if(!query.exec(queryTxt))
                {
                    qFatal(QString("One of the query failed to execute."
                                " Error detail: " + query.lastError().text()).toLocal8Bit());
                }
                query.finish();
            }
        }
        segs_db.close();
        if(i == 0)
            qInfo() << "COMPLETED creating \"segs\".";
        else
            qInfo() << "COMPLETED creating \"segs_game\".";
        i++;
    }
}

int main(int argc, char **argv)
{
    QLoggingCategory::setFilterRules("*.debug=true\nqt.*.debug=false");
    qInstallMessageHandler(errorHandler);
    QCoreApplication app(argc,argv);
    QCoreApplication::setApplicationName("segs-dbtool");
    QCoreApplication::setApplicationVersion("0.3");
    
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
    
//    const QStringList args = parser.positionalArguments();

    bool forced = parser.isSet(forceOption);
//    if(args.length() != 2 && args.length() != 0)
//        qFatal("ERROR: Number of arguments must be 0 or 2.");
//    bool nofile = args.isEmpty();
    
    // Check if dbtool is being run from server directory
    qInfo() << "Checking current directory for authserver...";
    if(!fileExists("./authserver.exe")) {
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

    createDatabases();

    Pause();
    return 0;

//    Broxen's Old Code:
//    // Let's iterate over db_files and create database files
//    for (const QString &db_file : db_files)
//    {

//        int last_slash = db_file.lastIndexOf('/',-1);
//        QString db_name = db_file.midRef(last_slash+1,-1).toString(); // filename only: segs
//        QFile db_path("./" + db_name); // destination path: ./segs

//        QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE" ,db_name);
//        // Otherwise, import contents of db_template into db_path
//        db.setDatabaseName(db_path.fileName());
//        db.setHostName("localhost");

//    // Remove both databases
//    QSqlDatabase::removeDatabase(segs);
//    QSqlDatabase::removeDatabase(segs_game);
}
