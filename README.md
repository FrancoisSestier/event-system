[![Windows](https://github.com/FrancoisSestier/event-system/actions/workflows/windows.yml/badge.svg)](https://github.com/FrancoisSestier/event-system/actions/workflows/windows.yml) [![Ubuntu](https://github.com/FrancoisSestier/event-system/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/FrancoisSestier/event-system/actions/workflows/ubuntu.yml) [![codecov](https://codecov.io/gh/FrancoisSestier/event-system/branch/master/graph/badge.svg?token=ZPDP1TAO3Z)](https://codecov.io/gh/FrancoisSestier/event-system) [![License: Unlicense](https://img.shields.io/badge/license-Unlicense-blue.svg)](http://unlicense.org/)
# Event System
Single Header Lightweight event system written in c++ 20
usage : include event-system.hpp in your project

# Usage Design
```c++

struct A{
    int a;
    int b;
};

struct B {
    string s;
};


using namespace epp;

using event_A = event<A>; // alternative solution 

int main(){
    bus<event<A>> bus; // create bus 
    int a = 0;
    int b = 0;
    event_queue<event_A, event<B>> queue; // create queue

    // attach callbacks
    bus.attach_back<event<A>>([&](event<A> e) {
        a = e->a;
        e.consume();
    }); 

    bus.attach_back<event<A>>([&](event<A> e) {
        //will never fire since event callback before always consume event<A> 
    });

    bus.attach_front<event<A>>([&](event<A> e) { //will always fire since it's before the event consuming callback 
     });

    queue.push_back<event<A>>(1, 2);

    bus.dipatch(queue);

}

```
