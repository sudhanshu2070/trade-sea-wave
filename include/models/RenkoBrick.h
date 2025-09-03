#pragma once
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

enum class Trend { UP, DOWN, NONE };

struct RenkoBrick {
    double open;
    double close;
    Trend trend;
    std::string ts;
    
    json to_json() const;
    
    std::string dir() const;
};