#pragma once
#include <string>

struct Sample {
    std::string id;
    std::string name;
    double avgProductionTime{0.0};  // min/ea
    double yield{0.0};              // 0.0 ~ 1.0
    int stock{0};                   // ea
};
