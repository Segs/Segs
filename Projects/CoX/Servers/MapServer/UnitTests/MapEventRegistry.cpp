#include <QTest>

#include "SEGSEventFactory.h"

using namespace SEGSEvents;
extern void register_MapServerEvents();
namespace
{
    const char *event_names[] =
    {
        "AbortQueuedPower",
        "ActivateInspiration",
        "ActivatePower",
        "ActivatePowerAtLocation",
        "AddTimeStateLog",
        "Browser",
        "BrowserClose",
        "BuyEnhancementSlot",
        "ChangeStance",
        "ChangeTitle",
        "ChatDividerMoved",
        "ChatMessage",
        "ChatReconfigure",
        "ClientQuit",
        "ClientResumedRendering",
        "CombineEnhanceResponse",
        "CombineEnhancementsReq",
        "ConsoleCommand",
        "CookieRequest",
        "DeadNoGurneyOK",
        "DescriptionAndBattleCry",
        "DialogButton",
        "DoorMessage",
        "EmailHeaders",
        "EmailMessageStatus",
        "EmailRead",
        "EnterDoor",
        "EntitiesRequest",
        "EntitiesResponse",
        "EntityInfoRequest",
        "EntityInfoResponse",
        "FloatingDamage",
        "FloatingInfo",
        "ForcedLogout",
        "FriendsListUpdate",
        "InfoMessageCmd",
        "InputState",
        "InspirationDockMode",
        "InteractWithEntity",
        "LevelUp",
        "LocationVisited",
        "MapInstanceConnected",
        "MapXferList",
        "MapXferListClose",
        "MiniMapState",
        "MoveEnhancement",
        "MoveInspiration",
        "NewEntity",
        "PlaqueVisited",
        "PowersDockMode",
        "RecvCostumeChange",
        "RecvNewPower",
        "RecvSelectedTitles",
        "RemoveKeybind",
        "ResetKeybinds",
        "SaveClientOptions",
        "Scene",
        "SceneRequest",
        "SelectKeybindProfile",
        "SendStance",
        "SetClientState",
        "SetDefaultPower",
        "SetDefaultPowerSend",
        "SetDestination",
        "SetEnhancement",
        "SetKeybind",
        "Shortcuts",
        "ShortcutsRequest",
        "SidekickOffer",
        "StandardDialogCmd",
        "SwitchTray",
        "SwitchViewPoint",
        "TailorOpen",
        "TargetChatChannelSelected",
        "TeamLooking",
        "TeamOffer",
        "TimeUpdate",
        "TradeCancel",
        "TradeInit",
        "TradeOffer",
        "TradeSuccess",
        "TradeUpdate",
        "TrashEnhancement",
        "TrashEnhancementInPower",
        "TrayAdd",
        "UnqueueAll",
        "WindowState",
    };
}

class MapEventRegistry : public QObject
{
    Q_OBJECT
private slots:
    void creationByName()
    {
        for(const char *ev_name : event_names)
        {
            QVERIFY2(create_by_name(ev_name)==nullptr,"no types registered yet, create_by_name result should be null");
        }
        // TODO: call register_all_events();
        register_MapServerEvents();
        for(const char *ev_name : event_names)
        {
            QVERIFY2(create_by_name(ev_name) != nullptr,
                     qPrintable(QString("all types registered, create_by_name(%1) result should be non-null").arg(ev_name)));
        }
    }
};

QTEST_MAIN(MapEventRegistry)

#include "MapEventRegistry.moc"
