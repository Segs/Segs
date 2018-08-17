#include "Servers/HandlerLocator.h"

#include "FriendHandler.h"
#include "FriendHandlerEvents.h"
#include "MessageBus.h"
#include "SEGSEventFactory.h"

#include <QTest>

using namespace SEGSEvents;
struct MockEventProcessor : public EventProcessor
{
    IMPL_ID(MockEventProcessor)

    // EventProcessor interface
protected:
    void dispatch(Event *) override { QVERIFY(false); }
    void serialize_from(std::istream &) override { QVERIFY(false); }
    void serialize_to(std::ostream &) override { QVERIFY(false); }
};

extern void register_FriendHandlerEvents();
void        sendMessages(EventSrc *tgt, std::initializer_list<Event *> msgs)
{
    for (auto *msg : msgs)
        tgt->putq(msg);
}
uint32_t createMockChar()
{
    static uint32_t char_id=1;
    //assert(false);
    return char_id++;
}
class FriendshipServiceFunctions : public QObject
{
    Q_OBJECT
    FriendHandler *     m_handler;
    uint32_t            mock_char_id_A;
    uint32_t            mock_char_id_B;
    uint32_t            mock_char_id_C;
    MockEventProcessor *m_mock_game_server;
    MessageBus          m_message_bus;
private slots:
    void initTestCase()
    {
        register_FriendHandlerEvents();
        HandlerLocator::setMessageBus(&m_message_bus);
        // createMockGameHandler();
        m_handler          = new FriendHandler(0);
        mock_char_id_A     = createMockChar();
        mock_char_id_B     = createMockChar();
        mock_char_id_C     = createMockChar();
        m_mock_game_server = new MockEventProcessor;

        HandlerLocator::setGame_Handler(0, m_mock_game_server);
    }

    void addingAndRemovingSame()
    {
        sendMessages(m_handler, {
                                    new FriendAddedMessage({mock_char_id_A, mock_char_id_B, {}}, 0),
                                    new FriendRemovedMessage({mock_char_id_A, mock_char_id_B}, 0),
                                });
        // process all messages to completion
        bool completed = false;
        for (int i = 0; i < 10; ++i) {
            completed |= m_handler->process_single_event() == 0;
            if(completed)
                break;
        }
        QVERIFY(completed);
        // todo: verify that mock_char_id_A and mock_char_id_B are no longer friends.
        QVERIFY(false);
    }
};

QTEST_MAIN(FriendshipServiceFunctions)

#include "FriendshipServiceFunctions.moc"
