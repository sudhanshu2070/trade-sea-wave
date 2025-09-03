#include "models/RenkoBrick.h"

json RenkoBrick::to_json() const {
    return {
        {"open", open},
        {"close", close},
        {"trend", (trend == Trend::UP) ? "UP" : (trend == Trend::DOWN) ? "DOWN" : "NONE"},
        {"ts", ts}
    };
}

std::string RenkoBrick::dir() const {
    return (trend == Trend::UP) ? "UP" : (trend == Trend::DOWN) ? "DOWN" : "NONE";
}