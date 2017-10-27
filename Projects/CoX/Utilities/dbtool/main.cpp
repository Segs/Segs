/* 
* SEGS dbtool v0.1 
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

QTextStream& qout()
{
    static QTextStream ts( stdout );
    return ts;
}

bool fileExists(QString path)
{
    QFileInfo check_file("./" + path);
    return check_file.exists() && check_file.isFile();
}

void Pause(void)
{
    qout().flush();
    qout() << endl << "Press ENTER to continue...";
    qout().flush();
    std::cin.ignore(100000, '\n');  // Ignore characters until an ENTER (newline) is received.
    return;
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

    qout() << parser.applicationDescription() << " v" << QCoreApplication::applicationVersion() << endl << endl;
    
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
    qout() << "Checking current folder...";
    if(!fileExists("./authserver")) {
        qout() << "SEGS dbtool must be run from the SEGS root folder (where authserver resides)" << endl;
        Pause();
        return 0;
    }
    qout() << "OK" << endl;

    // Check if database already exists
    qout() << "Checking for existing databases..." << endl;
    if((fileExists(segs) || fileExists(segs_game)) && !forced) {
        if(fileExists(segs))
            qout() << "WARNING! Database " << segs << " already exists." << endl;
        if(fileExists(segs_game))
            qout() << "WARNING! Database " << segs_game << " already exists." << endl;
        qout() << "Run dbtool with -f option to overwrite existing databases. THIS CANNOT BE UNDONE." << endl;
        Pause();
        return 0;
    }

    qout() << "Creating database files...";
    QString db_path;
    if(nofile)
        db_path = "./input.sql";
    else
        db_path = args.at(0);

    // Should probably classify all of this so we can call it twice, once for segs and again for segs_game
    QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE" );
    db.setDatabaseName( "./output.sql" );

    if( !db.open() )
    {
      qDebug() << db.lastError();
      qFatal( "Failed to connect to database." );
    }

    qDebug( "Connected!" );
    qout() << "COMPLETED!!" << endl;

    QSqlQuery qry;
    QFile db_template(segs);
    if(db_template.open(QFile::ReadOnly))
        qry.exec(db_template.readAll());

    // Close our db
    db.close();

    Pause();
    return 0;
}
