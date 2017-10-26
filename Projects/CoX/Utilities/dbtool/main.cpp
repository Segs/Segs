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
    QCommandLineParser parser;
    parser.setApplicationDescription("dbtool utility");
    parser.addOptions({
        {"new", "Create fresh database from "},
    });
    parser.addPositionalArgument("output", "File to process");
    parser.addHelpOption();

    parser.process(app);
    if(parser.positionalArguments().isEmpty() || parser.optionNames().isEmpty()) {
        parser.showHelp(0);
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
