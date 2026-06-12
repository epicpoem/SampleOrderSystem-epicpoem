#pragma once
#include "IClock.h"

class FakeClock : public IClock {
public:
    explicit FakeClock(std::time_t fixedTime = 0, const std::string& fixedDate = "20260101")
        : fixedTime_(fixedTime), fixedDate_(fixedDate) {}

    std::time_t now() const override { return fixedTime_; }
    std::string today() const override { return fixedDate_; }

    void setNow(std::time_t t) { fixedTime_ = t; }
    void setToday(const std::string& d) { fixedDate_ = d; }

private:
    std::time_t fixedTime_;
    std::string fixedDate_;
};