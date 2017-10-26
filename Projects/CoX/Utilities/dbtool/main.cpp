/* 
* SEGS dbtool v0.1 
* A database creation and management tool.
*/
#include <stdio.h>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>

int main(int argc, char **argv)
{
    QCoreApplication app(argc,argv);
    QCoreApplication::setApplicationName("segs-dbtool");
    QCoreApplication::setApplicationVersion("0.1");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("SEGS database management utility");
    parser.addHelpOption();
    parser.addVersionOption();
    
    parser.addPositionalArgument("file", QCoreApplication::translate("main", "The file to open."));
    
    // A boolean option with multiple names (-f, --force)
    QCommandLineOption forceOption(QStringList() << "f" << "force",
            QCoreApplication::translate("main", "Overwrite existing files."));
    parser.addOption(forceOption);

    parser.process(app);
    if(parser.positionalArguments().isEmpty() || parser.optionNames().isEmpty()) {
        parser.showHelp(0);
    }
    
    const QStringList args = parser.positionalArguments();
    // source is args.at(0), destination is args.at(1)

    bool force = parser.isSet(forceOption);
    
    bool fileExists(QString path) {
      QFileInfo check_file(path);
      return check_file.exists() && check_file.isFile();
    }
    
    // Check if dbtool is being run from server directory
    if(fileExists("authserver")) {
        qDebug() << "SEGS dbtool must be run from the SEGS root folder (where authserver resides)";
        system("pause");
        return 0;
    }
    // Check if database already exists
    if(fileExists("segs.sql") && fileExists("segs_game.sql")) {
        qDebug() << "SEGS databases already exist. Run dbtool with -f option to overwrite existing databases. THIS CANNOT BE UNDONE.";
        system("pause");
        return 0;
    }
    
    
    /* PSUEDO CODE
    if(weAreNotRunningFromServerDir())
        return RUN_US_FROM_SERVER_DIR;
    if(databaseFilesAreHere())
        return NOTHING_TO_DO;
    // create auth server account db
    QString path = SEGS_ACCOUNTS_DB_PATH;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    db.open();
    QSqlQuery query;
    QFile db_template("segs.sql");
    if(db_template.open(QFile::ReadOnly)
        query.exec(db_template.ReadAll());
    db.close();
    */
    
    system("pause");
    return 0;
}
