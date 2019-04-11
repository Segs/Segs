/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*
* SEGS dbtool v0.4 dated 2018-04-22
* A database creation and management tool.
*/

/*!
 * @addtogroup dbtool Projects/CoX/Utilities/dbtool
 * @{
 */

#include "PasswordHasher.h"
#include "Settings.h"
#include "Logging.h"

#include <QtCore/QSettings>
#include <QtCore/QString>
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

#include <iostream>
#include <vector>

// Return value enumeration
enum { SUCCESS, SETTINGS_MISSING, DBFOLDER_MISSING, NOT_FORCED,
       NOT_ENOUGH_PARAMS, SQLITE_DB_MISSING, DB_RM_FAILED, DB_CONN_FAILED,
       QUERY_FAILED, QUERY_PREP_FAILED, USERNAME_TAKEN };

class ConfigStruct
{
private:
    bool putFilePath();
public:
    QString m_driver;
    QString m_simple_name;
    QString m_filename;
    QString m_db_name; // for sqlite is path, otherwise name
    QString m_host;
    QString m_port;
    QString m_user;
    QString m_pass;
    QString m_template_path;
    bool m_character_db = false;

    ConfigStruct() = default;
    
    bool initialize_from_settings(const QString &settings_file_name, const QString &group_name);
    
    bool isMysql() const {return m_driver.startsWith("QMYSQL");}
    bool isPostgresql() const {return m_driver.startsWith("QPSQL");}
    bool isSqlite() const {return m_driver.startsWith("QSQLITE");}

};

bool ConfigStruct::initialize_from_settings(const QString &settings_file_name, const QString &group_name)
{
    // Set settings file path, since we're in a different directory
    Settings::setSettingsPath(settings_file_name);
    QString settings_full_path = Settings::getSettingsPath();

    if(!fileExists(settings_full_path))
    {
        qWarning() << "Cannot find settings template file" << settings_full_path;
        return false;
    }

    QSettings config(settings_full_path, QSettings::IniFormat, nullptr);

    config.beginGroup(QStringLiteral("AdminServer"));
        config.beginGroup(group_name);
            m_driver = config.value(QStringLiteral("db_driver"),"QSQLITE").toString();
            m_filename = config.value(QStringLiteral("db_name"),"segs").toString();
            m_db_name = Settings::getSEGSDir() + QDir::separator() + m_filename; // don't add suffix here, so that it can be optional for users
            m_host = config.value(QStringLiteral("db_host"),"127.0.0.1").toString();
            m_port = config.value(QStringLiteral("db_port"),"5432").toString();
            m_user = config.value(QStringLiteral("db_user"),"segs").toString();
            m_pass = config.value(QStringLiteral("db_pass"),"segs123").toString();
            m_character_db = group_name.compare("AccountDatabase");
        config.endGroup(); // group_name
    config.endGroup(); // AdminServer

    // store database names so we can use them later in migrations
    m_simple_name = QFileInfo(m_db_name).completeBaseName();

    if(!isSqlite())
        m_db_name = m_simple_name; // for client-server db systems, use name instead of path

    qCDebug(logSettings) << m_db_name << "database settings loaded from" << settings_full_path;

    return putFilePath();
}

bool ConfigStruct::putFilePath()
{
    // Find database templates directory
    QDir tpl_dir(Settings::getTemplateDirPath());
    if(!tpl_dir.exists())
    {
        qWarning() << "SEGS dbtool cannot find the SEGS root folder "
                   << "(where the default_setup directory resides)";
        return false;
    }

    qCDebug(logSettings) << "Templates Dir" << Settings::getTemplateDirPath();

    if(isSqlite())
    {
        if(m_character_db)
            m_template_path = tpl_dir.absolutePath() + QDir::separator() + "sqlite/segs_game_sqlite_create.sql";
        else
            m_template_path = tpl_dir.absolutePath() + QDir::separator() + "sqlite/segs_sqlite_create.sql";
    }
    else if(isMysql())
    {
        if(m_character_db)
            m_template_path = tpl_dir.absolutePath() + QDir::separator() + "mysql/segs_game_mysql_create.sql";
        else
            m_template_path = tpl_dir.absolutePath() + QDir::separator() + "mysql/segs_mysql_create.sql";
    }
    else if(isPostgresql())
    {
        if(m_character_db)
            m_template_path = tpl_dir.absolutePath() + QDir::separator() + "pgsql/segs_game_postgres_create.sql";
        else
            m_template_path = tpl_dir.absolutePath() + QDir::separator() + "pgsql/segs_postgres_create.sql";
    }
    else
    {
        qCritical("Unknown database driver.");
        return false;
    }

    qCDebug(logSettings) << "m_file_path" << m_template_path;
    return true;
}

