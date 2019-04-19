#include "example_service.h"

#include <QDebug>
#include <QtGlobal>

ExampleService::ExampleService() = default;

ExampleService::~ExampleService() = default;

int ExampleService::getRandomInt(int limit)
{
    return qrand() % limit;
}

QString ExampleService::printMessage(const QString& msg)
{
    qDebug().noquote() << QString("-> printMessage: '%1'").arg(msg);
    return QString("Return: '%1'").arg(msg);
}

void ExampleService::printNotification(const QString &msg) {
    qDebug().noquote() << QString("-> printNotification: '%1'").arg(msg);
}
