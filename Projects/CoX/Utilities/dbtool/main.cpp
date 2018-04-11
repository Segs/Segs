/*
* SEGS dbtool v0.2 dated 2017-11-04
* A database creation and management tool.
*/
#include "PasswordHasher/PasswordHasher.h"

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

static QString segs = "segs";
static QString segs_game = "segs_game";

static bool fileExists(QString path)
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
            fprintf(stderr, "%sCRITICAL!%s %s (%s:%u, %s)\n", colorTable[2], resetColor, localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtFatalMsg:
            fprintf(stderr, "%sFATAL!%s %s (%s:%u, %s)\n", colorTable[3], resetColor, localMsg.constData(), context.file, context.line, context.function);
            abort();
    }
}

void createDatabases()
{
    QDir db_dir(QDir::currentPath());
    qInfo() << "Creating database files...";
    std::map<QString, QString> files_and_targets =
    {
        {
            db_dir.currentPath() + "/default_dbs/sqlite/segs_sqlite_create.sql",
            db_dir.currentPath() + "/" + segs
        },
        {
            db_dir.currentPath() + "/default_dbs/sqlite/segs_game_sqlite_create.sql",
            db_dir.currentPath() + "/" + segs_game
        }
    };
//    if(nofile)
//        db_files << db_dir.currentPath() + "/default_dbs/sqlite/segs_sqlite_create.sql" << db_dir.currentPath() + "./default_dbs/sqlite/segs_game_sqlite_create.sql";
//    else
//        db_files = args;
    for(const std::pair<QString, QString> &source_and_target : files_and_targets)
    {
        const QString &source_file_string(source_and_target.first);
        const QString &target_file_string(source_and_target.second);
        if(!QFileInfo(source_file_string).isReadable())
        {
          qCritical() << source_file_string << "is not readable! Please check that the file is present and not corrupted.";
          break;
        }
        QFile source_file(source_file_string);
        QFile target_file(target_file_string);
        QSqlDatabase segs_db(QSqlDatabase::addDatabase("QSQLITE",target_file_string));
        QSqlQuery query(segs_db);
        if(target_file.exists())
        {
            if(!target_file.remove()) // We have to remove the file if it already exists; otherwise, many errors are thrown.
            {
                qInfo("FAILED to remove existing file:");
                qInfo().noquote()<<target_file_string;
                qFatal("Ensure no processes are using it and you have permission to modify it.");
            }
        }
        segs_db.setDatabaseName(target_file_string);
        segs_db.open(); // /*INSERT INTO accounts VALUES(1,'segsadmin',1,'2017-11-11 17:41:19',X'7365677331323300000000000000');*/ <- ignore this
        if(source_file.open(QIODevice::ReadOnly)) // Execute each command in the source file.
        {
            // The SQLite driver executes only a single (the first) query in the QSqlQuery.
            // If the script contains more queries, it needs to be split.
            QStringList scriptQueries = QTextStream(&source_file).readAll().split(';');

            foreach(QString queryTxt, scriptQueries)
            {
                if(queryTxt.trimmed().isEmpty())
                {
                    continue;
                }
                if(!query.exec(queryTxt))
                {
                    segs_db.rollback(); // Roll back the database if something goes wrong, so we're not left with useless poop.
                    qFatal("One of the query failed to execute.\n Error detail: %s\n",qPrintable(query.lastError().text()));
                }
                query.finish();
            }
        }
        segs_db.close();
        qInfo() << "COMPLETED creating:" << target_file_string;
    }
}