bool dbExists(const ConfigStruct &database_to_look_for)
{
    bool ret = false;
    QString querytext;
    if(database_to_look_for.isSqlite())
        return ret;
    else if(database_to_look_for.isMysql() || database_to_look_for.isPostgresql())
    {
        QSqlDatabase segs_db(QSqlDatabase::addDatabase(database_to_look_for.m_driver,
                                                       database_to_look_for.m_db_name));
        QSqlQuery query(segs_db);
        segs_db.setDatabaseName(database_to_look_for.m_db_name);
        segs_db.setHostName(database_to_look_for.m_host);
        segs_db.setPort(database_to_look_for.m_port.toInt());
        segs_db.setUserName(database_to_look_for.m_user);
        segs_db.setPassword(database_to_look_for.m_pass);
        segs_db.open();
        if(database_to_look_for.isMysql())
            querytext = "show tables";
        else
        {
            querytext = "SELECT table_schema || '.' || table_name FROM";
            querytext.append("information_schema.tables WHERE table_type = 'BASE TABLE' AND table_schema ='");
            querytext.append(database_to_look_for.m_db_name);
            querytext.append("';");
        }
        query.exec(querytext);
        if(query.size() >= 1)
            ret = true;
        
        segs_db.close();
    }
    QSqlDatabase::removeDatabase(database_to_look_for.m_db_name);
    return ret;
}

bool deleteDb(QString const& db_file_name)
{
    QFile target_file(db_file_name);
    if(target_file.exists() && !target_file.remove())
        return false;
    
    return true;
}

void Pause(void)
{
    qInfo() << endl << "Press ENTER to continue...";
    std::cin.ignore(100000, '\n');  // Ignore characters until an ENTER (newline) is received.
    return;
}

bool fileQueryDb(QFile &source_file, QSqlQuery &query)
{
    // Open file. If unsuccessful, return early.
    if(!source_file.open(QIODevice::ReadOnly))
    {
        qWarning().noquote() << "Query source file could not be opened.";
        return false;
    }
    // The SQLite driver executes only a single (the first) query in the QSqlQuery.
    // If the script contains more queries, it needs to be split.
    QStringList scriptQueries = QTextStream(&source_file).readAll().split(';');
    
    foreach(QString queryTxt, scriptQueries) // Execute each command in the source file.
    {
        if(queryTxt.trimmed().isEmpty())
            continue;
        
        if(!query.exec(queryTxt))
            return false;
        
        query.finish();
    }
    return true;
}

void errorHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static char log_buffer[4096]={0};
    static char category_text[256];
    log_buffer[0] = 0;
    category_text[0] = 0;
    if(strcmp(context.category,"default")!=0)
        snprintf(category_text, sizeof(category_text), "[%s]", context.category);

    QByteArray localMsg = msg.toLocal8Bit();

    #ifdef Q_OS_WIN
    const char *colorTable[4] = {"","","",""};
    const char * resetColor="";
    #else
    // assume all other platforms use VT color codes
    const char * colorTable[4] = {"","\033[1;33m","\033[91m","\033[41m"};
    const char * resetColor="\033[0m";
    #endif

    switch (type)
    {
        case QtDebugMsg:
            snprintf(log_buffer, sizeof(log_buffer), "%sDebug   : %s\n", category_text, localMsg.constData());
            break;
        case QtInfoMsg:
            // no prefix or category for informational messages, as these are end-user facing
            snprintf(log_buffer, sizeof(log_buffer), "%s\n", localMsg.constData());
            break;
        case QtWarningMsg:
            snprintf(log_buffer, sizeof(log_buffer), "%s%sWARNING!%s %s\n", category_text, colorTable[1], resetColor, localMsg.constData());
            break;
        case QtCriticalMsg:
            snprintf(log_buffer, sizeof(log_buffer), "%s%sCRITICAL!%s %s (%s:%u, %s)\n", category_text, colorTable[2], resetColor, localMsg.constData(),
                    context.file, context.line, context.function);
            break;
        case QtFatalMsg:
            snprintf(log_buffer, sizeof(log_buffer), "%s%sFATAL!%s %s (%s:%u, %s)\n", category_text, colorTable[3], resetColor, localMsg.constData(),
                    context.file, context.line, context.function);
            abort();
    }
    fprintf(stdout, "%s", log_buffer);
    fflush(stdout);
}

