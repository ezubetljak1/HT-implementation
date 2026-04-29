#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace ht::test {

using TestFunction = void (*)();

struct TestCase {
    std::string name;
    TestFunction function;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

inline void registerTest(const std::string& name, TestFunction function) {
    registry().push_back(TestCase{name, function});
}

class AutoRegister {
public:
    AutoRegister(const std::string& name, TestFunction function) {
        registerTest(name, function);
    }
};

} // namespace ht::test

#define HT_TEST(name)                                      \
    void name();                                           \
    static ht::test::AutoRegister name##_registration(     \
        #name,                                             \
        &name                                              \
    );                                                     \
    void name()
