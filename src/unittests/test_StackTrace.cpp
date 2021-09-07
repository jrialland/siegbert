#include <catch.hpp>
#include "logging/StackTrace.hpp"

#include <iostream>

void testFn() {
    using namespace std;
    for(auto s : logging::getStackTrace()) {
        //cout << s << endl;
    }
}

TEST_CASE("StackTrace", "[StackTrace]") {
    testFn();
}