void addAccount(const QString & username, const QString & password, uint16_t access_level)
{
    QDir db_dir(QDir::currentPath());
    const QStringList &target_file_string = {db_dir.currentPath() + "/" + segs,
                                             db_dir.currentPath() + "/" + segs_game
                                            };

// segs database file
    QFile target_file1(target_file_string[0]);

    QSqlDatabase auth_db(QSqlDatabase::addDatabase("QSQLITE", segs));
    auth_db.setDatabaseName(target_file_string[0]);
    auth_db.open();
    QSqlQuery auth_query(auth_db);
    if(!auth_query.prepare("INSERT INTO accounts (username,passw,access_level,salt) VALUES (?,?,?,?);"))
    {
        qDebug() << "SQL_ERROR:" << auth_query.lastError();
        return;
    }

    PasswordHasher hasher;
    QByteArray salt = hasher.generateSalt();
    QByteArray password_array = hasher.hashPassword(password.toUtf8(), salt);
    auth_query.bindValue(0, username);
    auth_query.bindValue(1, password_array);
    auth_query.bindValue(2, access_level);
    auth_query.bindValue(3, salt);

    if(!target_file1.exists())
        qFatal("Target file could not be found. Verify its existence and try again.");
    if(!auth_query.exec())
    {
        auth_db.rollback(); // Roll back the database if something goes wrong, so we're not left with useless poop.
        qDebug() << "SQL_ERROR:" << auth_query.lastError(); // Why the query failed
        return;
    }
    uint32_t account_id = auth_query.lastInsertId().toInt();
    auth_query.finish();
    auth_db.close();

// segs_game database file
    uint32_t max_slots = 8;
    QFile target_file2(target_file_string[1]);

    QSqlDatabase game_db(QSqlDatabase::addDatabase("QSQLITE", segs_game));
    game_db.setDatabaseName(target_file_string[1]);
    game_db.open();
    QSqlQuery game_query(game_db);
    if(!game_query.prepare("INSERT INTO accounts (account_id,max_slots) VALUES (?,?);"))
    {
        qDebug() << "SQL_ERROR:" << game_query.lastError();
        return;
    }

    game_query.bindValue(0, account_id);
    game_query.bindValue(1, max_slots);

    if(!target_file2.exists())
        qFatal("Target file could not be found. Verify its existence and try again.");
    if(!game_query.exec())
    {
        game_db.rollback(); // Roll back the database if something goes wrong, so we're not left with useless poop.
        qDebug() << "SQL_ERROR:" << game_query.lastError(); // Why the query failed
        return;
    }
    game_query.finish();
    game_db.close();
    qInfo() << "Successfully added user" << username << "to databases:" << segs << segs_game;
}

int main(int argc, char **argv)
{
    const QStringList known_commands {"create","adduser"};
    QLoggingCategory::setFilterRules("*.debug=true\nqt.*.debug=false");
    qInstallMessageHandler(errorHandler);
    QCoreApplication app(argc,argv);
    QCoreApplication::setApplicationName("segs-dbtool");
    QCoreApplication::setApplicationVersion("0.4");

    QCommandLineParser parser;
    parser.setApplicationDescription("SEGS database management utility");
    parser.addHelpOption();
    parser.addVersionOption();


    parser.addPositionalArgument("command", QCoreApplication::translate("main", "Command to execute."),"(create|adduser)");

    // A boolean option with multiple names (-f, --force)
    QCommandLineOption forceOption(QStringList() << "f" << "force",
        QCoreApplication::translate("main", "Overwrite existing database files. THIS CANNOT BE UNDONE."));
    QCommandLineOption loginOption(QStringList() << "l" << "login",
        QCoreApplication::translate("main", "Provide login for added account"),"login");
    QCommandLineOption passOption(QStringList() << "p" << "pass",
        QCoreApplication::translate("main", "Provide password for added account"),"password");
    QCommandLineOption accessLevelOption(QStringList() << "a" << "access_level",
        QCoreApplication::translate("main", "Provide access_level [1-9] for account"),"access_level","1");
    parser.addOption(forceOption);
    parser.addOption(loginOption);
    parser.addOption(passOption);
    parser.addOption(accessLevelOption);

    parser.process(app);
    const QStringList positionalArguments = parser.positionalArguments();
    if(positionalArguments.size()<1 || !known_commands.contains(positionalArguments.first()))
    {
        if(positionalArguments.size()>=1)
            qDebug()<<"Unkown command"<<positionalArguments.first();
        else
            qDebug()<<"Command is required";
        parser.showHelp(1);
    }
    int selected_operation = known_commands.indexOf(positionalArguments.first());
    switch (selected_operation) {
        case 0:
        {
            bool forced = parser.isSet(forceOption);
            // Check if dbtool is being run from server directory
            qInfo() << "Checking for default_dbs directory...";
            QDir default_dbs_dir(QDir::currentPath() + "/default_dbs");
            if(!default_dbs_dir.exists())
            {
                qDebug() << "SEGS dbtool must be run from the SEGS root folder (where the default_dbs directory resides)";
                Pause();
                return 0;
            }
            qInfo() << "default_dbs directory found!";

            // Check if database already exists
            qInfo() << "Checking for existing databases OR -f command...";
            if((fileExists(segs) || fileExists(segs_game)) && !forced)
            {
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
            addAccount("segsadmin", "segs123", 9);
            break;
        }
        case 1:
        {
            if(!parser.isSet(loginOption) || !parser.isSet(passOption))
            {
                qCritical()<< "adduser operation requires login and password";
                return -1;
            }
            if(!fileExists(segs))
            {
                qCritical() << "Cannot add account, the database does not exist";
                return -1;
            }
            addAccount(parser.value(loginOption),parser.value(passOption),parser.value(accessLevelOption).toUInt());
        }
    }
    Pause();
    return 0;
}