int createDatabases(std::vector<ConfigStruct> const& configs)
{
    /* Return values
       0: All went as expected.
       6: Config file not found.
       7: Removing existing database file failed.
       8: Could not connect to database.
       9: One of the SQL queries failed.
    */
    qInfo() << "Creating databases...";
    int returnvalue = SUCCESS;
    for(const ConfigStruct &cfg : configs)
    {
        if(!QFileInfo(cfg.m_template_path).isReadable())
        {
            qCritical() << cfg.m_template_path << "is not readable!"
                        << "Please check that the file is present and not corrupted.";
            returnvalue = SETTINGS_MISSING;
            break;
        }
        QFile source_file(cfg.m_template_path);
        QSqlDatabase segs_db(QSqlDatabase::addDatabase(cfg.m_driver, cfg.m_db_name));
        QSqlQuery query(segs_db);
        segs_db.setDatabaseName(cfg.m_db_name);
        if(cfg.isSqlite())
        {
            // We have to remove the file if it already exists;
            // otherwise, many errors are thrown.
            if(!deleteDb(cfg.m_db_name))
            {
                qWarning(qPrintable(QString("FAILED to remove existing file: %1").arg(cfg.m_db_name)));
                qCritical("Ensure no processes are using it and you have permission to modify it.");
                returnvalue = DB_RM_FAILED;
                break;
            }
        }
        else if(cfg.isMysql() || cfg.isPostgresql())
        {
            segs_db.setHostName(cfg.m_host);
            segs_db.setPort(cfg.m_port.toInt());
            segs_db.setUserName(cfg.m_user);
            segs_db.setPassword(cfg.m_pass);
        }

        if(!segs_db.open())
        {
            qCritical("Could not open connection to database.\n Details:%s",
                      qPrintable(segs_db.lastError().text()));
            returnvalue = DB_CONN_FAILED;
            break;
        }
        if(!fileQueryDb(source_file, query))
        {
            // Roll back the database if something goes wrong,
            // so we're not left with useless poop.
            segs_db.rollback(); 
            qCritical("One of the queries failed to execute.\n Error detail: %s\n",
                      qPrintable(query.lastError().text()));
            returnvalue = QUERY_FAILED;
            break;
        }
        source_file.close();
        segs_db.close();
        qInfo() << "COMPLETED creating:" << cfg.m_db_name;
    }
    for(auto opened: configs)
        QSqlDatabase::removeDatabase(opened.m_db_name);
    return returnvalue;
}

int addAccount(const ConfigStruct &char_database, const QString & username,
                const QString & password, uint16_t access_level)
{
    if(char_database.isSqlite())
    {
        QFile target_file(char_database.m_db_name);
        if(!target_file.exists())
        {
            qCritical("Target file could not be found. Verify its existence and try again.");
            return SQLITE_DB_MISSING;
        }
    }

    QSqlDatabase segs_db(QSqlDatabase::addDatabase(char_database.m_driver,
                                                   char_database.m_db_name));
    segs_db.setDatabaseName(char_database.m_db_name);
    if(char_database.isMysql() || char_database.isPostgresql())
    {
        segs_db.setHostName(char_database.m_host);
        segs_db.setPort(char_database.m_port.toInt());
        segs_db.setUserName(char_database.m_user);
        segs_db.setPassword(char_database.m_pass);
    }
    segs_db.open();
    QSqlQuery query(segs_db);
    if(!query.prepare("INSERT INTO accounts (username,passw,access_level,salt) VALUES (?,?,?,?);"))
    {
        qDebug() << "SQL_ERROR:" << query.lastError();
        return QUERY_PREP_FAILED;
    }

    PasswordHasher hasher;
    QByteArray salt = hasher.generateSalt();
    QByteArray password_array = hasher.hashPassword(password.toUtf8(), salt);
    query.bindValue(0, username);
    query.bindValue(1, password_array);
    query.bindValue(2, access_level);
    query.bindValue(3, salt);

    if(!query.exec())
    {
        int sqlErrorCode = query.lastError().nativeErrorCode().toInt();
        if(sqlErrorCode == 19 || sqlErrorCode == 1062 || sqlErrorCode == 23503)
        {
            // Unique constraint error.
            // SQLite:        19
            // MySQL:       1062
            // PostgreSQL: 23503
            qWarning() << "Error: Username already taken. Please try another name.";
            return USERNAME_TAKEN;
        }
        qDebug() << "SQL_ERROR:" << query.lastError(); // Why the query failed
        return QUERY_FAILED;
    }
    query.finish();
    segs_db.close();
    return SUCCESS;
}

