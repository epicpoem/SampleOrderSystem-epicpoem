#pragma once
#include "IClock.h"
#include <sstream>
#include <iomanip>

class SystemClock : public IClock {
public:
    std::time_t now() const override {
        return std::time(nullptr);
    }

    std::string today() const override {
        std::time_t t = now();
        std::tm tm_local{};
        localtime_s(&tm_local, &t);
        std::ostringstream oss;
        oss << std::put_time(&tm_local, "%Y%m%d");
        return oss.str();
    }
};
