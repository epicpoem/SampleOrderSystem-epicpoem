#pragma once
#include <ctime>
#include <string>

class IClock {
public:
    virtual ~IClock() = default;
    virtual std::time_t now() const = 0;
    virtual std::string today() const = 0;  // YYYYMMDD
};
