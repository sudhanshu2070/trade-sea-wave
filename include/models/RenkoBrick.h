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
    
    json to_json() const {
        return {
            {"open", open},
            {"close", close},
            {"trend", (trend == Trend::UP) ? "UP" : (trend == Trend::DOWN) ? "DOWN" : "NONE"},
            {"ts", ts}
        };
    }
    
    std::string dir() const {
        return (trend == Trend::UP) ? "UP" : (trend == Trend::DOWN) ? "DOWN" : "NONE";
    }
};