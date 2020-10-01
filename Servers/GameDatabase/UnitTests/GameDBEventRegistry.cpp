#include <QTest>

#include "SEGSEventFactory.h"

using namespace SEGSEvents;
extern void register_GameDatabaseEvents();
namespace
{
    const char *event_names[] =
    {
        "CharacterUpdateMessage",
        "CostumeUpdateMessage",
        "RemoveCharacterRequest",
        "RemoveCharacterResponse",
        "GameAccountRequest",
        "GameAccountResponse",
        "CreateNewCharacterRequest",
        "CreateNewCharacterResponse",
        "GetEntityRequest",
        "GetEntityResponse",
        "WouldNameDuplicateRequest",
        "WouldNameDuplicateResponse",
        "GameDbErrorMessage",
        "SetClientOptionsMessage",
        "PlayerUpdateMessage",
    };
}

class GameDBEventRegistry : public QObject {
    Q_OBJECT
private slots:
    void creationByName()
    {
        for(const char *ev_name : event_names)
        {
            QVERIFY2(create_by_name(ev_name)==nullptr,"no types registered yet, create_by_name result should be null");
        }
        // TODO: call register_all_events();
        register_GameDatabaseEvents();
        for(const char *ev_name : event_names)
        {
            QVERIFY2(create_by_name(ev_name) != nullptr,
                     qPrintable(QString("all types registered, create_by_name(%1) result should be non-null").arg(ev_name)));
        }
    }
};

QTEST_MAIN(GameDBEventRegistry)

#include "GameDBEventRegistry.moc"