int main(int argc, char **argv)
{
    const QStringList known_commands {"create","adduser"};
    int returnvalue = SUCCESS;
    qInstallMessageHandler(errorHandler);
    QCoreApplication app(argc,argv);
    QCoreApplication::setApplicationName("segs-dbtool");
    QCoreApplication::setApplicationVersion("0.4");

    QCommandLineParser parser;
    parser.setApplicationDescription("SEGS database management utility");
    parser.addHelpOption();
    parser.addVersionOption();

    std::vector<ConfigStruct> configs;
    configs.emplace_back(ConfigStruct());
    configs.emplace_back(ConfigStruct());

    parser.addPositionalArgument("command",
                                 QCoreApplication::translate("main", "Command to execute."),
                                 "(create|adduser)");

    // A boolean option with multiple names (-f, --force)
    QCommandLineOption forceOption(QStringList() << "f" << "force",
        QCoreApplication::translate("main", "Overwrite existing database files. THIS CANNOT BE UNDONE."));
    QCommandLineOption loginOption(QStringList() << "l" << "login",
        QCoreApplication::translate("main", "Provide login for added account"),"login");
    QCommandLineOption passOption(QStringList() << "p" << "pass",
        QCoreApplication::translate("main", "Provide password for added account"),"password");
    QCommandLineOption accessLevelOption(QStringList() << "a" << "access_level",
        QCoreApplication::translate("main", "Provide access_level [1-9] for account"),"access_level","1");
    QCommandLineOption configFileOption(QStringList() << "c" << "config",
        QCoreApplication::translate("main", "Use provided settings file, default = settings.cfg"),
                                        "config", "settings.cfg");
    parser.addOption(forceOption);
    parser.addOption(loginOption);
    parser.addOption(passOption);
    parser.addOption(accessLevelOption);
    parser.addOption(configFileOption);
    
    parser.process(app);
    const QStringList positionalArguments = parser.positionalArguments();
    if(positionalArguments.size()<1 || !known_commands.contains(positionalArguments.first()))
    {
        if(positionalArguments.size()>=1)
            qWarning()<<"Unknown command" << positionalArguments.first();
        else
            qWarning() << "Please run dbTool with a command argument.";

        parser.showHelp(1);
    }
    
    if(!configs[0].initialize_from_settings(parser.value(configFileOption), "AccountDatabase"))
    {
        qCritical(qPrintable(QString("File \"%1\" not found.").arg(parser.value(configFileOption))));
        return SETTINGS_MISSING;
    }
    configs[1].initialize_from_settings(parser.value(configFileOption), "CharacterDatabase");
    
    // Set QT Logging filters after we've initialized settings.cfg with the correct path
    setLoggingFilter();

    // Handle command argument
    int selected_operation = known_commands.indexOf(positionalArguments.first());
    switch (selected_operation)
    {
        case 0:
        {
            bool forced = parser.isSet(forceOption);
            // Check if dbtool is being run from server directory
            qInfo() << "Checking for template directory...";
            QDir default_dbs_dir(Settings::getTemplateDirPath());
            if(!default_dbs_dir.exists())
            {
                qWarning() << "SEGS dbtool must be run from the SEGS root folder "
                           << "(where the default_setup directory resides)";
                returnvalue = DBFOLDER_MISSING;
                break;
            }
            qInfo() << "DB Templates directory found!";

            // Check if database already exists
            qInfo() << "Checking for existing databases OR -f command...";
            if((((configs[0].isSqlite() && fileExists(configs[0].m_db_name)) ||
                 (configs[1].isSqlite() && fileExists(configs[1].m_db_name))) ||
                (((configs[0].isMysql() || configs[0].isPostgresql()) &&
                  dbExists(configs[0])) ||
                 ((configs[1].isMysql() || configs[1].isPostgresql()) &&
                  dbExists(configs[1]))))
               && !forced)
            {
                if(fileExists(configs[0].m_db_name) || dbExists(configs[0]))
                    qWarning() << "Database" << configs[0].m_db_name << "already exists.";
                
                if(fileExists(configs[1].m_db_name) || dbExists(configs[1]))
                    qWarning() << "Database" << configs[1].m_db_name << "already exists.";
                
                qInfo() << "Run dbtool with -f option to overwrite existing databases. "
                        << "THIS CANNOT BE UNDONE.";
                returnvalue = NOT_FORCED;
                break;
            }

            if(forced)
                qWarning() << "Forced flag used '-f'. Existing databases may be overwritten.";

            returnvalue = createDatabases(configs);

            /*!
             * @brief Replaced default admin user creation with a warning that no admin account was created.
             * @brief Also added text to show them the proper format in which to enter to submit an admin account.
             */

            qInfo() << "\nNO ADMIN USER ACCOUNTS CREATED IN DATABASE!"
                    << "\nUse the following example to add an admin account to the database:"
                    << "\ndbtool adduser -l <username> -p <password> -a 9";
            break;
        }
        case 1:
        {
            if(!parser.isSet(loginOption) || !parser.isSet(passOption))
            {
                qCritical()<< "adduser operation requires login and password";
                return NOT_ENOUGH_PARAMS;
            }
            if(configs[0].isSqlite() && !fileExists(configs[0].m_db_name))
            {
                qCritical() << "Cannot add account, the database does not exist";
                return SQLITE_DB_MISSING;
            }
            returnvalue = addAccount(configs[0], parser.value(loginOption),
                                     parser.value(passOption),
                                     parser.value(accessLevelOption).toUInt());
        }
    }
    Pause();
    return returnvalue;
}

//! @}
