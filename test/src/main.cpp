#include <gtest/gtest.h>

#include <event-system.hpp>

struct A {
    int a;
    int b;
};

struct B {
    char s;
};

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

using namespace epp;

TEST(Dispatcher, Bus) {
    epp::bus<event<A>> bus;
    int a = 0;
    int b = 0;
    std::string s;

    bus.attach_back<event<A>>([&](event<A>& e) {
        a = e->a;
        e.consume();
        ASSERT_EQ(e->a, 1);
    });

    bus.attach_back<event<A>>([&](event<A>& e) {
        ASSERT_EQ(
            1,
            100);  // always false checking if event consuming is working right
    });

    bus.attach_front<event<A>>([&](event<A>& e){ ASSERT_EQ(a, 0); });

    bus.dipatch<event<A>>(1, 2);
}

TEST(Dispatcher, Queues) {
    bus<event<A>> bus;
    int a = 0;
    int b = 0;
    event_queue<event<A>, event<B>> queue;
    bus.attach_back<event<A>>([&](event<A>& e) {
        a = e->a;
        e.consume();
        ASSERT_EQ(e->a, 1);
    });

    bus.attach_back<event<A>>([&](event<A>& e) {
        ASSERT_EQ(
            1,
            100);  // always false checking if event consuming is working right
    });

    bus.attach_front<event<A>>([&](event<A> e){ ASSERT_EQ(a, 0); });

    queue.push_back<event<A>>(1, 2);

    ASSERT_EQ(queue.size(), 1);
    ASSERT_FALSE(queue.empty());

    bus.dipatch(queue);
    ASSERT_EQ(queue.size(), 1);
    bus.dipatch(queue);
    queue.clear();
    ASSERT_EQ(queue.size(), 0);
    ASSERT_TRUE(queue.empty());

}

TEST(Event,MakeBus) {
    using some_event = event_type_list<event<A>,event<B>>;

    auto bus = make_bus<some_event>();

}