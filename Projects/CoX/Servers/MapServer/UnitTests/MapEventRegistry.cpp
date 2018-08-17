#include <QTest>

#include "SEGSEventFactory.h"

using namespace SEGSEvents;
extern void register_MapEvents();
namespace
{
    const char *event_names[] =
    {
        "ShortcutsRequest",
        "SceneRequest",
        "EntitiesRequest",
        "ClientQuit",
        "ForcedLogout",
        "CookieRequest",
        "ConsoleCommand",
        "ClientResumedRendering",
        "MiniMapState",
        "CombineRequest",
        "MapInstanceConnected",
        "InspirationDockMode",
        "EnterDoor",
        "ChangeStance",
        "SetDestination",
        "ActivateInspiration",
        "SetDefaultPowerSend",
        "SetDefaultPower",
        "UnqueueAll",
        "AbortQueuedPower",
        "DescriptionAndBattleCry",
        "SwitchViewPoint",
        "TargetChatChannelSelected",
        "ChatReconfigure",
        "PowersDockMode",
        "SwitchTray",
        "SetKeybind",
        "RemoveKeybind",
        "ResetKeybinds",
        "SelectKeybindProfile",
        "ChatDividerMoved",
        "ChatMessage",
        "EmailHeaders",
        "EmailRead",
        "EntitiesResponse",
        "FloatingDamage",
        "FriendsListUpdate",
        "InfoMessageCmd",
        "InputState",
        "InteractWithEntity",
        "LocationVisited",
        "NewEntity",
        "PlaqueVisited",
        "EntityInfoResponse",
        "EntityInfoRequest",
        "SaveClientOptions",
        "Scene",
        "Shortcuts",
        "SidekickOffer",
        "StandardDialogCmd",
        "TeamLooking",
        "TeamOffer",
        "WindowState"
    };
}

class MapEventRegistry : public QObject {
    Q_OBJECT
private slots:
    void creationByName()
    {
        for(const char *ev_name : event_names)
        {
            QVERIFY2(create_by_name(ev_name)==nullptr,"no types registered yet, create_by_name result should be null");
        }
        // TODO: call register_all_events();
        register_MapEvents();
        for(const char *ev_name : event_names)
        {
            QVERIFY2(create_by_name(ev_name)!=nullptr,"all types registered, create_by_name result should be non-null");
        }
    }
};

QTEST_MAIN(MapEventRegistry)

#include "MapEventRegistry.moc"
