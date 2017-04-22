#include <iostream>
#include <vector>

#include "Stream.h"

struct A {
public:
    A() {
        std::cout << "A ctor\n";
    }

    A (const A& other){
        std::cout << "A copy ctor\n";
    }

    A(A&& other){
        std::cout << "A move ctor\n";
    }

    ~A() {
        std::cout << "A d tor\n";
    }
};

int main(int argc, char **argv){
    //A a;

    //auto stream_test_a = MakeStream({a});

    auto stream = MakeStream({1, 2, 3, 4, 5, 6, 7, 8});

    //stream_test_a | skip(0) | filter([](A x){return true;}) | map([](A x){return x;}) | to_vector();

    (stream | skip(3) | filter([](int x){return x < 7;}) | map([](int x){return x * x;}) | print_to(std::cout)) << std::endl;

    return 0;
}