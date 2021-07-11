#include <gtest/gtest.h>

#include <event-system.hpp>

struct Event1 {
    int a;
    int b;
};

struct Event2 {
    char s;
};

struct Event3 {
    std::string s;
};

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(Dispatcher, Event) {
    epp::dispatcher eventHandler;

    int a = 0;
    int b = 0;
    std::string s;

    eventHandler.on<Event1>([&](Event1 e) {
        a = e.a;
        ASSERT_EQ(e.a, 1);
    });
    eventHandler.on<Event1>([&](Event1 e) {
        b = e.b;
        ASSERT_EQ(e.b, 2);
    });
    eventHandler.on<Event2>([](Event2 e) { ASSERT_EQ(e.s, 't'); });
    eventHandler.on<Event3>([&](Event3 e) {
        s = e.s;
        ASSERT_EQ(e.s, "test");
    });

    eventHandler.emit<Event1>(1, 2);
    eventHandler.emit<Event2>('t');
    eventHandler.emit<Event3>("test");


    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 2);
    ASSERT_EQ(s, "test");
}

TEST(Dispatcher, EventQueues) {
    epp::dispatcher dispatcher;
    epp::event_queue main_event_queue;
    int a = 0;
    int b = 0;
    std::string s;

    dispatcher.on<Event1>([&](Event1 e) {
        a = e.a;
        ASSERT_EQ(e.a, 1);
    });
    dispatcher.on<Event1>([&](Event1 e) {
        b = e.b;
        ASSERT_EQ(e.b, 2);
    });
    dispatcher.on<Event2>([](Event2 e) { ASSERT_EQ(e.s, 's'); });
    dispatcher.on<Event3>([&](Event3 e) {
        s = e.s;
        ASSERT_EQ(e.s, "test");
    });

    main_event_queue.push<Event1>(1, 2);
    main_event_queue.push<Event2>('s');
    main_event_queue.push<Event3>("test");

    ASSERT_EQ(main_event_queue.size(), 3);

    dispatcher.emit(main_event_queue);

    ASSERT_TRUE(main_event_queue.empty());

    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 2);
    ASSERT_EQ(s, "test");
}
