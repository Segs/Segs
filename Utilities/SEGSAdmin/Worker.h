#ifndef WORKER_H
#define WORKER_H

#include <QObject>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    virtual ~Worker();

signals:
    void workerCompleted();
    void sendUIMessage(QString message);
    void sendUIPercentage(int percentage);
    void piggWorkerFailed();
    void piggWorkerSucceeded();
    void dataReady();

public slots:
    void piggDispatcher();

private:
    bool processPiggFile(const QString program, const QStringList arguments);
    bool runBinConverter();

};

#endif // WORKER_H
