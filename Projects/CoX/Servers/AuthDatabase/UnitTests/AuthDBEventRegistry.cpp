#include <QTest>

#include "SEGSEventFactory.h"

using namespace SEGSEvents;
extern void register_AuthDBEvents();
namespace
{
    const char *event_names[] =
    {
        "CreateAccountMessage",
        "AuthDbErrorMessage",
        "RetrieveAccountRequest",
        "RetrieveAccountResponse",
        "ValidatePasswordRequest",
        "ValidatePasswordResponse",
    };
}

class AuthDBEventRegistry : public QObject {
    Q_OBJECT
private slots:
    void creationByName()
    {
        for(const char *ev_name : event_names)
        {
            QVERIFY2(create_by_name(ev_name)==nullptr,"no types registered yet, create_by_name result should be null");
        }
        // TODO: call register_all_events();
        register_AuthDBEvents();
        for(const char *ev_name : event_names)
        {
            QVERIFY2(create_by_name(ev_name)!=nullptr,"all types registered, create_by_name result should be non-null");
        }
    }
};

QTEST_MAIN(AuthDBEventRegistry)

#include "AuthDBEventRegistry.moc"
