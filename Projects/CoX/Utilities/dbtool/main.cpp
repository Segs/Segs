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
#include <QtCore/QtSql>
#include <QtCore/QDebug>
#include <QtCore/QSqlDatabase>
#include <QtCore/QSqlError>

QTextStream& qStdOut()
{
    static QTextStream ts( stdout );
    return ts;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc,argv);
    QCoreApplication::setApplicationName("segs-dbtool");
    QCoreApplication::setApplicationVersion("0.1");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("SEGS database management utility");
    parser.addHelpOption();
    parser.addVersionOption();
    
    parser.addPositionalArgument("file", QCoreApplication::translate("main", "Database Script file to import."));
    
    parser.addOptions({
        // A boolean option with a single name (-p)
        {"p",
            QCoreApplication::translate("main", "Show progress during copy")},
        // Force the tool to overwrite the existing database (-f, --force)
        {{"f", "force"},
            QCoreApplication::translate("main", "Overwrite existing database files. THIS CANNOT BE UNDONE.")},
    });

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
        qStdOut() << "SEGS dbtool must be run from the SEGS root folder (where authserver resides)";
        system("pause");
        return 0;
    }
    // Check if database already exists
    if((fileExists("segs.sql") || fileExists("segs_game.sql")) && !force) {
        qStdOut() << "SEGS databases already exist. Run dbtool with -f option to overwrite existing databases. THIS CANNOT BE UNDONE.";
        system("pause");
        return 0;
    }
    
    // Should probably classify this so we can call it twice, once for segs and again for segs_game
    QString path = "./segs.sql";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE"); //not dbConnection
    db.setDatabaseName(path);
    db.open();
    QSqlQuery qry;
    QFile db_template("./db/segs.sql");
    if(db_template.open(QFile::ReadOnly)
        query.exec(db_template.ReadAll());
       
    //...
    
    // Close our db
    db.close();
    system("pause");
    return 0;
}
