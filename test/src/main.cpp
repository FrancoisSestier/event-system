#include <gtest/gtest.h>
#include <event-system.hpp>

struct Event1{
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

TEST(EVENT,EVENT){
    epp::dispatcher eventHandler;

    eventHandler.on<Event1>([](Event1 e){ ASSERT_EQ(e.a,1); });
    eventHandler.on<Event1>([](Event1 e){ ASSERT_EQ(e.b,2); });

    eventHandler.on<Event2>([](Event2 e){ ASSERT_EQ(e.s,'t'); });
    eventHandler.on<Event3>([](Event3 e){ ASSERT_EQ(e.s,"test"); });

    eventHandler.emit<Event1>(1,2);
    eventHandler.emit<Event2>('t');
    eventHandler.emit<Event3>("test");

}
