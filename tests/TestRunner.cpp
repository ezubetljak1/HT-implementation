#include "TestSupport.hpp"

#include <exception>
#include <iostream>

int main() {
    int passed = 0;
    int failed = 0;

    for (const ht::test::TestCase& test : ht::test::registry()) {
        try {
            test.function();
            std::cout << "[PASS] " << test.name << "\n";
            ++passed;
        } catch (const std::exception& ex) {
            std::cout << "[FAIL] " << test.name << ": " << ex.what() << "\n";
            ++failed;
        } catch (...) {
            std::cout << "[FAIL] " << test.name << ": unknown exception\n";
            ++failed;
        }
    }

    std::cout << "\nPassed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";

    return failed == 0 ? 0 : 1;
}
