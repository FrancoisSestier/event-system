[![Windows](https://github.com/FrancoisSestier/event-system/actions/workflows/windows.yml/badge.svg)](https://github.com/FrancoisSestier/event-system/actions/workflows/windows.yml) [![Ubuntu](https://github.com/FrancoisSestier/event-system/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/FrancoisSestier/event-system/actions/workflows/ubuntu.yml) [![codecov](https://codecov.io/gh/FrancoisSestier/event-system/branch/master/graph/badge.svg?token=ZPDP1TAO3Z)](https://codecov.io/gh/FrancoisSestier/event-system) [![License: Unlicense](https://img.shields.io/badge/license-Unlicense-blue.svg)](http://unlicense.org/)
# Event System
Single Header Lightweight event system written in c++ 17
usage : include event-system.hpp in your project

# Usage Design
```c++

struct Event1{
    int a;
    int b;
};

struct Event2 {
    string s;
};

int main(){
    epp::dispatcher eventHandler;

    eventHandler.on<Event1>([](Event1 e){ std::cout << "Event 1 a : " << e.a << std::end });
    eventHandler.on<Event1>([](Event1 e){ std::cout << "Event 1 b : " << e.b << std::end });
    eventHandler.on<Event2>([](Event2 e){ std::cout << "Event 2 s : " << e.s << std::end });

    // Synchronous Event Dispatching 

    eventHandler.emit<Event1>(1,2);
    //Output :
    //Event 1 a : 1
    //Event 1 a : 2
    
    eventHandler.emit<Event2>("test");
    //Output :
    //Event 2 s : test
    
    // Asynchronous Event Dispatching
    
    epp::event_queue event_queue1;
    epp::event_queue event_queue2;
    
    event_queue1.push<Event1>(3, 4);
    event_queue1.push<Event2>("hi");
    
    std::swap(event_queue1, event_queue2)
    
    eventHandler.emit(event_queue2);
    //Output :
    //Event 1 a : 3
    //Event 1 a : 4
    //Event 2 s : hi
    

}

```
