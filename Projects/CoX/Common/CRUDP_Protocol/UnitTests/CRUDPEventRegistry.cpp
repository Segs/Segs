#include <QTest>

#include "SEGSEventFactory.h"

using namespace SEGSEvents;
extern void register_CRUDPEvents();
namespace
{
    const char *event_names[] =
    {
        "Packet",
        "ConnectRequest",
        "ConnectResponse",
        "DisconnectRequest",
        "DisconnectResponse",
        "Idle",
        "UnknownEvent",

    };
}

class CRUDPEventRegistry : public QObject {
    Q_OBJECT
private slots:
    void creationByName()
    {
        for(const char *ev_name : event_names)
        {
            QVERIFY2(create_by_name(ev_name)==nullptr,"no types registered yet, create_by_name result should be null");
        }
        // TODO: call register_all_events();
        register_CRUDPEvents();
        for(const char *ev_name : event_names)
        {
            QVERIFY2(create_by_name(ev_name)!=nullptr,"all types registered, create_by_name result should be non-null");
        }
    }
};

QTEST_MAIN(CRUDPEventRegistry)

#include "CRUDPEventRegistry.moc"
