#include "Worker.h"

#include <QProcess>
#include <QDebug>
#include <QDir>

Worker::Worker(QObject *parent) : QObject(parent)
{

}

Worker::~Worker()
{

}

// Work through pigg files in working directory and process
void Worker::piggDispatcher()
{
    emit sendUIMessage("Extracting pigg files");
    float counter = 0;
    QStringList fileTypes;
    fileTypes << "*.pigg";
    QDir sourceDir(QDir::currentPath().append("/data"));
    QFileInfoList piggInfoList = sourceDir.entryInfoList(fileTypes);

    foreach(QFileInfo fileInfo, piggInfoList)
    {
        QString program = "piggtool";
        QStringList arguments = {"-x", fileInfo.absoluteFilePath(), QDir::currentPath().append("/data")};
        #if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
            program.prepend("./");
        #endif

        emit sendUIMessage("Processing: " + fileInfo.fileName());
        if(!processPiggFile(program, arguments))
        {
            emit sendUIMessage("Failed to process: " + fileInfo.fileName());
        }
        //Delete pigg file once done with it
        QFile::remove(fileInfo.absoluteFilePath());

        // Calculate percentage complete and send back to UI
        float progress = counter / piggInfoList.count() * 100;
        int rounded_progress = qRound(progress);
        emit sendUIPercentage(rounded_progress);
        counter++;
    }
    emit sendUIPercentage(100);

    if(runBinConverter())
        emit dataReady();
}

// Process PiggFile
bool Worker::processPiggFile(const QString program, const QStringList arguments)
{
    QProcess* pigg_tool = new QProcess();
    pigg_tool->start(program, arguments);

    if(!pigg_tool->waitForStarted())
        return false;

    pigg_tool->waitForFinished();
    pigg_tool->deleteLater();
    return true;
}

// Runs binconverter for ent_types conversion after piggtool extraction
bool Worker::runBinConverter()
{
    emit sendUIMessage("Starting BinConverter");
    QString program = "binConverter";
    QStringList arguments = {QDir::currentPath().append("/data/ent_types")};
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    program.prepend("./");
#endif
    QProcess* bin_converter = new QProcess(this);
    bin_converter->start(program, arguments);

    if(!bin_converter->waitForStarted())
    {
        QString error = "BinConverter Error: " + bin_converter->errorString();
        emit sendUIMessage(error);
        return false;
    }

    bin_converter->waitForFinished();
    emit sendUIMessage("BinConverter Successful");
    bin_converter->deleteLater();
    return true;
}
