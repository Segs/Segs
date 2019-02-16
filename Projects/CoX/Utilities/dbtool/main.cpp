/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*
* SEGS dbtool v0.5 dated 2019-02-11
* A database creation and management tool.
*/

/*!
 * @addtogroup dbtool Projects/CoX/Utilities/dbtool
 * @{
 */

#include "DBConnection.h"
#include "DatabaseConfig.h"

#include "Settings.h"
#include "Logging.h"

#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <memory>

#define ProjectName "SEGS dbtool"
#define VersionNumber "0.5"
#define VersionString ProjectName " v" VersionNumber

enum dbToolCommands
{
    CREATE,
    ADDUSER,
    UPGRADE,
    INFO,
};

void Pause(void)
{
    qInfo() << endl << "Press ENTER to continue...";
    std::cin.ignore(100000, '\n');  // Ignore characters until an ENTER (newline) is received.
    return;
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

int main(int argc, char **argv)
{
    const QStringList known_commands {"create","adduser","upgrade","info"};
    dbToolResult ret = dbToolResult::SUCCESS;
    setLoggingFilter(); // Set QT Logging filters
    qInstallMessageHandler(errorHandler);
    QCoreApplication app(argc,argv);
    QCoreApplication::setApplicationName("segs-dbtool");
    QCoreApplication::setApplicationVersion(VersionNumber);

    QCommandLineParser parser;
    parser.setApplicationDescription("SEGS database management utility");
    parser.addHelpOption();
    parser.addVersionOption();

    std::vector<DatabaseConfig> configs;
    configs.emplace_back(DatabaseConfig());
    configs.emplace_back(DatabaseConfig());

    parser.addPositionalArgument("command",
                                 QCoreApplication::translate("main", "Command to execute.\ncreate\nadduser\nupgrade"),
                                 "<create|adduser|upgrade|info>");

    // A boolean option with multiple names (e.g. -f, --force)
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
            qWarning() << "Unknown command" << positionalArguments.first();
        else
            qWarning() << "Please run dbTool with a command argument.";

        parser.showHelp(1);
    }
    
    if(!configs[0].initialize_from_settings(parser.value(configFileOption), "AccountDatabase"))
    {
        qCritical(qPrintable(QString("File \"%1\" not found.").arg(parser.value(configFileOption))));
        return int(dbToolResult::SETTINGS_MISSING);
    }
    // we've just checked if settings exists, so we can safely assume it does here
    configs[1].initialize_from_settings(parser.value(configFileOption), "CharacterDatabase");
    
    // Check if dbtool is being run from server directory
    qInfo() << "Opening database templates directory...";
    QDir default_dbs_dir(Settings::getDefaultDirPath());
    if(!default_dbs_dir.exists())
    {
        qWarning() << "SEGS dbtool cannot find the SEGS root folder "
                   << "(where the default_setup directory resides)";
        return int(dbToolResult::DBFOLDER_MISSING);
    }

    // stick all db connections in a vector to iterate over when necessary
    std::vector<std::unique_ptr<DBConnection>> segs_dbs;
    for(const auto &cfg : configs)
    {
        qInfo() << "Fetching database configuration from settings.cfg for" << cfg.m_name;
        segs_dbs.push_back(std::make_unique<DBConnection>(cfg));
        segs_dbs.back()->open();

        if(!segs_dbs.back()->isConnected())
        {
            qWarning() << "Database" << cfg.m_db_path << "does not exists.";
            segs_dbs.back()->close();
            continue;
        }
    }

    // Handle command argument
    int selected_operation = known_commands.indexOf(positionalArguments.first());
    switch (selected_operation)
    {
        case dbToolCommands::CREATE:
        {
            // Check if database already exists
            qInfo() << "Checking for existing databases OR -f command...";

            bool forced = parser.isSet(forceOption);
            if(!forced)
            {
                for(const auto &db : segs_dbs)
                {
                    if(db->isConnected())
                        qWarning() << "Database" << db->m_config.m_db_path << "already exists.";
                }
                
                qInfo() << "Run dbtool with -f option to overwrite ALL existing databases. "
                        << "THIS CANNOT BE UNDONE.";
                ret = dbToolResult::NOT_FORCED;
                break;
            }

            // if forced flag was used, create databases
            qWarning() << "Forced flag used '-f'. ALL existing databases will be overwritten.";
            for(const auto &db : segs_dbs)
                ret = db->createDB();

            /*!
             * @brief Replaced default admin user creation with a warning that no admin account was created.
             * @brief Also added text to show them the proper format in which to enter to submit an admin account.
             */
            qInfo() << "\nNO ADMIN USER ACCOUNTS CREATED IN DATABASE!"
                    << "\nUse the following example to add an admin account to the database:"
                    << "\ndbtool adduser -l <username> -p <password> -a 9";

            break;
        }
        case dbToolCommands::ADDUSER:
        {
            if(!parser.isSet(loginOption) || !parser.isSet(passOption))
            {
                qCritical()<< "adduser operation requires login and password";
                return int(dbToolResult::NOT_ENOUGH_PARAMS);
            }
            if(configs[0].isSqlite() && !fileExists(configs[0].m_db_path))
            {
                qCritical() << "Cannot add account, the database does not exist";
                return int(dbToolResult::SQLITE_DB_MISSING);
            }
            ret = segs_dbs[0]->addAccount(parser.value(loginOption),
                                     parser.value(passOption),
                                     parser.value(accessLevelOption).toUInt());
            break;
        }
        case dbToolCommands::UPGRADE:
        {
            for(auto &db : segs_dbs)
                db->runUpgrades();

            break;
        }
        case dbToolCommands::INFO:
        {
            qInfo() << "\nDatabase Information:";
            for(auto &db : segs_dbs)
            {
                qInfo().noquote() << QString("Database: %1").arg(db->getName());
                qInfo().noquote() << QString("    Version: %1").arg(db->getDBVersion());
                qInfo().noquote() << QString("    Driver:  %1").arg(db->m_config.m_driver);
                qInfo().noquote() << QString("    Host:    %1:%2").arg(db->m_config.m_host).arg(db->m_config.m_port);
                qInfo().noquote() << QString("    User:    %1").arg(db->m_config.m_user);
                qInfo().noquote() << QString("    Pass:    %1").arg(db->m_config.m_pass);
            }

            break;
        }
    }

    // close all database connections
    for(auto &db : segs_dbs)
        db->close();

    Pause();
    return int(ret);
}

//! @}
