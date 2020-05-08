#pragma once

#include "catch.h"

inline bool TEST(const std::function<bool()> &func) {
    static int sc_test_number = 0;

    auto ret = func();

    if (!ret) {
        std::abort();
    }

    return ret;
}
